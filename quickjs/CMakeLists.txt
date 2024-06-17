
project(quickjs)


set(CMAKE_C_STANDARD 99)



set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w -fwrapv -O3 -Os -DNDEBUG")


add_compile_options(
    -Wno-sign-compare
    -Wno-missing-field-initializers
    -Wundef
    -Wuninitialized
    -Wunused
    -Wno-unused-parameter
    -Wwrite-strings
    -Wchar-subscripts
    -funsigned-char
    -Wno-array-bounds 
    -Wno-format-truncation
)




file(READ "VERSION" VERSION_CONTENT)
string(STRIP "${VERSION_CONTENT}" VERSION)

add_definitions(
    -D_GNU_SOURCE
    -DCONFIG_BIGNUM
    -DCONFIG_VERSION=\"${VERSION}\"
)



if (CONFIG_WIN32)
    add_definitions(-D__USE_MINGW_ANSI_STDIO)
endif()


file(GLOB SOURCES "src/*.c")


add_library(quickjs STATIC ${SOURCES})



target_include_directories(quickjs PUBLIC include src)





