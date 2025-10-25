#pragma once

#include "config.h"
#include "database.h"
#include "pixiv_api.h"
#include "thread_pool.h"
#include <atomic>
#include <memory>
#include <unordered_set>
#include <unordered_map>

namespace pixiv2billfish {

struct Statistics {
    std::atomic<int> total_count{0};
    std::atomic<int> success_count{0};
    std::atomic<int> fail_count{0};
    std::atomic<int> skip_count{0};
    
    void print(const std::string& prefix) const;
};

class Processor {
public:
    Processor(const Config& config, Database& db);
    ~Processor();
    
    // 运行处理流程
    bool run();

private:
    const Config& config_;
    Database& db_;
    bool is_v3_db_;
    
    // API客户端
    std::unique_ptr<PixivAPI> pixiv_api_;
    
    // 线程池
    std::unique_ptr<ThreadPool> tag_pool_;
    std::unique_ptr<ThreadPool> note_pool_;
    
    // 缓存
    std::unordered_map<std::string, int64_t> tag_cache_;  // tag_name -> tag_id
    std::unordered_set<int64_t> existing_file_tags_;      // file_id with tags
    std::unordered_set<int64_t> existing_file_notes_;     // file_id with notes
    
    // 批量写入缓冲区
    std::vector<TagRecord> pending_tags_;
    std::vector<TagJoinFileRecord> pending_tag_joins_;
    std::vector<NoteRecord> pending_notes_;
    std::mutex buffer_mutex_;
    
    // 统计信息
    Statistics tag_stats_;
    Statistics note_stats_;
    
    // 初始化
    bool initialize();
    
    // 加载缓存数据
    void load_cache();
    
    // 处理标签任务
    void process_tag_task(const FileRecord& file, int index, int total);
    
    // 处理备注任务
    void process_note_task(const FileRecord& file, int index, int total);
    
    // 添加标签到缓冲区
    void add_tags_to_buffer(int64_t file_id, const std::vector<std::string>& tags);
    
    // 添加备注到缓冲区
    void add_note_to_buffer(int64_t file_id, const std::string& note, const std::string& origin);
    
    // 刷新缓冲区到数据库
    bool flush_tag_buffer(bool force = false);
    bool flush_tag_join_buffer(bool force = false);
    bool flush_note_buffer(bool force = false);
    
    // 检查标签是否存在
    std::optional<int64_t> check_tag_exist(const std::string& tag_name);
    
    // 生成新的标签ID
    int64_t generate_tag_id();
    
    // 更新Artist标签（V3数据库）
    void update_artist_tags();
};

} // namespace pixiv2billfish
