set(SOURCES
    ArmArchitecture.cpp
    ArmArchitecture.h
    ArmDisassembler.cpp
    ArmDisassembler.h
    ArmInstructionAnalyzer.cpp
    ArmInstructionAnalyzer.h
)

add_library(nc-arch-arm ${SOURCES})
target_link_libraries(nc-arch-arm nc-core nc-common capstone ${Boost_LIBRARIES} ${QT_LIBRARIES})

# vim:set et sts=4 sw=4 nospell:
