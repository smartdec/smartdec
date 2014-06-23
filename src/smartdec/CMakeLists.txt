set(SOURCES
    main.cpp
)

add_executable(smartdec WIN32 ${SOURCES})
target_link_libraries(smartdec nc-gui nc-core ${Boost_LIBRARIES} ${QT_LIBRARIES})
if(MSVC)
    set_target_properties(smartdec PROPERTIES LINK_FLAGS /ENTRY:mainCRTStartup)
endif()

install(TARGETS smartdec RUNTIME DESTINATION bin COMPONENT standalone)
if(WIN32)
    install_qt4_executable("bin/smartdec.exe")
endif()

# vim:set et sts=4 sw=4 nospell:
