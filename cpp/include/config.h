#pragma once

#include <string>
#include <map>

namespace pixiv2billfish {

struct Config {
    // 数据库配置
    std::string db_path = "billfish.db";
    
    // 代理配置
    bool use_proxies = false;
    std::string http_proxy;
    std::string https_proxy;
    
    // 功能开关
    bool write_tag = true;
    bool write_note = true;
    bool skip_existing = true;
    
    // 处理范围
    int start_file_num = 0;
    int end_file_num = 0;
    
    // 线程配置
    int tag_thread_count = 8;
    int note_thread_count = 8;
    
    // 网络配置
    int request_timeout = 5;
    int retry_count = 5;
    int request_delay_ms = 100; // 请求间延迟，避免频繁请求
    
    // 批量写入配置
    int batch_size_tag = 20;
    int batch_size_tag_join = 50;
    int batch_size_note = 10;
    
    // Pixiv API配置
    std::string pixiv_api_url = "https://www.pixiv.net/ajax/illust/";
    std::string pixiv_artwork_url = "https://www.pixiv.net/artworks/";
    
    // HTTP Headers
    std::map<std::string, std::string> headers = {
        {"Host", "www.pixiv.net"},
        {"referer", "https://www.pixiv.net/"},
        {"origin", "https://accounts.pixiv.net"},
        {"accept-language", "zh-CN,zh;q=0.9"},
        {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36"}
    };
    
    // 加载配置
    bool load_from_file(const std::string& filename);
    
    // 保存配置
    bool save_to_file(const std::string& filename) const;
};

} // namespace pixiv2billfish
