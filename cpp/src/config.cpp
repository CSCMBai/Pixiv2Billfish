#include "config.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace pixiv2billfish {

bool Config::load_from_file(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        json j;
        file >> j;
        
        // 加载配置项
        if (j.contains("db_path")) db_path = j["db_path"];
        if (j.contains("use_proxies")) use_proxies = j["use_proxies"];
        if (j.contains("http_proxy")) http_proxy = j["http_proxy"];
        if (j.contains("https_proxy")) https_proxy = j["https_proxy"];
        if (j.contains("write_tag")) write_tag = j["write_tag"];
        if (j.contains("write_note")) write_note = j["write_note"];
        if (j.contains("skip_existing")) skip_existing = j["skip_existing"];
        if (j.contains("start_file_num")) start_file_num = j["start_file_num"];
        if (j.contains("end_file_num")) end_file_num = j["end_file_num"];
        if (j.contains("tag_thread_count")) tag_thread_count = j["tag_thread_count"];
        if (j.contains("note_thread_count")) note_thread_count = j["note_thread_count"];
        if (j.contains("request_timeout")) request_timeout = j["request_timeout"];
        if (j.contains("retry_count")) retry_count = j["retry_count"];
        if (j.contains("request_delay_ms")) request_delay_ms = j["request_delay_ms"];
        
        spdlog::info("配置文件加载成功: {}", filename);
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("加载配置文件失败: {}", e.what());
        return false;
    }
}

bool Config::save_to_file(const std::string& filename) const {
    try {
        json j;
        
        j["db_path"] = db_path;
        j["use_proxies"] = use_proxies;
        j["http_proxy"] = http_proxy;
        j["https_proxy"] = https_proxy;
        j["write_tag"] = write_tag;
        j["write_note"] = write_note;
        j["skip_existing"] = skip_existing;
        j["start_file_num"] = start_file_num;
        j["end_file_num"] = end_file_num;
        j["tag_thread_count"] = tag_thread_count;
        j["note_thread_count"] = note_thread_count;
        j["request_timeout"] = request_timeout;
        j["retry_count"] = retry_count;
        j["request_delay_ms"] = request_delay_ms;
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        file << j.dump(4);
        
        spdlog::info("配置文件保存成功: {}", filename);
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("保存配置文件失败: {}", e.what());
        return false;
    }
}

} // namespace pixiv2billfish
