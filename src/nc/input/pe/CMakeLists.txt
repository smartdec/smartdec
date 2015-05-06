set(SOURCES
    PeParser.cpp
    PeParser.h
    pe.h
    poppack.h
    pshpack2.h
)

add_library(nc-input-pe ${SOURCES})
target_link_libraries(nc-input-pe nc-core nc-common ${Boost_LIBRARIES} ${QT_LIBRARIES})

# vim:set et sts=4 sw=4 nospell:
