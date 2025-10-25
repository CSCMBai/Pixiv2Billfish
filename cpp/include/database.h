#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <mutex>

namespace pixiv2billfish {

struct FileRecord {
    int64_t id;
    std::string name;
};

struct TagRecord {
    int64_t id;
    std::string name;
};

struct TagJoinFileRecord {
    int64_t file_id;
    int64_t tag_id;
};

struct NoteRecord {
    int64_t file_id;
    std::string note;
};

class Database {
public:
    explicit Database(const std::string& db_path);
    ~Database();
    
    // 禁用拷贝和移动
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    // 打开数据库连接
    bool open();
    
    // 关闭数据库连接
    void close();
    
    // 检查是否为3.0版本数据库
    bool is_version_3();
    
    // 获取文件列表
    std::vector<FileRecord> get_files(int start, int limit);
    
    // 获取文件总数
    int64_t get_file_count();
    
    // 获取所有标签
    std::vector<TagRecord> get_tags(bool is_v3);
    
    // 获取文件-标签关联
    std::vector<TagJoinFileRecord> get_tag_join_files();
    
    // 获取备注
    std::vector<NoteRecord> get_notes();
    
    // 批量插入标签
    bool insert_tags(const std::vector<TagRecord>& tags, bool is_v3);
    
    // 批量插入文件-标签关联
    bool insert_tag_join_files(const std::vector<TagJoinFileRecord>& records);
    
    // 批量插入备注
    bool insert_notes(const std::vector<NoteRecord>& notes);
    
    // 获取Artist父标签ID
    std::optional<int64_t> get_artist_tag_id();
    
    // 创建Artist父标签
    bool create_artist_tag();
    
    // 获取Artist子标签
    std::vector<TagRecord> get_artist_subtags();
    
    // 更新Artist标签
    bool update_artist_tags(const std::vector<TagRecord>& tags, int64_t parent_id);
    
    // 开始事务
    bool begin_transaction();
    
    // 提交事务
    bool commit_transaction();
    
    // 回滚事务
    bool rollback_transaction();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
    std::mutex mutex_;
};

} // namespace pixiv2billfish
