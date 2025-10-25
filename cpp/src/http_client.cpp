#include "http_client.h"
#include <curl/curl.h>
#include <spdlog/spdlog.h>
#include <thread>
#include <chrono>

namespace pixiv2billfish {

class HttpClient::Impl {
public:
    Impl() : timeout_(5) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }
    
    ~Impl() {
        curl_global_cleanup();
    }
    
    std::string http_proxy_;
    std::string https_proxy_;
    int timeout_;
    std::map<std::string, std::string> headers_;
    
    static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        size_t total_size = size * nmemb;
        userp->append(static_cast<char*>(contents), total_size);
        return total_size;
    }
};

HttpClient::HttpClient() : pimpl_(std::make_unique<Impl>()) {}

HttpClient::~HttpClient() = default;

void HttpClient::set_proxy(const std::string& http_proxy, const std::string& https_proxy) {
    pimpl_->http_proxy_ = http_proxy;
    pimpl_->https_proxy_ = https_proxy;
}

void HttpClient::set_timeout(int seconds) {
    pimpl_->timeout_ = seconds;
}

void HttpClient::set_headers(const std::map<std::string, std::string>& headers) {
    pimpl_->headers_ = headers;
}

std::optional<HttpResponse> HttpClient::get(const std::string& url, int retry_count) {
    for (int attempt = 0; attempt < retry_count; ++attempt) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            spdlog::error("CURL初始化失败");
            return std::nullopt;
        }
        
        HttpResponse response;
        response.success = false;
        
        // 设置URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        
        // 设置回调
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Impl::write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
        
        // 设置超时
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, pimpl_->timeout_);
        
        // 禁用SSL验证（与Python版本一致）
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        
        // 设置代理
        if (!pimpl_->http_proxy_.empty()) {
            curl_easy_setopt(curl, CURLOPT_PROXY, pimpl_->http_proxy_.c_str());
        }
        
        // 设置请求头
        struct curl_slist* headers = nullptr;
        for (const auto& [key, value] : pimpl_->headers_) {
            std::string header = key + ": " + value;
            headers = curl_slist_append(headers, header.c_str());
        }
        if (headers) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }
        
        // 执行请求
        CURLcode res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            response.status_code = static_cast<int>(http_code);
            response.success = true;
            
            if (headers) {
                curl_slist_free_all(headers);
            }
            curl_easy_cleanup(curl);
            
            return response;
        }
        
        if (headers) {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
        
        // 重试前等待
        if (attempt < retry_count - 1) {
            spdlog::debug("请求失败，重试 {}/{}: {}", attempt + 1, retry_count, url);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    
    spdlog::warn("请求失败，已达最大重试次数: {}", url);
    return std::nullopt;
}

std::optional<HttpResponse> HttpClient::post(const std::string& url, 
                                             const std::string& data,
                                             int retry_count) {
    for (int attempt = 0; attempt < retry_count; ++attempt) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            return std::nullopt;
        }
        
        HttpResponse response;
        response.success = false;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Impl::write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, pimpl_->timeout_);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        
        if (!pimpl_->http_proxy_.empty()) {
            curl_easy_setopt(curl, CURLOPT_PROXY, pimpl_->http_proxy_.c_str());
        }
        
        struct curl_slist* headers = nullptr;
        for (const auto& [key, value] : pimpl_->headers_) {
            std::string header = key + ": " + value;
            headers = curl_slist_append(headers, header.c_str());
        }
        if (headers) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }
        
        CURLcode res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            response.status_code = static_cast<int>(http_code);
            response.success = true;
            
            if (headers) {
                curl_slist_free_all(headers);
            }
            curl_easy_cleanup(curl);
            
            return response;
        }
        
        if (headers) {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
        
        if (attempt < retry_count - 1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    
    return std::nullopt;
}

} // namespace pixiv2billfish
