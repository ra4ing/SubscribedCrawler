cmake_minimum_required(VERSION 3.14)
project(CrawlerProject VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost libraries
find_package(Boost 1.83 REQUIRED COMPONENTS system thread regex url)
if(Boost_FOUND)
    message(STATUS "Boost found: ${Boost_VERSION}")
endif()

# Find OpenSSL
find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    message(STATUS "OpenSSL found: ${OPENSSL_VERSION}")
endif()

# 查找 Gumbo 库
find_package(PkgConfig REQUIRED)
pkg_check_modules(GUMBO REQUIRED gumbo)
if(GUMBO_FOUND)
    message(STATUS "Gumbo found: ${GUMBO_VERSION}")
endif()

# 集成 spdlog
include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.0
)

# 配置 spdlog
FetchContent_MakeAvailable(spdlog)

# Collect Source Files
file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/crawler/*.cpp
)

add_executable(crawler ${SOURCES})

target_include_directories(crawler
    PRIVATE
        ${Boost_INCLUDE_DIRS}
        ${GUMBO_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(crawler
    PRIVATE
        Boost::system 
        Boost::thread
        Boost::regex
        Boost::url
        ${GUMBO_LIBRARIES}
        OpenSSL::SSL 
        OpenSSL::Crypto
        spdlog::spdlog
        pthread
)