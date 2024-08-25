cmake_minimum_required(VERSION 3.20)


add_executable(SpiderApp
	main.cpp
        utils.h utils.cpp
        spider.h spider.cpp
	)


target_compile_features(SpiderApp PRIVATE cxx_std_17)

target_link_libraries(SpiderApp OpenSSL::SSL)

target_include_directories(SpiderApp PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(SpiderApp ${Boost_LIBRARIES})

target_link_libraries(SpiderApp SearchEngineLib)

target_include_directories(SpiderApp PRIVATE ../SearchEngineLib)


# Копируем файл INI после сборки
add_custom_command(
    TARGET SpiderApp
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CONFIG_FILE}
        $<TARGET_FILE_DIR:SpiderApp>/config.ini
    COMMENT "Copying config.ini to the output directory"
)
