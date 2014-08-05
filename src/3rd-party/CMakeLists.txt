add_subdirectory(libudis86)
add_subdirectory(libiberty)
add_subdirectory(undname)

include(ExternalProject)
ExternalProject_Add(capstone-project
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/capstone
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX:string=${EP_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DBUILD_STATIC=ON
        -DBUILD_SHARED=OFF
        -DBUILD_TESTS=OFF
        -DARM_SUPPORT=ON
        -DARM64_SUPPORT=OFF
        -DMIPS_SUPPORT=OFF
        -DPPC_SUPPORT=OFF
        -DSPARC_SUPPORT=OFF
        -DSYSZ_SUPPORT=OFF
        -DXCORE_SUPPORT=OFF
        -DX86_SUPPORT=OFF
        -DX86_REDUCE=OFF
        <SOURCE_DIR>
    BUILD_COMMAND
        echo "Will build on install."
    INSTALL_COMMAND
        cmake --build . --target install --config ${CMAKE_BUILD_TYPE}
)

# vim:set et sts=4 sw=4 nospell:
