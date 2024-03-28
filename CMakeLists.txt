cmake_minimum_required(VERSION 3.27)
project(SteamIconFix)

set(CMAKE_CXX_STANDARD 23)

include_directories("./include")
link_directories("./lib")
set(CURL_LIBRARY "-lcurl")
set(CURL_INCLUDE_DIR "./include")

add_executable(SteamIconFix
        SteamIconFix.cpp)

find_package(CURL REQUIRED)
target_link_libraries(SteamIconFix ${CURL_LIBRARIES})
target_link_libraries(SteamIconFix wsock32 ws2_32)
target_link_libraries(SteamIconFix wldap32)
target_link_libraries(SteamIconFix winmm)
target_link_libraries(SteamIconFix crypt32)
target_link_libraries(SteamIconFix normaliz)
target_link_libraries(SteamIconFix bcrypt)
set(CMAKE_EXE_LINKER_FLAGS "-static")
add_executable(SteamIconFix_C main.c
        SteamIconFix_C.h
        SteamIconFix_C.c)
