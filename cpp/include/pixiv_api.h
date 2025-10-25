#pragma once

#include "http_client.h"
#include "config.h"
#include <vector>
#include <string>
#include <optional>

namespace pixiv2billfish {

struct IllustInfo {
    std::string title;
    std::string artist;
    std::string user_id;
    int bookmark_count;
    std::string comment;
    std::vector<std::string> tags;
};

class PixivAPI {
public:
    explicit PixivAPI(const Config& config);
    ~PixivAPI() = default;
    
    // 获取插画标签
    std::optional<std::vector<std::string>> get_tags(const std::string& pid);
    
    // 获取插画详细信息（用于备注）
    std::optional<IllustInfo> get_illust_info(const std::string& pid);
    
    // 从文件名提取PID
    static std::optional<std::string> extract_pid(const std::string& filename);
    
    // 格式化备注信息
    static std::string format_note(const IllustInfo& info);

private:
    HttpClient http_client_;
    const Config& config_;
    
    // 处理艺术家名称
    static std::string process_artist_name(const std::string& artist);
    
    // 清理HTML标签
    static std::string clean_html(const std::string& html);
};

} // namespace pixiv2billfish
