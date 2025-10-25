#include "database.h"
#include <sqlite3.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace pixiv2billfish {

class Database::Impl {
public:
    sqlite3* db_ = nullptr;
    std::string db_path_;
    
    explicit Impl(const std::string& path) : db_path_(path) {}
    
    ~Impl() {
        if (db_) {
            sqlite3_close(db_);
        }
    }
    
    bool execute(const std::string& sql) {
        char* err_msg = nullptr;
        int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);
        
        if (rc != SQLITE_OK) {
            std::string error = err_msg ? err_msg : "Unknown error";
            sqlite3_free(err_msg);
            spdlog::error("SQL执行失败: {}", error);
            return false;
        }
        
        return true;
    }
};

Database::Database(const std::string& db_path) 
    : pimpl_(std::make_unique<Impl>(db_path)) {}

Database::~Database() {
    close();
}

bool Database::open() {
    int rc = sqlite3_open(pimpl_->db_path_.c_str(), &pimpl_->db_);
    
    if (rc != SQLITE_OK) {
        spdlog::error("无法打开数据库: {}", sqlite3_errmsg(pimpl_->db_));
        return false;
    }
    
    // 设置性能优化参数
    pimpl_->execute("PRAGMA synchronous = OFF");
    pimpl_->execute("PRAGMA journal_mode = MEMORY");
    pimpl_->execute("PRAGMA temp_store = MEMORY");
    pimpl_->execute("PRAGMA cache_size = 10000");
    
    return true;
}

void Database::close() {
    if (pimpl_->db_) {
        sqlite3_close(pimpl_->db_);
        pimpl_->db_ = nullptr;
    }
}

bool Database::is_version_3() {
    const char* sql = "SELECT * FROM sqlite_master WHERE type = 'table' AND tbl_name = 'bf_tag_v2';";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db_, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return false;
    }
    
    bool has_table = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    
    return has_table;
}

int64_t Database::get_file_count() {
    const char* sql = "SELECT COUNT(*) FROM bf_file";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db_, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return 0;
    }
    
    int64_t count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int64(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

std::vector<FileRecord> Database::get_files(int start, int limit) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<FileRecord> files;
    
    const char* sql = "SELECT id, name FROM bf_file LIMIT ? OFFSET ?";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db_, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        spdlog::error("准备SQL失败: {}", sqlite3_errmsg(pimpl_->db_));
        return files;
    }
    
    sqlite3_bind_int(stmt, 1, limit);
    sqlite3_bind_int(stmt, 2, start);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        FileRecord file;
        file.id = sqlite3_column_int64(stmt, 0);
        
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (name) {
            file.name = name;
        }
        
        files.push_back(file);
    }
    
    sqlite3_finalize(stmt);
    return files;
}

std::vector<TagRecord> Database::get_tags(bool is_v3) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<TagRecord> tags;
    
    std::string sql = is_v3 ? 
        "SELECT id, name FROM bf_tag_v2" : 
        "SELECT id, name FROM bf_tag";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db_, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return tags;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TagRecord tag;
        tag.id = sqlite3_column_int64(stmt, 0);
        
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (name) {
            tag.name = name;
        }
        
        tags.push_back(tag);
    }
    
    sqlite3_finalize(stmt);
    return tags;
}

std::vector<TagJoinFileRecord> Database::get_tag_join_files() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<TagJoinFileRecord> records;
    
    const char* sql = "SELECT file_id, tag_id FROM bf_tag_join_file";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db_, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return records;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TagJoinFileRecord record;
        record.file_id = sqlite3_column_int64(stmt, 0);
        record.tag_id = sqlite3_column_int64(stmt, 1);
        records.push_back(record);
    }
    
    sqlite3_finalize(stmt);
    return records;
}

std::vector<NoteRecord> Database::get_notes() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<NoteRecord> notes;
    
    const char* sql = "SELECT file_id, note FROM bf_material_userdata";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db_, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return notes;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        NoteRecord note;
        note.file_id = sqlite3_column_int64(stmt, 0);
        
        const char* note_text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (note_text) {
            note.note = note_text;
        }
        
        notes.push_back(note);
    }
    
    sqlite3_finalize(stmt);
    return notes;
}

bool Database::begin_transaction() {
    return pimpl_->execute("BEGIN TRANSACTION");
}

bool Database::commit_transaction() {
    return pimpl_->execute("COMMIT");
}

bool Database::rollback_transaction() {
    return pimpl_->execute("ROLLBACK");
}

bool Database::insert_tags(const std::vector<TagRecord>& tags, bool is_v3) {
    if (tags.empty()) return true;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string sql = is_v3 ?
        "INSERT INTO bf_tag_v2 (id, name) VALUES (?, ?)" :
        "INSERT INTO bf_tag (id, name) VALUES (?, ?)";
    
    begin_transaction();
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db_, sql.c_str(), -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        rollback_transaction();
        return false;
    }
    
    for (const auto& tag : tags) {
        sqlite3_bind_int64(stmt, 1, tag.id);
        sqlite3_bind_text(stmt, 2, tag.name.c_str(), -1, SQLITE_TRANSIENT);
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            spdlog::error("插入标签失败: {}", tag.name);
        }
        
        sqlite3_reset(stmt);
    }
    
    sqlite3_finalize(stmt);
    commit_transaction();
    
    return true;
}

bool Database::insert_tag_join_files(const std::vector<TagJoinFileRecord>& records) {
    if (records.empty()) return true;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    const char* sql = "INSERT INTO bf_tag_join_file (file_id, tag_id) VALUES (?, ?)";
    
    begin_transaction();
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db_, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        rollback_transaction();
        return false;
    }
    
    for (const auto& record : records) {
        sqlite3_bind_int64(stmt, 1, record.file_id);
        sqlite3_bind_int64(stmt, 2, record.tag_id);
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            // 忽略重复插入错误
        }
        
        sqlite3_reset(stmt);
    }
    
    sqlite3_finalize(stmt);
    commit_transaction();
    
    return true;
}

bool Database::insert_notes(const std::vector<NoteRecord>& notes) {
    if (notes.empty()) return true;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    const char* sql = "INSERT INTO bf_material_userdata (file_id, note, origin) VALUES (?, ?, ?)";
    
    begin_transaction();
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db_, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        rollback_transaction();
        return false;
    }
    
    for (const auto& note_record : notes) {
        sqlite3_bind_int64(stmt, 1, note_record.file_id);
        sqlite3_bind_text(stmt, 2, note_record.note.c_str(), -1, SQLITE_TRANSIENT);
        
        // origin从note中提取（格式化时包含）
        std::string origin;
        size_t pos = note_record.note.find("Origin:");
        if (pos != std::string::npos) {
            size_t end = note_record.note.find("\r\n", pos);
            if (end != std::string::npos) {
                origin = note_record.note.substr(pos + 7, end - pos - 7);
            }
        }
        
        sqlite3_bind_text(stmt, 3, origin.c_str(), -1, SQLITE_TRANSIENT);
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            spdlog::error("插入备注失败: file_id={}", note_record.file_id);
        }
        
        sqlite3_reset(stmt);
    }
    
    sqlite3_finalize(stmt);
    commit_transaction();
    
    return true;
}

std::optional<int64_t> Database::get_artist_tag_id() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    const char* sql = "SELECT id FROM bf_tag_v2 WHERE name='Artist'";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db_, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return std::nullopt;
    }
    
    std::optional<int64_t> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = sqlite3_column_int64(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return result;
}

bool Database::create_artist_tag() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    const char* sql = "INSERT INTO bf_tag_v2 (name) VALUES ('Artist')";
    
    char* err_msg = nullptr;
    int rc = sqlite3_exec(pimpl_->db_, sql, nullptr, nullptr, &err_msg);
    
    if (rc != SQLITE_OK) {
        if (err_msg) {
            spdlog::error("创建Artist标签失败: {}", err_msg);
            sqlite3_free(err_msg);
        }
        return false;
    }
    
    return true;
}

std::vector<TagRecord> Database::get_artist_subtags() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<TagRecord> tags;
    
    const char* sql = "SELECT id, name FROM bf_tag_v2 WHERE name LIKE 'Artist:%' AND (pid IS NULL OR pid = 0)";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db_, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        return tags;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TagRecord tag;
        tag.id = sqlite3_column_int64(stmt, 0);
        
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (name) {
            tag.name = name;
        }
        
        tags.push_back(tag);
    }
    
    sqlite3_finalize(stmt);
    return tags;
}

bool Database::update_artist_tags(const std::vector<TagRecord>& tags, int64_t parent_id) {
    if (tags.empty()) return true;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    const char* sql = "UPDATE bf_tag_v2 SET name = ?, pid = ? WHERE id = ?";
    
    begin_transaction();
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pimpl_->db_, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        rollback_transaction();
        return false;
    }
    
    for (const auto& tag : tags) {
        // 移除 "Artist:" 前缀
        std::string new_name = tag.name;
        if (new_name.find("Artist:") == 0) {
            new_name = new_name.substr(7);
        }
        
        sqlite3_bind_text(stmt, 1, new_name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, parent_id);
        sqlite3_bind_int64(stmt, 3, tag.id);
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            spdlog::error("更新Artist标签失败: {}", tag.name);
        }
        
        sqlite3_reset(stmt);
    }
    
    sqlite3_finalize(stmt);
    commit_transaction();
    
    return true;
}

} // namespace pixiv2billfish
