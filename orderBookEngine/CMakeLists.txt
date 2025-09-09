cmake_minimum_required(VERSION 3.16)
project(MyProject LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

find_package(nlohmann_json 3.2.0 REQUIRED)

include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)
set(RDKAFKA_LIBRARIES rdkafka++ rdkafka)
set(RDKAFKA_INCLUDE_DIRS /opt/homebrew/include)

add_executable(MyProject
    main.cpp
    Servers/server.cpp
    orderBook.cpp
    message.cpp
    order-producer.cpp
    new-order-consumer.cpp
    orderBookAggregator.cpp
)

target_include_directories(MyProject PRIVATE
    ${RDKAFKA_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(MyProject PRIVATE 
    ${RDKAFKA_LIBRARIES}
    nlohmann_json::nlohmann_json 
)

if(MSVC)
    target_compile_options(MyProject PRIVATE /W4)
else()
    target_compile_options(MyProject PRIVATE -Wall -Wextra -pedantic)
endif()
