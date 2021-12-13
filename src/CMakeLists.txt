set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror -Wextra -g")

set(Boost_USE_MULTITHREADED ON)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)
find_package(CURL)

include_directories(/usr/local/include ${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIR} /usr/local/include/mysql-connector-cpp)

if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    add_definitions(-DHAVE_CURL)
endif()

add_library(db_api SHARED db_api.cpp)

add_executable(example example.cpp)
add_executable(parse_operands_gen parse_operands_gen.cpp)

add_library(parser SHARED parser.cpp lexer.cpp instruction.cpp parse_operands.cpp)

add_executable(bot bot.cpp)

add_custom_command(OUTPUT parse_operands.cpp
                  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/parse_operands_gen
                  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                  DEPENDS parse_operands_gen)

# add_dependencies(parser parse_operands.cpp)



set(FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR /usr/local/include/mysql-connector-cpp)
include_directories(${FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR}/include)
link_directories(${FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR}/lib64)

target_link_libraries(bot db_api /usr/local/include/mysql-connector-cpp/libmysqlcppconn8.so mysqlcppconn /usr/local/lib/libTgBot.a ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${CURL_LIBRARIES} )

target_link_libraries(example parser)
target_include_directories(parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# link_directories(/usr/local/lib)

# target_link_libraries(bot db_api /usr/local/lib/libmysqlcppconn.so /usr/local/lib/libTgBot.a ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${CURL_LIBRARIES})
