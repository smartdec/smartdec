set(SOURCES
    cp-demangle.c
    cp-demangle.h
    demangle.h
)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    add_definitions(-Wno-unused-function -Wno-implicit-fallthrough)
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    add_definitions(-Wno-unused-function)
endif()

if(MSVC)
    add_definitions(-Dinline=__inline -Dalloca=_alloca -D_CRT_SECURE_NO_WARNINGS /wd4244 /wd4133 /wd4047 /wd4267)
endif()

add_definitions(-DHAVE_STDLIB_H -DHAVE_STRING_H -DIN_GLIBCPP_V3)
add_library(iberty ${SOURCES}) 

# vim:set et sts=4 sw=4 nospell:
