project(game)
cmake_policy(SET CMP0072 NEW)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -frtti -fno-strict-aliasing")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -mtune=native")



add_compile_options(
    -Wextra
    -Wall
    -Wno-sign-compare
    -Wno-missing-field-initializers
    -Wundef
    -Wuninitialized
    -Wunused
    -Wno-unused-parameter
    -Wwrite-strings
    -Wchar-subscripts
    -funsigned-char
)



find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})




find_package(OpenGL REQUIRED)
if(OpenGL_FOUND)
    include_directories(${OPENGL_INCLUDE_DIR})
endif()



set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)



file(GLOB SOURCES "src/*.cpp")
add_executable(game_main   ${SOURCES})

target_include_directories(core PUBLIC include)
target_link_libraries(game_main PRIVATE core quickjs ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})

target_precompile_headers(game_main PRIVATE ../core/include/pch.h)

if(CMAKE_BUILD_TYPE MATCHES Debug)
     target_compile_options(game_main PRIVATE -fsanitize=address -fsanitize=undefined -fsanitize=leak -g -Winvalid-pch)
     target_link_options(game_main PRIVATE -fsanitize=address -fsanitize=undefined -fsanitize=leak -g -Winvalid-pch) 
endif()
