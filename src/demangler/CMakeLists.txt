set(SOURCES
    main.cpp
)

add_executable(demangler ${SOURCES})
target_link_libraries(demangler iberty undname)

if (NOT ${IDA_PLUGIN_ENABLED})
    install(TARGETS demangler RUNTIME DESTINATION bin)
    if(WIN32)
        install_qt4_executable("bin/demangler.exe")
    endif()
endif()

# vim:set et sts=4 sw=4 nospell:
