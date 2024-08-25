cmake_minimum_required(VERSION 3.20)

project(SearchEngine)


# TODO: Define by your own needs:
set(OPENSSL_ROOT_DIR "C:\\openssl-build")
set(BOOST_ROOT "C:\\boost")
set(libpqxx_DIR "C:\\libpqxx-7.9.2")


set(CONFIG_FILE "${CMAKE_SOURCE_DIR}/config.ini")
# maybe unnecessary
set(OPENSSL_LIBRARIES ${OPENSSL_ROOT_DIR}/lib)
include_directories(${OPENSSL_ROOT_DIR}/include)
link_directories(${OPENSSL_LIBRARIES})

# TODO: If you are running on Windows below 10, change this:
add_compile_definitions(_WIN32_WINNT=0x0A01)
cmake_policy(SET CMP0144 NEW)

# Boost:
find_package(Boost 1.80.0 REQUIRED COMPONENTS system thread locale)

# PQXX
set(PostgreSQL_ADDITIONAL_VERSIONS "15")
add_subdirectory(${libpqxx_DIR} libpqxx-build)

# OpenSSL
find_package(OpenSSL REQUIRED)

add_subdirectory(SearchEngineLib)

add_subdirectory(http_server)

add_subdirectory(spider)




