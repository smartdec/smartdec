set(SOURCES
    main.cpp
)

add_executable(nocode ${SOURCES})
target_link_libraries(nocode nc-core ${Boost_LIBRARIES} ${QT_LIBRARIES})

install(TARGETS nocode RUNTIME DESTINATION bin COMPONENT standalone)
if(WIN32)
    install_qt4_executable("bin/nocode.exe")
endif()

# vim:set et sts=4 sw=4 nospell:
