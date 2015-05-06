set(SOURCES
    CallingConventions.cpp
    CallingConventions.h
    IntelArchitecture.cpp
    IntelArchitecture.h
    IntelDataflowAnalyzer.cpp
    IntelDataflowAnalyzer.h
    IntelInstruction.cpp
    IntelInstruction.h
    IntelInstructionAnalyzer.cpp
    IntelInstructionAnalyzer.h
    IntelInstructionDisassembler.cpp
    IntelInstructionDisassembler.h
    IntelMnemonics.cpp
    IntelMnemonics.h
    IntelOperands.cpp
    IntelOperands.h
    IntelRegisters.cpp
    IntelRegisters.h
    IntelUniversalAnalyzer.cpp
    IntelUniversalAnalyzer.h
    Prefixes.cpp
    Prefixes.h
)

add_library(nc-arch-intel ${SOURCES})
target_link_libraries(nc-arch-intel nc-core nc-common udis86 ${Boost_LIBRARIES} ${QT_LIBRARIES})

# vim:set et sts=4 sw=4 nospell:
