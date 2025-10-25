#include "processor.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <chrono>

namespace pixiv2billfish {

void Statistics::print(const std::string& prefix) const {
    spdlog::info("=== {} 统计 ===", prefix);
    spdlog::info("  总数: {}", total_count.load());
    spdlog::info("  成功: {}", success_count.load());
    spdlog::info("  失败: {}", fail_count.load());
    spdlog::info("  跳过: {}", skip_count.load());
}

Processor::Processor(const Config& config, Database& db)
    : config_(config), db_(db), is_v3_db_(false) {
}

Processor::~Processor() = default;

bool Processor::initialize() {
    // 检查数据库版本
    is_v3_db_ = db_.is_version_3();
    spdlog::info("数据库版本: {}", is_v3_db_ ? "3.0+" : "2.x");
    
    // 创建API客户端
    pixiv_api_ = std::make_unique<PixivAPI>(config_);
    
    // 创建线程池
    if (config_.write_tag) {
        tag_pool_ = std::make_unique<ThreadPool>(config_.tag_thread_count);
        spdlog::info("标签线程池已创建: {} 线程", config_.tag_thread_count);
    }
    
    if (config_.write_note) {
        note_pool_ = std::make_unique<ThreadPool>(config_.note_thread_count);
        spdlog::info("备注线程池已创建: {} 线程", config_.note_thread_count);
    }
    
    // 加载缓存
    load_cache();
    
    return true;
}

void Processor::load_cache() {
    spdlog::info("正在加载缓存数据...");
    
    // 加载标签缓存
    auto tags = db_.get_tags(is_v3_db_);
    for (const auto& tag : tags) {
        tag_cache_[tag.name] = tag.id;
    }
    spdlog::info("已加载 {} 个标签", tag_cache_.size());
    
    // 加载文件-标签关联
    auto tag_joins = db_.get_tag_join_files();
    for (const auto& join : tag_joins) {
        existing_file_tags_.insert(join.file_id);
    }
    spdlog::info("已加载 {} 个文件标签关联", existing_file_tags_.size());
    
    // 加载备注
    auto notes = db_.get_notes();
    for (const auto& note : notes) {
        if (!note.note.empty()) {
            existing_file_notes_.insert(note.file_id);
        }
    }
    spdlog::info("已加载 {} 个文件备注", existing_file_notes_.size());
}

bool Processor::run() {
    if (!initialize()) {
        spdlog::error("初始化失败");
        return false;
    }
    
    // 获取文件列表
    int64_t total_files = db_.get_file_count();
    spdlog::info("数据库中共有 {} 个文件", total_files);
    
    int start = config_.start_file_num;
    int limit = config_.end_file_num == 0 ? 
        static_cast<int>(total_files - start) : config_.end_file_num;
    
    spdlog::info("处理范围: {} - {}", start, start + limit);
    
    auto files = db_.get_files(start, limit);
    spdlog::info("成功加载 {} 个文件", files.size());
    
    if (files.empty()) {
        spdlog::warn("没有文件需要处理");
        return true;
    }
    
    // 提交任务
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<std::future<void>> futures;
    
    for (size_t i = 0; i < files.size(); ++i) {
        const auto& file = files[i];
        int index = static_cast<int>(i + 1);
        int total = static_cast<int>(files.size());
        
        if (config_.write_tag && tag_pool_) {
            futures.push_back(tag_pool_->enqueue(
                &Processor::process_tag_task, this, file, index, total
            ));
        }
        
        if (config_.write_note && note_pool_) {
            futures.push_back(note_pool_->enqueue(
                &Processor::process_note_task, this, file, index, total
            ));
        }
    }
    
    // 等待所有任务完成
    spdlog::info("等待所有任务完成...");
    
    for (auto& future : futures) {
        future.get();
    }
    
    if (tag_pool_) {
        tag_pool_->wait_all();
    }
    
    if (note_pool_) {
        note_pool_->wait_all();
    }
    
    // 刷新剩余缓冲区
    spdlog::info("正在写入剩余数据...");
    
    if (config_.write_tag) {
        flush_tag_buffer(true);
        flush_tag_join_buffer(true);
    }
    
    if (config_.write_note) {
        flush_note_buffer(true);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
    
    // 打印统计信息
    if (config_.write_tag) {
        tag_stats_.print("标签");
    }
    
    if (config_.write_note) {
        note_stats_.print("备注");
    }
    
    spdlog::info("总耗时: {} 秒", duration.count());
    
    // 更新Artist标签（仅V3数据库）
    if (is_v3_db_ && config_.write_tag) {
        update_artist_tags();
    }
    
    return true;
}

void Processor::process_tag_task(const FileRecord& file, int index, int total) {
    tag_stats_.total_count++;
    
    // 提取PID
    auto pid_opt = PixivAPI::extract_pid(file.name);
    if (!pid_opt) {
        tag_stats_.fail_count++;
        spdlog::debug("[{}/{}] 无法提取PID: {}", index, total, file.name);
        return;
    }
    
    std::string pid = *pid_opt;
    
    // 检查是否需要跳过
    if (config_.skip_existing && existing_file_tags_.count(file.id) > 0) {
        tag_stats_.skip_count++;
        spdlog::debug("[{}/{}] 已有标签，跳过: {}", index, total, file.name);
        return;
    }
    
    // 获取标签
    auto tags_opt = pixiv_api_->get_tags(pid);
    if (!tags_opt || tags_opt->empty()) {
        tag_stats_.fail_count++;
        spdlog::warn("[{}/{}] 获取标签失败: {}", index, total, file.name);
        return;
    }
    
    // 添加到缓冲区
    add_tags_to_buffer(file.id, *tags_opt);
    
    tag_stats_.success_count++;
    spdlog::info("[{}/{}] 标签处理完成: {} (PID={}, {} tags)", 
                 index, total, file.name, pid, tags_opt->size());
    
    // 定期刷新缓冲区
    flush_tag_buffer(false);
    flush_tag_join_buffer(false);
}

void Processor::process_note_task(const FileRecord& file, int index, int total) {
    note_stats_.total_count++;
    
    // 提取PID
    auto pid_opt = PixivAPI::extract_pid(file.name);
    if (!pid_opt) {
        note_stats_.fail_count++;
        spdlog::debug("[{}/{}] 无法提取PID: {}", index, total, file.name);
        return;
    }
    
    std::string pid = *pid_opt;
    
    // 检查是否需要跳过
    if (config_.skip_existing && existing_file_notes_.count(file.id) > 0) {
        note_stats_.skip_count++;
        spdlog::debug("[{}/{}] 已有备注，跳过: {}", index, total, file.name);
        return;
    }
    
    // 获取插画信息
    auto info_opt = pixiv_api_->get_illust_info(pid);
    if (!info_opt) {
        note_stats_.fail_count++;
        spdlog::warn("[{}/{}] 获取插画信息失败: {}", index, total, file.name);
        return;
    }
    
    // 格式化备注
    std::string note = PixivAPI::format_note(*info_opt);
    std::string origin = config_.pixiv_artwork_url + pid;
    
    // 添加到缓冲区
    add_note_to_buffer(file.id, note, origin);
    
    note_stats_.success_count++;
    spdlog::info("[{}/{}] 备注处理完成: {} (PID={})", 
                 index, total, file.name, pid);
    
    // 定期刷新缓冲区
    flush_note_buffer(false);
}

void Processor::add_tags_to_buffer(int64_t file_id, const std::vector<std::string>& tags) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    
    for (const auto& tag : tags) {
        auto tag_id_opt = check_tag_exist(tag);
        
        if (tag_id_opt) {
            // 标签已存在
            pending_tag_joins_.push_back({file_id, *tag_id_opt});
        } else {
            // 新标签
            int64_t new_tag_id = generate_tag_id();
            pending_tags_.push_back({new_tag_id, tag});
            pending_tag_joins_.push_back({file_id, new_tag_id});
            
            // 更新缓存
            tag_cache_[tag] = new_tag_id;
        }
    }
}

void Processor::add_note_to_buffer(int64_t file_id, const std::string& note, const std::string& origin) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    
    NoteRecord record;
    record.file_id = file_id;
    record.note = note + "\r\nOrigin:" + origin;
    
    pending_notes_.push_back(record);
}

bool Processor::flush_tag_buffer(bool force) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    
    if (pending_tags_.empty() || 
        (!force && pending_tags_.size() < static_cast<size_t>(config_.batch_size_tag))) {
        return true;
    }
    
    bool success = db_.insert_tags(pending_tags_, is_v3_db_);
    if (success) {
        spdlog::debug("已写入 {} 个标签", pending_tags_.size());
        pending_tags_.clear();
    }
    
    return success;
}

bool Processor::flush_tag_join_buffer(bool force) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    
    if (pending_tag_joins_.empty() || 
        (!force && pending_tag_joins_.size() < static_cast<size_t>(config_.batch_size_tag_join))) {
        return true;
    }
    
    bool success = db_.insert_tag_join_files(pending_tag_joins_);
    if (success) {
        spdlog::debug("已写入 {} 个文件-标签关联", pending_tag_joins_.size());
        
        // 更新缓存
        for (const auto& join : pending_tag_joins_) {
            existing_file_tags_.insert(join.file_id);
        }
        
        pending_tag_joins_.clear();
    }
    
    return success;
}

bool Processor::flush_note_buffer(bool force) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    
    if (pending_notes_.empty() || 
        (!force && pending_notes_.size() < static_cast<size_t>(config_.batch_size_note))) {
        return true;
    }
    
    bool success = db_.insert_notes(pending_notes_);
    if (success) {
        spdlog::debug("已写入 {} 个备注", pending_notes_.size());
        
        // 更新缓存
        for (const auto& note : pending_notes_) {
            existing_file_notes_.insert(note.file_id);
        }
        
        pending_notes_.clear();
    }
    
    return success;
}

std::optional<int64_t> Processor::check_tag_exist(const std::string& tag_name) {
    // 对于Artist标签，尝试两种形式
    if (is_v3_db_ && tag_name.find("Artist:") == 0) {
        std::string simple_name = tag_name.substr(7);
        auto it = tag_cache_.find(simple_name);
        if (it != tag_cache_.end()) {
            return it->second;
        }
    }
    
    auto it = tag_cache_.find(tag_name);
    if (it != tag_cache_.end()) {
        return it->second;
    }
    
    return std::nullopt;
}

int64_t Processor::generate_tag_id() {
    if (tag_cache_.empty()) {
        return 1;
    }
    
    // 找到最大ID
    int64_t max_id = 0;
    for (const auto& [name, id] : tag_cache_) {
        if (id > max_id) {
            max_id = id;
        }
    }
    
    return max_id + 1;
}

void Processor::update_artist_tags() {
    spdlog::info("更新Artist标签...");
    
    // 获取或创建Artist父标签
    auto artist_id_opt = db_.get_artist_tag_id();
    if (!artist_id_opt) {
        if (!db_.create_artist_tag()) {
            spdlog::error("创建Artist父标签失败");
            return;
        }
        artist_id_opt = db_.get_artist_tag_id();
        if (!artist_id_opt) {
            spdlog::error("获取Artist标签ID失败");
            return;
        }
    }
    
    int64_t artist_id = *artist_id_opt;
    spdlog::info("Artist父标签ID: {}", artist_id);
    
    // 获取需要更新的Artist子标签
    auto artist_subtags = db_.get_artist_subtags();
    if (artist_subtags.empty()) {
        spdlog::info("没有需要更新的Artist标签");
        return;
    }
    
    spdlog::info("找到 {} 个Artist子标签需要更新", artist_subtags.size());
    
    // 更新标签
    if (db_.update_artist_tags(artist_subtags, artist_id)) {
        spdlog::info("Artist标签更新成功");
    } else {
        spdlog::error("Artist标签更新失败");
    }
}

} // namespace pixiv2billfish
