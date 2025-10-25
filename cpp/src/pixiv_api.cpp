#include "pixiv_api.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <regex>
#include <thread>
#include <chrono>

using json = nlohmann::json;

namespace pixiv2billfish {

PixivAPI::PixivAPI(const Config& config) : config_(config) {
    http_client_.set_timeout(config.request_timeout);
    http_client_.set_headers(config.headers);
    
    if (config.use_proxies) {
        http_client_.set_proxy(config.http_proxy, config.https_proxy);
    }
}

std::optional<std::string> PixivAPI::extract_pid(const std::string& filename) {
    // 支持的扩展名
    static const std::vector<std::string> extensions = {
        "jpg", "png", "gif", "webp", "webm", "zip",
        "jpg.lnk", "png.lnk", "gif.lnk", "webp.lnk", "webm.lnk", "zip.lnk"
    };
    
    // 检查扩展名
    bool valid_ext = false;
    for (const auto& ext : extensions) {
        if (filename.size() > ext.size() && 
            filename.substr(filename.size() - ext.size()) == ext) {
            valid_ext = true;
            break;
        }
    }
    
    if (!valid_ext) {
        return std::nullopt;
    }
    
    // 提取PID
    std::string pid;
    
    size_t dash_pos = filename.find('-');
    size_t underscore_pos = filename.find('_');
    size_t dot_pos = filename.find('.');
    
    if (dash_pos != std::string::npos) {
        pid = filename.substr(0, dash_pos);
    } else if (underscore_pos != std::string::npos) {
        pid = filename.substr(0, underscore_pos);
    } else if (dot_pos != std::string::npos) {
        pid = filename.substr(0, dot_pos);
    } else {
        return std::nullopt;
    }
    
    // 验证PID是否为数字
    if (pid.empty() || !std::all_of(pid.begin(), pid.end(), ::isdigit)) {
        return std::nullopt;
    }
    
    return pid;
}

std::string PixivAPI::process_artist_name(const std::string& artist) {
    std::string result = artist;
    
    // 移除@后面的内容
    size_t at_pos = result.rfind('@');
    if (at_pos != std::string::npos && at_pos >= 2 && at_pos <= result.size() - 3) {
        result = result.substr(0, at_pos);
    }
    
    // 移除全角@后面的内容
    size_t fullwidth_at_pos = result.rfind('＠');
    if (fullwidth_at_pos != std::string::npos && 
        fullwidth_at_pos >= 2 && fullwidth_at_pos <= result.size() - 3) {
        result = result.substr(0, fullwidth_at_pos);
    }
    
    return result;
}

std::string PixivAPI::clean_html(const std::string& html) {
    std::string result = html;
    
    // 替换<br />为换行
    std::regex br_regex("<br\\s*/?>", std::regex::icase);
    result = std::regex_replace(result, br_regex, "\r\n");
    
    // 处理链接
    std::regex link_regex("<a\\s+href=\"([^\"]+)\"[^>]*>", std::regex::icase);
    result = std::regex_replace(result, link_regex, "[url]$1[/url]\r\n");
    
    // 移除其他HTML标签
    std::regex tag_regex("<[^>]+>");
    result = std::regex_replace(result, tag_regex, "");
    
    // 移除jump.php链接
    std::regex jump_regex("\\[url\\]/jump\\.php[^\\]]*\\[/url\\]\\r\\n");
    result = std::regex_replace(result, jump_regex, "");
    
    return result;
}

std::optional<std::vector<std::string>> PixivAPI::get_tags(const std::string& pid) {
    std::string url = config_.pixiv_api_url + pid;
    
    // 请求延迟
    if (config_.request_delay_ms > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(config_.request_delay_ms));
    }
    
    auto response = http_client_.get(url, config_.retry_count);
    
    if (!response || !response->success) {
        spdlog::warn("获取标签失败 PID={}", pid);
        return std::nullopt;
    }
    
    if (response->status_code == 404) {
        spdlog::warn("PID={} 返回404", pid);
        return std::vector<std::string>{"Error:404"};
    }
    
    try {
        json j = json::parse(response->body);
        
        if (j["error"].get<bool>()) {
            spdlog::warn("API返回错误 PID={}: {}", pid, j["message"].get<std::string>());
            return std::nullopt;
        }
        
        std::vector<std::string> tag_list;
        
        // 添加艺术家名称
        std::string artist = j["body"]["userName"].get<std::string>();
        artist = process_artist_name(artist);
        tag_list.push_back("Artist:" + artist);
        
        // 添加标签
        auto tags = j["body"]["tags"]["tags"];
        for (const auto& tag : tags) {
            // 添加英文翻译
            if (tag.contains("translation") && tag["translation"].contains("en")) {
                tag_list.push_back(tag["translation"]["en"].get<std::string>());
            }
            // 添加原始标签
            tag_list.push_back(tag["tag"].get<std::string>());
        }
        
        // 去重
        std::sort(tag_list.begin(), tag_list.end());
        tag_list.erase(std::unique(tag_list.begin(), tag_list.end()), tag_list.end());
        
        return tag_list;
        
    } catch (const json::exception& e) {
        spdlog::error("解析JSON失败 PID={}: {}", pid, e.what());
        return std::nullopt;
    }
}

std::optional<IllustInfo> PixivAPI::get_illust_info(const std::string& pid) {
    std::string url = config_.pixiv_api_url + pid;
    
    // 请求延迟
    if (config_.request_delay_ms > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(config_.request_delay_ms));
    }
    
    auto response = http_client_.get(url, config_.retry_count);
    
    if (!response || !response->success) {
        spdlog::warn("获取插画信息失败 PID={}", pid);
        return std::nullopt;
    }
    
    if (response->status_code == 404) {
        spdlog::warn("PID={} 返回404", pid);
        IllustInfo info;
        info.comment = "Error:404";
        return info;
    }
    
    try {
        json j = json::parse(response->body);
        
        if (j["error"].get<bool>()) {
            spdlog::warn("API返回错误 PID={}: {}", pid, j["message"].get<std::string>());
            return std::nullopt;
        }
        
        IllustInfo info;
        info.title = j["body"]["illustTitle"].get<std::string>();
        info.artist = process_artist_name(j["body"]["userName"].get<std::string>());
        info.user_id = j["body"]["userId"].get<std::string>();
        info.bookmark_count = j["body"]["bookmarkCount"].get<int>();
        
        std::string raw_comment = j["body"]["illustComment"].get<std::string>();
        info.comment = clean_html(raw_comment);
        
        return info;
        
    } catch (const json::exception& e) {
        spdlog::error("解析JSON失败 PID={}: {}", pid, e.what());
        return std::nullopt;
    }
}

std::string PixivAPI::format_note(const IllustInfo& info) {
    std::string note;
    
    note += "Title:" + info.title + "\r\n";
    note += "Artist:" + info.artist + "\r\n";
    note += "UID:" + info.user_id + "\r\n";
    note += "Bookmark:" + std::to_string(info.bookmark_count) + "\r\n";
    
    if (!info.comment.empty()) {
        note += "Comment:\r\n" + info.comment;
    } else {
        note += "No Comment\r\n";
    }
    
    // 转义单引号
    size_t pos = 0;
    while ((pos = note.find('\'', pos)) != std::string::npos) {
        note.replace(pos, 1, "''");
        pos += 2;
    }
    
    return note;
}

} // namespace pixiv2billfish
