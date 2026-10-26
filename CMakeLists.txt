cmake_minimum_required(VERSION 3.16)

# ─── One list of files, not two ──────────────────────────────────────
#
# qmake is what this repository builds with and what the CI runs, and
# StellarQtSDK.pri is where every source is listed. This file used to keep its
# own copy of that list, and the copy had drifted: it was missing src/qml/
# entirely, along with contractspec, wasmsections and about half the test
# suite. A CMake build that quietly compiles thirty files fewer than the qmake
# one is worse than none, because it goes green.
#
# So the lists are read from the .pri at configure time. Add a file there and
# both builds pick it up; there is nothing here to forget. The version too: it
# is the one the .pri hands the SDK as STELLAR_QT_SDK_CLIENT_VERSION, so the
# installed package cannot report a different number from the library inside.
# Editing either file, or the parser, makes the next build reconfigure.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(ReadQmakeLists)
read_qmake_version("${CMAKE_CURRENT_SOURCE_DIR}/StellarQtSDK.pri" _stellar_version)

project(StellarQtSDK VERSION ${_stellar_version} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)

# The QML layer is optional, and OFF by default here as it is in the .pri: it
# links QtQml, which a plain C++ consumer has no business pulling in. The lab
# turns it on with -DSTELLAR_QT_QML=ON; a library user need not. (It defaulted
# to ON here while the .pri defaulted to OFF, so the same SDK dragged QtQml into
# a CMake consumer and not into a qmake one.)
option(STELLAR_QT_QML "Build the optional QML facade layer (links QtQml)" OFF)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
set(_stellar_qt_components Core Network Test)
if(STELLAR_QT_QML)
    list(APPEND _stellar_qt_components Qml)
endif()
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${_stellar_qt_components})

if(STELLAR_QT_QML)
    read_qmake_lists("${CMAKE_CURRENT_SOURCE_DIR}/StellarQtSDK.pri" STELLAR WITH_QML)
else()
    read_qmake_lists("${CMAKE_CURRENT_SOURCE_DIR}/StellarQtSDK.pri" STELLAR)
endif()

list(LENGTH STELLAR_SOURCES _stellar_source_count)
message(STATUS "StellarQtSDK ${PROJECT_VERSION}: ${_stellar_source_count} "
               "sources from StellarQtSDK.pri")
if(STELLAR_QT_QML)
    message(STATUS "StellarQtSDK: QML layer ON (src/qml/ is built and "
                   "Qt${QT_VERSION_MAJOR}::Qml is linked)")
else()
    message(STATUS "StellarQtSDK: QML layer OFF, the default (no QtQml "
                   "dependency; -DSTELLAR_QT_QML=ON builds src/qml/)")
endif()

add_library(StellarQtSDK STATIC ${STELLAR_SOURCES} ${STELLAR_HEADERS})

# Debug and Release side by side under one prefix. Both archives used to be
# installed as the same file (StellarQtSDK.lib, libStellarQtSDK.a), so the
# second install overwrote the first while the exported targets of both
# configurations still named it: a Debug consumer linked the Release archive,
# or the other way round, and with MSVC the two runtimes (/MDd, /MD) do not
# mix. With the postfix each configuration's export names its own file, and
# find_package() hands a consumer the one that matches its configuration; a
# RelWithDebInfo or MinSizeRel consumer gets the release archive when its own
# configuration is not installed (StellarQtSDKConfig.cmake.in maps them).
set_target_properties(StellarQtSDK PROPERTIES DEBUG_POSTFIX "d")

# ─── Link this library WHOLE, or lose the type registrations ─────────
#
# READ THIS BEFORE LINKING StellarQtSDK FROM ANYWHERE.
#
# About twenty response .cpp files register their nested gadgets with
# Q_COREAPP_STARTUP_FUNCTION, and that startup function is the only thing
# referring to those objects. A linker drops an object file from a static
# archive when nothing refers to it, so the registration never runs — and then
# QMetaProperty::read cannot fill a QList<AssetAmount> and hands back an empty
# list. No error, no warning, just a liquidity pool with no reserves.
#
# That is not hypothetical: it cost eight test failures the first time this
# library was built as an archive, every one of them "expected 2, actual 0".
# qmake never had the problem because both consumers compile the SDK sources
# straight into their own executable, so every translation unit is there by
# construction.
#
# So link the target that carries the whole-archive request, and not a bare
# `StellarQtSDK`. It has the same name in this tree (add_subdirectory) as in an
# installed package (find_package):
#
#     target_link_libraries(yourapp PRIVATE StellarQtSDK::StellarQtSDK)
#
# In an installed package the archive itself is StellarQtSDK::StellarQtSDKStatic.
# Nothing should link that one directly, for the reason above.
add_library(StellarQtSDKWholeArchive INTERFACE)
add_library(StellarQtSDK::StellarQtSDK ALIAS StellarQtSDKWholeArchive)
set_target_properties(StellarQtSDKWholeArchive PROPERTIES EXPORT_NAME StellarQtSDK)
set_target_properties(StellarQtSDK PROPERTIES EXPORT_NAME StellarQtSDKStatic)

if(CMAKE_VERSION VERSION_LESS 3.24)
    message(WARNING
        "CMake ${CMAKE_VERSION} cannot request whole-archive linking. The "
        "response type registrations will be dropped and deserialised lists "
        "will come back empty. Use CMake 3.24 or newer, or build with qmake. "
        "No CMake package is installed from this configuration.")
    target_link_libraries(StellarQtSDKWholeArchive INTERFACE StellarQtSDK)
else()
    # install(EXPORT) rewrites a target name to its namespaced export name in a
    # plain link item, but NOT inside $<LINK_LIBRARY:…> (checked with CMake
    # 3.30): with one spelling, the installed package would ask the consumer's
    # linker for a library called just "StellarQtSDK". Hence the two.
    target_link_libraries(StellarQtSDKWholeArchive INTERFACE
        "$<BUILD_INTERFACE:$<LINK_LIBRARY:WHOLE_ARCHIVE,StellarQtSDK>>"
        "$<INSTALL_INTERFACE:$<LINK_LIBRARY:WHOLE_ARCHIVE,StellarQtSDK::StellarQtSDKStatic>>"
    )
endif()

# The variable this file used to hand a parent project; kept, so a parent that
# links ${STELLAR_QT_SDK_LINK} still links the archive whole.
set(STELLAR_QT_SDK_LINK StellarQtSDK::StellarQtSDK)
# (PROJECT_IS_TOP_LEVEL needs CMake 3.21; this works on the 3.16 declared above.)
if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(STELLAR_QT_SDK_LINK "${STELLAR_QT_SDK_LINK}" PARENT_SCOPE)
endif()

# ─── Everything the .pri sets, set the same way here ─────────────────
#
# PUBLIC rather than PRIVATE: a consumer compiling against these headers needs
# the same QIODeviceEnums spelling and the same include paths, and getting that
# wrong produces errors deep inside the SDK's headers rather than at the call
# site. BUILD_INTERFACE keeps these source-tree paths out of anything an
# install or export might ever write; the installed headers have one root
# (see "Installing" below).
set(_stellar_include_dest "${CMAKE_INSTALL_INCLUDEDIR}/StellarQtSDK")
target_include_directories(StellarQtSDK PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
    "$<INSTALL_INTERFACE:${_stellar_include_dest}>"
)

target_compile_definitions(StellarQtSDK PUBLIC
    STELLAR_QT_REPLY_TIMEOUT=30000
    ED25519_NO_SEED
    QT_DEPRECATED_WARNINGS
    STELLAR_QT_SDK_CLIENT_NAME="qtcpp-stellar-sdk"
    STELLAR_QT_SDK_CLIENT_VERSION="${PROJECT_VERSION}"
)

# QIODevice's enums moved into QIODeviceBase in Qt 6. The .pri picks the
# spelling by major version and so does this.
if(QT_VERSION_MAJOR GREATER 5)
    target_compile_definitions(StellarQtSDK PUBLIC QIODeviceEnums=QIODeviceBase)
else()
    target_compile_definitions(StellarQtSDK PUBLIC QIODeviceEnums=QIODevice)
endif()

if(STELLAR_QT_QML)
    target_compile_definitions(StellarQtSDK PUBLIC STELLAR_QT_QML)
endif()

# Qt 5.15.2 ships QtCore headers (qfloat16.h, qendian.h) that specialize
# std::numeric_limits WITHOUT first including <limits> (QTBUG-90395, fixed in
# 5.15.3+ and Qt 6). On GCC 11+ the compiler binds std::numeric_limits to Qt's
# partial declaration first, and later <chrono>/<random>/qrandom.h fail with
# "std::numeric_limits is not a template". Force-include <limits> as the very
# first thing in every translation unit: the problem is TU-wide ordering, so no
# per-header include can fix it. Scoped to Qt 5 with GCC or Clang — Qt 6 has the
# fixed header and MSVC was never affected.
if(QT_VERSION_MAJOR LESS 6
   AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
        OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    # C++ only: <limits> is no C header, and the library (and a consumer)
    # has C sources too (ed25519).
    target_compile_options(StellarQtSDK PUBLIC "$<$<COMPILE_LANGUAGE:CXX>:SHELL:-include limits>")
endif()

target_link_libraries(StellarQtSDK PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
)
if(STELLAR_QT_QML)
    target_link_libraries(StellarQtSDK PUBLIC Qt${QT_VERSION_MAJOR}::Qml)
endif()

# ─── The test suite ──────────────────────────────────────────────────
#
# Its class list comes out of StellarQtSDKTest.pro for the same reason as the
# sources: the suite gains a file most weeks and a list that has to be updated
# twice is a list that will be updated once. A test that is not compiled is not
# a test that passes — it is one nobody notices is gone.
enable_testing()

# read_qmake_version on a few one-line .pri files: whole x.y.z versions are
# read as written, anything else is refused (see cmake/ReadQmakeLists.cmake).
add_test(NAME ReadQmakeVersion
    COMMAND "${CMAKE_COMMAND}"
            "-DREAD_QMAKE_VERSION_SELFTEST=${CMAKE_CURRENT_BINARY_DIR}/ReadQmakeVersionTest"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ReadQmakeLists.cmake"
)

# The Debug archive has its own name (DEBUG_POSTFIX above), so both
# configurations can be installed under one prefix.
add_test(NAME DebugPostfix
    COMMAND "${CMAKE_COMMAND}" -E echo "$<CONFIG>:$<TARGET_FILE_NAME:StellarQtSDK>"
)
set_tests_properties(DebugPostfix PROPERTIES PASS_REGULAR_EXPRESSION
    "^(Debug:(lib)?StellarQtSDKd|(Release|RelWithDebInfo|MinSizeRel)?:(lib)?StellarQtSDK)\\.(lib|a)")

read_qmake_headers("${CMAKE_CURRENT_SOURCE_DIR}/StellarQtSDKTest.pro"
                   "${CMAKE_CURRENT_SOURCE_DIR}" STELLAR_TEST)

# Without the QML layer its tests go too: they include src/qml/, which was not
# compiled, and QtQml, which was not found, so with them in the list the suite
# did not build at all with STELLAR_QT_QML=OFF. The qmake suite always builds
# the layer (its .pro defines STELLAR_QT_QML before including the .pri), so it
# has no such switch to read; what marks a QML test is the directory the .pro
# lists it from, test/qml/. Tests elsewhere that touch the layer (pricetest.h)
# guard it with #ifdef STELLAR_QT_QML themselves, and are compiled either way.
set(_stellar_skipped_tests "")
if(NOT STELLAR_QT_QML)
    set(_stellar_kept_tests "")
    foreach(_test IN LISTS STELLAR_TEST_HEADERS)
        file(RELATIVE_PATH _test_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_test}")
        if(_test_rel MATCHES "^test/qml/")
            list(APPEND _stellar_skipped_tests "${_test_rel}")
        else()
            list(APPEND _stellar_kept_tests "${_test}")
        endif()
    endforeach()
    # If the QML tests move, this filter keeps them all and the build fails at
    # the link with symbols from src/qml/. Say so here instead.
    if(_stellar_skipped_tests STREQUAL "")
        message(FATAL_ERROR
            "StellarQtSDKTest.pro lists no test under test/qml/, so the QML "
            "tests cannot be left out of a STELLAR_QT_QML=OFF build. They "
            "moved: update the filter above.")
    endif()
    set(STELLAR_TEST_HEADERS "${_stellar_kept_tests}")
endif()

list(LENGTH STELLAR_TEST_HEADERS _stellar_test_count)
list(LENGTH _stellar_skipped_tests _stellar_skipped_count)
if(_stellar_skipped_count GREATER 0)
    message(STATUS "StellarQtSDK: ${_stellar_test_count} test headers from "
                   "StellarQtSDKTest.pro; ${_stellar_skipped_count} under "
                   "test/qml/ left out because STELLAR_QT_QML is OFF")
else()
    message(STATUS "StellarQtSDK: ${_stellar_test_count} test headers from "
                   "StellarQtSDKTest.pro")
endif()

# One moc file per header, the way qmake does it — NOT AUTOMOC.
#
# AUTOMOC concatenates every generated moc into a single mocs_compilation.cpp,
# which drags all hundred-odd test headers into one translation unit. Two things
# break there that are perfectly fine apart, and neither is the build system's
# fault to fix:
#
#   - this SDK has two classes called qstellar::LiquidityPool, one in
#     src/liquiditypool.h and one in src/responses/liquiditypool.h, and in a
#     shared TU whichever was included first wins;
#   - the test suite is written against the unqualified 0.x API, so it is
#     already living with a namespace flattened on purpose.
#
# qmake gives each header its own TU and the question never arises. Doing the
# same here keeps the two builds compiling the same code rather than making the
# CMake build a reason to rename a public class.

# The defines are stated once and handed to both the compiler and moc.
#
# moc reads the same #if as the compiler does, and several test classes guard
# their slots — ServerTest's live submissions sit behind STELLAR_SKIP_LIVE_TESTS.
# Give moc a different set from the compiler and it emits a meta-object for
# methods that were never compiled, which fails at the link with a message that
# points at generated code and not at the cause.
set(_stellar_test_defines
    STELLAR_ALLOW_UNSECURE_RANDOM
    STELLAR_SKIP_LIVE_TESTS
    STELLAR_ENABLE_TEST_METHODS
    # The suite is written against the unqualified 0.x API (KeyPair, Server, …).
    # A new consumer must NOT define this; it picks up the strict qstellar::
    # names instead.
    STELLAR_QT_LEGACY_GLOBAL_NS
)
if(STELLAR_QT_QML)
    list(APPEND _stellar_test_defines STELLAR_QT_QML)
endif()

set(_stellar_moc_options
    "-I${CMAKE_CURRENT_SOURCE_DIR}"
    "-I${CMAKE_CURRENT_SOURCE_DIR}/src"
)
foreach(_define IN LISTS _stellar_test_defines)
    list(APPEND _stellar_moc_options "-D${_define}")
endforeach()

set_source_files_properties(${STELLAR_TEST_HEADERS} PROPERTIES SKIP_AUTOMOC ON)
qt_wrap_cpp(STELLAR_TEST_MOC ${STELLAR_TEST_HEADERS}
    OPTIONS ${_stellar_moc_options}
)

add_executable(StellarQtSDKTest EXCLUDE_FROM_ALL
    ${STELLAR_TEST_SOURCES}
    ${STELLAR_TEST_HEADERS}
    ${STELLAR_TEST_MOC}
)
set_target_properties(StellarQtSDKTest PROPERTIES AUTOMOC OFF)

target_compile_definitions(StellarQtSDKTest PRIVATE ${_stellar_test_defines})

target_link_libraries(StellarQtSDKTest PRIVATE
    StellarQtSDK::StellarQtSDK
    Qt${QT_VERSION_MAJOR}::Test
)

add_test(NAME StellarQtSDKTest COMMAND StellarQtSDKTest)
add_custom_target(check
    COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure
    DEPENDS StellarQtSDKTest
)

# ─── Installing ──────────────────────────────────────────────────────
#
#     cmake --install <build> --prefix <prefix>
#
# and then, in the consuming project (CMake 3.24 or newer):
#
#     find_package(StellarQtSDK 1.0 REQUIRED)
#     target_link_libraries(yourapp PRIVATE StellarQtSDK::StellarQtSDK)
#
# Headers keep the layout the .pri gives a qmake consumer, whose INCLUDEPATH is
# the repository root and src/: they go under one root, include/StellarQtSDK/,
# with src/ flattened into it and xdr/ beside it. So "keypair.h",
# "responses/accountresponse.h" and "xdr/stellartransaction.h" resolve as they
# do in the tree, and the "../" includes between headers still land. The root
# is its own directory because names such as util.h, network.h or server.h
# would collide with anything else installed under the same prefix.
#
# Only the headers the .pri lists for this configuration are installed: not
# src/qml/ in a build without the QML layer, and not external/, which no public
# header includes.
foreach(_header IN LISTS STELLAR_HEADERS)
    file(RELATIVE_PATH _header_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_header}")
    if(_header_rel MATCHES "^src/(.+)$")
        set(_header_rel "${CMAKE_MATCH_1}")
    elseif(NOT _header_rel MATCHES "^xdr/")
        continue()
    endif()
    get_filename_component(_header_dir "${_header_rel}" DIRECTORY)
    install(FILES "${_header}"
            DESTINATION "${_stellar_include_dest}/${_header_dir}")
endforeach()

if(CMAKE_VERSION VERSION_LESS 3.24)
    # The library and its headers, but no package: a StellarQtSDK::StellarQtSDK
    # that cannot link whole would hand the consumer empty lists (see above).
    install(TARGETS StellarQtSDK
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    )
else()
    set(_stellar_config_dest "${CMAKE_INSTALL_LIBDIR}/cmake/StellarQtSDK")

    install(TARGETS StellarQtSDK StellarQtSDKWholeArchive
        EXPORT StellarQtSDKTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    )
    install(EXPORT StellarQtSDKTargets
        NAMESPACE StellarQtSDK::
        DESTINATION "${_stellar_config_dest}"
    )

    include(CMakePackageConfigHelpers)
    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/StellarQtSDKConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/StellarQtSDKConfig.cmake"
        INSTALL_DESTINATION "${_stellar_config_dest}"
    )
    # Same major version: the public API is frozen at 1.0.0 and only grows.
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/StellarQtSDKConfigVersion.cmake"
        COMPATIBILITY SameMajorVersion
    )
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/StellarQtSDKConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/StellarQtSDKConfigVersion.cmake"
        DESTINATION "${_stellar_config_dest}"
    )
endif()
