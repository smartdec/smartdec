set(MOC_HEADERS
    SignalLogger.h
)

set(SOURCES
    BitTwiddling.h
    ByteOrder.h
    CancellationToken.cpp
    CancellationToken.h
    CheckedCast.h
    Conversions.cpp
    Conversions.h
    DisjointSet.h
    Escaping.cpp
    Escaping.h
    Exception.cpp
    Exception.h
    Foreach.h
    GitSHA1.h
    Kinds.h
    LogToken.h
    Logger.h
    PrintCallback.h
    Printable.h
    Range.h
    RangeClass.h
    SizedValue.h
    StreamLogger.h
    Types.h
    Unreachable.h
    Unused.h
    Visitor.h
    Warnings.h
    make_unique.h
)

qt4_wrap_cpp(SOURCES ${MOC_HEADERS})

# Find out current revision.
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GitSHA1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp" @ONLY)
list(APPEND SOURCES "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp")

add_library(nc-common ${SOURCES})
target_link_libraries(nc-common ${Boost_LIBRARIES} ${QT_LIBRARIES})

# vim:set et sts=4 sw=4 nospell:
