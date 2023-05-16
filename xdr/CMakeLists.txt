file(GLOB SOURCES "./*")

include_directories("${CMAKE_SOURCE_DIR}")

add_library (
    xdr
    STATIC
)

target_sources (
    xdr
    PUBLIC
        ${SOURCES}
)

find_package (Qt5 COMPONENTS Core REQUIRED)

target_link_libraries (
    xdr
    PUBLIC
        Qt5
)