cmake_minimum_required(VERSION 3.15)
project(Pixiv2Billfish VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 检测编译器类型
if(MINGW)
    message(STATUS "Detected MinGW compiler")
    set(USE_MINGW TRUE)
    
    # 尝试多个可能的 MSYS2/MinGW 路径
    set(POSSIBLE_MINGW_ROOTS
        "C:/msys64/mingw64"
        "D:/msys64/mingw64"
        "C:/msys2/mingw64"
        "D:/msys2/mingw64"
        "D:/mingw64"
        "C:/mingw64"
    )
    
    foreach(ROOT ${POSSIBLE_MINGW_ROOTS})
        if(EXISTS "${ROOT}/include" AND EXISTS "${ROOT}/lib")
            list(APPEND CMAKE_PREFIX_PATH "${ROOT}")
            message(STATUS "Added to search path: ${ROOT}")
        endif()
    endforeach()
endif()

# 查找依赖库
if(USE_MINGW)
    # MinGW 使用 pkg-config
    find_package(PkgConfig QUIET)
    
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(CURL REQUIRED libcurl)
        pkg_check_modules(SQLITE3 REQUIRED sqlite3)
        pkg_check_modules(SPDLOG REQUIRED spdlog)
    else()
        # 手动查找库
        message(STATUS "Searching for libraries manually...")
        
        find_library(CURL_LIBRARY NAMES curl libcurl curl.dll libcurl.dll
            PATHS ${CMAKE_PREFIX_PATH}
            PATH_SUFFIXES lib bin
        )
        find_path(CURL_INCLUDE_DIR curl/curl.h
            PATHS ${CMAKE_PREFIX_PATH}
            PATH_SUFFIXES include
        )
        
        find_library(SQLITE3_LIBRARY NAMES sqlite3 libsqlite3 sqlite3.dll libsqlite3.dll
            PATHS ${CMAKE_PREFIX_PATH}
            PATH_SUFFIXES lib bin
        )
        find_path(SQLITE3_INCLUDE_DIR sqlite3.h
            PATHS ${CMAKE_PREFIX_PATH}
            PATH_SUFFIXES include
        )
        
        find_library(SPDLOG_LIBRARY NAMES spdlog libspdlog spdlog.dll libspdlog.dll
            PATHS ${CMAKE_PREFIX_PATH}
            PATH_SUFFIXES lib bin
        )
        find_path(SPDLOG_INCLUDE_DIR spdlog/spdlog.h
            PATHS ${CMAKE_PREFIX_PATH}
            PATH_SUFFIXES include
        )
        
        # spdlog 依赖 fmt
        find_library(FMT_LIBRARY NAMES fmt libfmt fmt.dll libfmt.dll
            PATHS ${CMAKE_PREFIX_PATH}
            PATH_SUFFIXES lib bin
        )
        find_path(FMT_INCLUDE_DIR fmt/core.h
            PATHS ${CMAKE_PREFIX_PATH}
            PATH_SUFFIXES include
        )
        
        message(STATUS "CURL_LIBRARY: ${CURL_LIBRARY}")
        message(STATUS "CURL_INCLUDE_DIR: ${CURL_INCLUDE_DIR}")
        message(STATUS "SQLITE3_LIBRARY: ${SQLITE3_LIBRARY}")
        message(STATUS "SQLITE3_INCLUDE_DIR: ${SQLITE3_INCLUDE_DIR}")
        message(STATUS "SPDLOG_LIBRARY: ${SPDLOG_LIBRARY}")
        message(STATUS "SPDLOG_INCLUDE_DIR: ${SPDLOG_INCLUDE_DIR}")
        message(STATUS "FMT_LIBRARY: ${FMT_LIBRARY}")
        message(STATUS "FMT_INCLUDE_DIR: ${FMT_INCLUDE_DIR}")
        
        if(NOT CURL_LIBRARY OR NOT CURL_INCLUDE_DIR)
            message(FATAL_ERROR "\nCURL not found!\n"
                "Please install in MSYS2 MinGW 64-bit terminal:\n"
                "  pacman -S mingw-w64-x86_64-curl\n"
                "Or download from: https://curl.se/windows/\n"
                "Searched paths: ${CMAKE_PREFIX_PATH}")
        endif()
        
        if(NOT SQLITE3_LIBRARY OR NOT SQLITE3_INCLUDE_DIR)
            message(FATAL_ERROR "\nSQLite3 not found!\n"
                "Please install in MSYS2 MinGW 64-bit terminal:\n"
                "  pacman -S mingw-w64-x86_64-sqlite3\n"
                "Searched paths: ${CMAKE_PREFIX_PATH}")
        endif()
        
        if(NOT SPDLOG_LIBRARY OR NOT SPDLOG_INCLUDE_DIR)
            message(FATAL_ERROR "\nspdlog not found!\n"
                "Please install in MSYS2 MinGW 64-bit terminal:\n"
                "  pacman -S mingw-w64-x86_64-spdlog\n"
                "Searched paths: ${CMAKE_PREFIX_PATH}")
        endif()
        
        if(NOT FMT_LIBRARY OR NOT FMT_INCLUDE_DIR)
            message(FATAL_ERROR "\nfmt not found!\n"
                "Please install in MSYS2 MinGW 64-bit terminal:\n"
                "  pacman -S mingw-w64-x86_64-fmt\n"
                "Searched paths: ${CMAKE_PREFIX_PATH}")
        endif()
    endif()
    
    # nlohmann-json 是 header-only 库
    find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp
        PATHS ${CMAKE_PREFIX_PATH}
        PATH_SUFFIXES include
    )
    if(NOT NLOHMANN_JSON_INCLUDE_DIR)
        message(FATAL_ERROR "\nnlohmann-json not found!\n"
            "Please install in MSYS2 MinGW 64-bit terminal:\n"
            "  pacman -S mingw-w64-x86_64-nlohmann-json\n"
            "Searched paths: ${CMAKE_PREFIX_PATH}")
    endif()
    
    message(STATUS "NLOHMANN_JSON_INCLUDE_DIR: ${NLOHMANN_JSON_INCLUDE_DIR}")
else()
    # MSVC 或其他编译器使用标准 find_package
    find_package(CURL REQUIRED)
    find_package(SQLite3 REQUIRED)
    find_package(nlohmann_json 3.2.0 REQUIRED)
    find_package(spdlog REQUIRED)
endif()

find_package(Threads REQUIRED)

# 源文件
set(SOURCES
    src/main.cpp
    src/config.cpp
    src/database.cpp
    src/http_client.cpp
    src/pixiv_api.cpp
    src/thread_pool.cpp
    src/processor.cpp
)

# 头文件
set(HEADERS
    include/config.h
    include/database.h
    include/http_client.h
    include/pixiv_api.h
    include/thread_pool.h
    include/processor.h
)

# 可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 包含目录
if(USE_MINGW)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CURL_INCLUDE_DIR}
        ${CURL_INCLUDE_DIRS}
        ${SQLITE3_INCLUDE_DIR}
        ${SQLITE3_INCLUDE_DIRS}
        ${SPDLOG_INCLUDE_DIR}
        ${SPDLOG_INCLUDE_DIRS}
        ${NLOHMANN_JSON_INCLUDE_DIR}
    )
    
    # 链接库
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${CURL_LIBRARY}
        ${CURL_LIBRARIES}
        ${SQLITE3_LIBRARY}
        ${SQLITE3_LIBRARIES}
        ${SPDLOG_LIBRARY}
        ${SPDLOG_LIBRARIES}
        ${FMT_LIBRARY}
        Threads::Threads
        ws2_32  # Windows sockets
    )
else()
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CURL_INCLUDE_DIRS}
        ${SQLite3_INCLUDE_DIRS}
    )
    
    # 链接库
    target_link_libraries(${PROJECT_NAME} PRIVATE
        CURL::libcurl
        SQLite::SQLite3
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        Threads::Threads
    )
endif()

# 编译选项
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /O2 /arch:AVX2)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -O3 -march=native)
endif()

# 安装
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
