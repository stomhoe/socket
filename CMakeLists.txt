cmake_minimum_required(VERSION 3.20)

project(reto2
    VERSION 0.1.0
    LANGUAGES CXX
)

# Require C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific warning flags
if (MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Create static library for UDP Socket
add_library(_udp_socket STATIC
    src/Socket.cpp
)

# Include directory for the library
target_include_directories(_udp_socket
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# Main executable
add_executable(${PROJECT_NAME}
    src/udp_example.cpp
)

# Link the static library to the main executable
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        _udp_socket
)

# Include directory for the executable
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

# Installation
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(TARGETS _udp_socket DESTINATION lib)
install(FILES include/Socket.hpp DESTINATION include)
