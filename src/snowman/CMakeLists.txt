set(SOURCES
    main.cpp
)

add_executable(snowman WIN32 ${SOURCES})
target_link_libraries(snowman nc nc-gui ${Boost_LIBRARIES} ${NC_QT_CORE} ${NC_QT_WIDGETS})
if(MSVC)
    set_target_properties(snowman PROPERTIES LINK_FLAGS /ENTRY:mainCRTStartup)
endif()

if (NOT ${IDA_PLUGIN_ENABLED})
    install(TARGETS snowman RUNTIME DESTINATION bin)
    if(WIN32 AND NOT ${NC_QT5})
        install_qt4_executable("bin/snowman.exe")
    endif()
endif()

# vim:set et sts=4 sw=4 nospell:
