set(SOURCES
    main.cpp
)

add_executable(nocode ${SOURCES})
target_link_libraries(nocode nc-core ${Boost_LIBRARIES} ${QT_LIBRARIES})

# vim:set et sts=4 sw=4 nospell:
