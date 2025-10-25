#include "config.h"
#include "database.h"
#include "processor.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>
#include <memory>

using namespace pixiv2billfish;

void setup_logger() {
    try {
        // 控制台输出
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);
        
        // 文件输出
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            "pixiv2billfish.log", true);
        file_sink->set_level(spdlog::level::debug);
        
        // 创建logger
        std::vector<spdlog::sink_ptr> sinks {console_sink, file_sink};
        auto logger = std::make_shared<spdlog::logger>("main", sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::debug);
        
        spdlog::set_default_logger(logger);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "日志初始化失败: " << ex.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    setup_logger();
    
    spdlog::info("=== Pixiv2Billfish C++ Version ===");
    spdlog::info("高性能版本启动中...");
    
    try {
        // 加载配置
        Config config;
        std::string config_file = "config.json";
        
        if (argc > 1) {
            config_file = argv[1];
        }
        
        if (!config.load_from_file(config_file)) {
            spdlog::warn("配置文件 {} 未找到，使用默认配置", config_file);
        }
        
        // 打印配置信息
        spdlog::info("配置信息:");
        spdlog::info("  数据库路径: {}", config.db_path);
        spdlog::info("  使用代理: {}", config.use_proxies ? "是" : "否");
        spdlog::info("  写入标签: {}", config.write_tag ? "是" : "否");
        spdlog::info("  写入备注: {}", config.write_note ? "是" : "否");
        spdlog::info("  跳过已存在: {}", config.skip_existing ? "是" : "否");
        spdlog::info("  起始文件: {}", config.start_file_num);
        spdlog::info("  结束文件: {}", config.end_file_num == 0 ? "全部" : std::to_string(config.end_file_num));
        spdlog::info("  标签线程数: {}", config.tag_thread_count);
        spdlog::info("  备注线程数: {}", config.note_thread_count);
        
        // 打开数据库
        Database db(config.db_path);
        if (!db.open()) {
            spdlog::error("无法打开数据库: {}", config.db_path);
            return 1;
        }
        
        spdlog::info("数据库连接成功");
        
        // 创建处理器
        Processor processor(config, db);
        
        // 运行处理
        if (!processor.run()) {
            spdlog::error("处理失败");
            return 1;
        }
        
        spdlog::info("=== 处理完成 ===");
        
    } catch (const std::exception& e) {
        spdlog::error("发生异常: {}", e.what());
        return 1;
    }
    
    return 0;
}
