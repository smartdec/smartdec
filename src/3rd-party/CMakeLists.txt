add_subdirectory(libudis86)

if(${NC_WITH_LIBIBERTY})
    add_subdirectory(libiberty)
endif()

if(${NC_WITH_UNDNAME})
    add_subdirectory(undname)
endif()

# vim:set et sts=4 sw=4 nospell:
