cmake_minimum_required(VERSION 3.20)


add_executable(HttpServerApp 
	main.cpp
        http_connection.h
        http_connection.cpp
	)

target_compile_features(HttpServerApp PRIVATE cxx_std_17) 

target_include_directories(HttpServerApp PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(HttpServerApp ${Boost_LIBRARIES})

target_link_libraries(HttpServerApp OpenSSL::SSL)

target_link_libraries(HttpServerApp SearchEngineLib)

target_include_directories(HttpServerApp PRIVATE ../SearchEngineLib)


# Копируем файл INI после сборки
add_custom_command(
    TARGET HttpServerApp
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CONFIG_FILE}
        $<TARGET_FILE_DIR:HttpServerApp>/config.ini
    COMMENT "Copying config.ini to the output directory"
)

