set(SOURCES
    ElfParser.cpp
    ElfParser.h
    elf32.h
    elf64.h
    elf_common.h
)

add_library(nc-input-elf ${SOURCES})
target_link_libraries(nc-input-elf nc-core nc-common ${Boost_LIBRARIES} ${QT_LIBRARIES})

# vim:set et sts=4 sw=4 nospell:
