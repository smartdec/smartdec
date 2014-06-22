set(SOURCES
    main.cpp
)

add_executable(demangler ${SOURCES})
target_link_libraries(demangler iberty undname)

# vim:set et sts=4 sw=4 nospell:
