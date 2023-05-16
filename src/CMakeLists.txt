file (
    GLOB
    SOURCES
        "./*"
        "./requests/*"
        "./federation/*"

        "./responses/*"
        "./responses/effects/*"
        "./responses/operations/*"
)

add_library (
    stellar_source
    STATIC
)

target_sources (
    stellar_source
    PUBLIC
        ${SOURCES}
)

find_package (Qt5 COMPONENTS Core REQUIRED)
include_directories("${CMAKE_SOURCE_DIR}")

target_link_libraries (
    xdr
    PUBLIC
        Qt5
)