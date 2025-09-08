cmake_minimum_required(VERSION 3.16)
project(MyProject LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()


find_package(Boost 1.74 REQUIRED COMPONENTS system)

include_directories(/opt/homebrew/include) 
link_directories(/opt/homebrew/lib)

add_executable(MyProject
    main.cpp
    Servers/server.cpp
    orderBook.cpp
    message.cpp
    Servers/serverBoost.cpp
    order-producer.cpp
    new-order-consumer.cpp
    orderBookAggregator.cpp
)

target_link_libraries(MyProject PRIVATE rdkafka++ rdkafka)

if(Boost_FOUND)
    target_include_directories(MyProject PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(MyProject PRIVATE ${Boost_LIBRARIES})
endif()

target_include_directories(MyProject PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(/opt/homebrew/include) #

if(MSVC)
    target_compile_options(MyProject PRIVATE /W4)
else()
    target_compile_options(MyProject PRIVATE -Wall -Wextra -pedantic)
endif()
