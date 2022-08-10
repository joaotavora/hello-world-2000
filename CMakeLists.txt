cmake_minimum_required(VERSION 3.9)
project(hello CXX) # PROJECT_NAME is now "hello"
set(CMAKE_CXX_STANDARD 20)

# This will come in handy for LSP servers such as clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Decent compile options
add_compile_options(-Wall -Wextra -Werror -pedantic)

# Conan is important
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Add a fancy-sounding "core lib"
set(core_lib ${PROJECT_NAME}-lib)
file(GLOB_RECURSE src_cpp CONFIGURE_DEPENDS "src/core/*.cpp")
add_library(${core_lib} STATIC ${src_cpp})
target_link_libraries(${core_lib} PRIVATE ${CONAN_LIBS})
target_include_directories(${core_lib} PUBLIC ./src)
set_target_properties(${core_lib} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

# The silly example -DHELLO_IS_HELLO preprocessor directive can still apply
target_compile_definitions(${core_lib} PUBLIC HELLO_IS_HELLO)

# Now make the "hello" executable target depending on "core lib"
file(GLOB src_cpp CONFIGURE_DEPENDS "src/*.cpp")
add_executable(${PROJECT_NAME} ${src_cpp})
target_link_libraries(${PROJECT_NAME} PRIVATE ${core_lib} ${CONAN_LIBS})




