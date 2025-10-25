#pragma once

#include <string>
#include <map>
#include <memory>
#include <optional>

namespace pixiv2billfish {

struct HttpResponse {
    int status_code;
    std::string body;
    std::map<std::string, std::string> headers;
    bool success;
};

class HttpClient {
public:
    HttpClient();
    ~HttpClient();
    
    // 设置代理
    void set_proxy(const std::string& http_proxy, const std::string& https_proxy);
    
    // 设置超时
    void set_timeout(int seconds);
    
    // 设置请求头
    void set_headers(const std::map<std::string, std::string>& headers);
    
    // GET请求
    std::optional<HttpResponse> get(const std::string& url, int retry_count = 5);
    
    // POST请求
    std::optional<HttpResponse> post(const std::string& url, 
                                     const std::string& data,
                                     int retry_count = 5);

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace pixiv2billfish
