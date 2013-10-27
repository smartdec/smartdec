set(SOURCES
    undname.c
    undname.h
)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    add_definitions(-Wno-sign-compare)
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    add_definitions(-Wno-sign-compare)
endif()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS /wd4018 /wd4267 /wd4244)
endif()

add_library(undname ${SOURCES})

# vim:set et sts=4 sw=4 nospell:
