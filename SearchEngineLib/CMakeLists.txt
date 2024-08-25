cmake_minimum_required(VERSION 3.20)

project(SearchEngineLib)

add_library(SearchEngineLib STATIC
    logger.h
    link.h
    ini_config.h
    database_manager.h
    main.cpp
)

target_compile_features(SearchEngineLib PRIVATE cxx_std_17)

target_link_libraries(SearchEngineLib pqxx)

target_include_directories(SearchEngineLib PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(SearchEngineLib ${Boost_LIBRARIES})
