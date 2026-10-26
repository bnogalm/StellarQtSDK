# Reads the file lists out of a qmake .pri, so CMake and qmake cannot disagree.
#
# WHY THIS EXISTS. This repository builds with qmake — that is what the CI runs
# and what the maintainer edits — and StellarQtSDK.pri is where every source
# file is listed. The CMakeLists used to keep a second copy of that list, typed
# out by hand, and it had drifted badly: by the time this was written it was
# missing src/qml/ entirely, plus contractspec, wasmsections and roughly half
# the test suite. A CMake build that quietly compiles thirty files fewer than
# the qmake one is worse than no CMake build, because it goes green.
#
# So there is one list. This parses it.
#
# The grammar is small and stable: a `NAME += \` line, then indented
# `    $$PWD/path \` continuation lines, ending at the first line without a
# trailing backslash. Nothing here tries to be a general qmake parser — it
# handles exactly what these files contain, and says so loudly if that changes.
#
# CMake reruns itself when a CMakeLists.txt changes, not when a file it merely
# file(READ)s does. So a source added to the .pri used to stay out of the build
# until somebody reconfigured by hand, and the build that went green was short
# by that file. Every file read here, and this parser itself, is therefore a
# configure dependency: edit one and the next build reconfigures first.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_FILE}")

# Pulls one `NAME += \ …` block out of `content` into `out_var`, as absolute
# paths. `base` replaces $$PWD and resolves relative entries.
function(_qmake_extract content name base out_var)
    set(collected "")
    set(in_block FALSE)

    string(REPLACE "\r\n" "\n" content "${content}")

    # Two things have to happen before this can be split into lines.
    #
    # A semicolon is CMake's list separator, so any in the text would invent
    # entries. There are none in these files today; the guard costs nothing.
    #
    # And the trailing backslash that continues a qmake line ESCAPES the
    # separator once the string becomes a list — so `foo.cpp \` and the line
    # after it arrive fused into a single element, and `SOURCES += \` arrives
    # glued to its first file. That is what made the first version of this
    # parser return nothing at all. Turn the continuation into a marker that
    # survives the split.
    string(REPLACE ";" "@SEMI@" content "${content}")
    # Trailing whitespace after the backslash is legal and does happen, so the
    # pattern has to allow it — one line in this suite has a space there, and
    # without this it kept its backslash and reached CMake as a broken escape.
    string(REGEX REPLACE "\\\\[ \t]*\n" " @CONT@\n" content "${content}")
    string(REPLACE "\n" ";" lines "${content}")

    foreach(line IN LISTS lines)
        string(STRIP "${line}" trimmed)

        if(NOT in_block)
            # `SOURCES += \`, `SOURCES *= \` or `SOURCES = \`, alone on a line.
            if(trimmed MATCHES "^${name}[ \t]*[*+]?=[ \t]*@CONT@$")
                set(in_block TRUE)
            endif()
            continue()
        endif()

        # A comment or a blank line inside a block ends it: the lists in these
        # files never contain either, so seeing one means the shape changed.
        if(trimmed STREQUAL "" OR trimmed MATCHES "^#")
            set(in_block FALSE)
            continue()
        endif()

        set(entry "${trimmed}")
        string(REGEX REPLACE "[ \t]*@CONT@$" "" entry "${entry}")
        string(REPLACE "@SEMI@" ";" entry "${entry}")
        string(STRIP "${entry}" entry)
        string(REPLACE "$$PWD" "${base}" entry "${entry}")

        if(NOT entry STREQUAL "")
            # The .pri writes $$PWD/…; the test .pro writes plain relative
            # paths. Both mean "relative to the file they are written in".
            if(NOT IS_ABSOLUTE "${entry}")
                set(entry "${base}/${entry}")
            endif()
            list(APPEND collected "${entry}")
        endif()

        # No continuation marker: that was the last entry of the block.
        if(NOT trimmed MATCHES "@CONT@$")
            set(in_block FALSE)
        endif()
    endforeach()

    set(${out_var} "${collected}" PARENT_SCOPE)
endfunction()

# Reads SOURCES and HEADERS from `pri_file`, including the optional QML layer
# when WITH_QML is given. Sets <prefix>_SOURCES and <prefix>_HEADERS.
function(read_qmake_lists pri_file prefix)
    cmake_parse_arguments(ARG "WITH_QML" "" "" ${ARGN})

    if(NOT EXISTS "${pri_file}")
        message(FATAL_ERROR "read_qmake_lists: ${pri_file} does not exist")
    endif()
    get_filename_component(base "${pri_file}" DIRECTORY)
    file(READ "${pri_file}" content)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${pri_file}")

    # The QML layer sits inside `contains(DEFINES, STELLAR_QT_QML) { … }` at the
    # end of the file. Split there so the two halves are parsed separately: the
    # block has its own SOURCES/HEADERS and they must not leak into a build that
    # did not ask for them.
    set(qml_marker "contains(DEFINES, STELLAR_QT_QML)")
    string(FIND "${content}" "${qml_marker}" qml_at)
    if(qml_at EQUAL -1)
        message(FATAL_ERROR
            "read_qmake_lists: ${pri_file} no longer has the '${qml_marker}' "
            "block. The QML layer moved and this parser is now silently "
            "dropping it — fix this rather than the symptom.")
    endif()
    string(SUBSTRING "${content}" 0 ${qml_at} core_part)
    string(SUBSTRING "${content}" ${qml_at} -1 qml_part)

    _qmake_extract("${core_part}" "SOURCES" "${base}" sources)
    _qmake_extract("${core_part}" "HEADERS" "${base}" headers)

    if(ARG_WITH_QML)
        _qmake_extract("${qml_part}" "SOURCES" "${base}" qml_sources)
        _qmake_extract("${qml_part}" "HEADERS" "${base}" qml_headers)
        list(APPEND sources ${qml_sources})
        list(APPEND headers ${qml_headers})
    endif()

    # A silently short list is the failure this whole file exists to prevent.
    list(LENGTH sources source_count)
    if(source_count LESS 100)
        message(FATAL_ERROR
            "read_qmake_lists: only ${source_count} sources came out of "
            "${pri_file}. That is far too few — the list format changed and "
            "this parser is dropping files.")
    endif()

    # Every path must exist. A typo in the .pri is a qmake warning and a CMake
    # link error much later; catching it at configure time names the file.
    foreach(path IN LISTS sources headers)
        if(NOT EXISTS "${path}")
            message(FATAL_ERROR
                "read_qmake_lists: ${path} is listed in ${pri_file} but does "
                "not exist")
        endif()
    endforeach()

    set(${prefix}_SOURCES "${sources}" PARENT_SCOPE)
    set(${prefix}_HEADERS "${headers}" PARENT_SCOPE)
endfunction()

# The same, for a test .pro. The suite lists its test classes under HEADERS,
# because each one is a header-only QObject that the collector picks up.
function(read_qmake_headers pro_file base prefix)
    if(NOT EXISTS "${pro_file}")
        message(FATAL_ERROR "read_qmake_headers: ${pro_file} does not exist")
    endif()
    file(READ "${pro_file}" content)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${pro_file}")

    _qmake_extract("${content}" "HEADERS" "${base}" headers)
    _qmake_extract("${content}" "SOURCES" "${base}" sources)

    list(LENGTH headers header_count)
    if(header_count LESS 50)
        message(FATAL_ERROR
            "read_qmake_headers: only ${header_count} test classes came out "
            "of ${pro_file}. A test that is not compiled is not a test that "
            "passes — it is one nobody notices is gone.")
    endif()

    set(${prefix}_HEADERS "${headers}" PARENT_SCOPE)
    set(${prefix}_SOURCES "${sources}" PARENT_SCOPE)
endfunction()

# Reads the SDK version out of the .pri's STELLAR_QT_SDK_CLIENT_VERSION define
# into `out_var` (e.g. 1.0.4). The version the SDK reports to Horizon and the one
# an installed CMake package reports to find_package() must be the same number;
# typed twice, the CMake one was the copy a release would forget to bump.
#
# The whole value, and only an x.y.z one. This used to keep the x.y.z it
# found at the start and drop the rest, and the CMakeLists rebuilds the
# client-version define from it: a .pri saying "1.1.0-rc1" gave qmake builds
# that report 1.1.0-rc1 and CMake builds that report 1.1.0, from one tree and
# with no warning. project(VERSION) cannot carry a suffix, and rewrites a
# number with a leading zero (01.0.4 becomes 1.0.4), so a version that is not
# exactly x.y.z with plain numbers is refused here rather than shortened.
# Checked by the ReadQmakeVersion test (see the end of this file).
function(read_qmake_version pri_file out_var)
    if(NOT EXISTS "${pri_file}")
        message(FATAL_ERROR "read_qmake_version: ${pri_file} does not exist")
    endif()
    file(STRINGS "${pri_file}" lines
         REGEX "^[ \t]*DEFINES[ \t]*[*+]?=[ \t]*STELLAR_QT_SDK_CLIENT_VERSION=")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${pri_file}")

    list(LENGTH lines line_count)
    if(NOT line_count EQUAL 1)
        message(FATAL_ERROR
            "read_qmake_version: expected exactly one "
            "'DEFINES += STELLAR_QT_SDK_CLIENT_VERSION=…x.y.z…' line in "
            "${pri_file}, found ${line_count}. The version is read from there "
            "so that CMake and qmake cannot disagree about it.")
    endif()

    # Everything after the '=' up to a qmake comment, with qmake's quoting
    # (\"\\\"1.0.4\\\"\") taken off: what is left is the string the qmake
    # build reports.
    string(REGEX MATCH "STELLAR_QT_SDK_CLIENT_VERSION=([^#]*)" _stellar_match "${lines}")
    set(version "${CMAKE_MATCH_1}")
    string(REPLACE "\\" "" version "${version}")
    string(REPLACE "\"" "" version "${version}")
    string(STRIP "${version}" version)
    if(NOT version MATCHES "^(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)$")
        message(FATAL_ERROR
            "read_qmake_version: ${pri_file} sets STELLAR_QT_SDK_CLIENT_VERSION "
            "to '${version}', which is not exactly x.y.z. A CMake project "
            "version has no room for anything else, and a CMake build that "
            "reported only part of it would tell Horizon a different client "
            "version from the qmake build of the same tree. Use x.y.z "
            "(numbers without leading zeros).")
    endif()
    set(${out_var} "${version}" PARENT_SCOPE)
endfunction()

# ─── The ReadQmakeVersion test ───────────────────────────────────────
#
# Run as a script, this file checks read_qmake_version (the CMakeLists
# registers it with add_test):
#
#     cmake -DREAD_QMAKE_VERSION_SELFTEST=<scratch dir> -P ReadQmakeLists.cmake
#
# writes a one-line .pri per case, reads each in a child cmake (a refusal is a
# FATAL_ERROR, which only a separate process can observe) and fails unless
# every version is read whole or refused as expected.
if(CMAKE_SCRIPT_MODE_FILE STREQUAL CMAKE_CURRENT_LIST_FILE)
    if(DEFINED READ_QMAKE_VERSION_CHECK)
        read_qmake_version("${READ_QMAKE_VERSION_CHECK}" _version)
        message(STATUS "READ_QMAKE_VERSION=${_version}")
    elseif(DEFINED READ_QMAKE_VERSION_SELFTEST)
        file(MAKE_DIRECTORY "${READ_QMAKE_VERSION_SELFTEST}")
        # "what the .pri says|what must be read", nothing after the bar for a
        # refusal. The suffixed one used to be read as 1.0.4.
        set(_cases
            "1.0.4|1.0.4"
            "10.20.30|10.20.30"
            "1.0.4\" # released|1.0.4"
            "1.0.4-rc1|"
            "1.0|"
            "1.0.4.1|"
            "01.0.4|")
        set(_failures 0)
        foreach(_case IN LISTS _cases)
            string(FIND "${_case}" "|" _bar)
            string(SUBSTRING "${_case}" 0 ${_bar} _written)
            math(EXPR _after "${_bar} + 1")
            string(SUBSTRING "${_case}" ${_after} -1 _expected)
            # As the .pri writes it: DEFINES += …VERSION=\"\\\"1.0.4\\\"\"
            if(_written MATCHES "^([^\"]*)\"(.*)$")
                set(_line "DEFINES += STELLAR_QT_SDK_CLIENT_VERSION=\\\"\\\\\\\"${CMAKE_MATCH_1}\\\\\\\"\\\"${CMAKE_MATCH_2}")
            else()
                set(_line "DEFINES += STELLAR_QT_SDK_CLIENT_VERSION=\\\"\\\\\\\"${_written}\\\\\\\"\\\"")
            endif()
            set(_pri "${READ_QMAKE_VERSION_SELFTEST}/version.pri")
            file(WRITE "${_pri}" "QT += network\n${_line}\n")
            execute_process(
                COMMAND "${CMAKE_COMMAND}" "-DREAD_QMAKE_VERSION_CHECK=${_pri}"
                        -P "${CMAKE_CURRENT_LIST_FILE}"
                RESULT_VARIABLE _rc OUTPUT_VARIABLE _out ERROR_VARIABLE _err)
            set(_read "")
            if(_rc EQUAL 0 AND _out MATCHES "READ_QMAKE_VERSION=([^\n]*)")
                set(_read "${CMAKE_MATCH_1}")
            endif()
            if(_expected STREQUAL "")
                if(_rc EQUAL 0)
                    message(SEND_ERROR "'${_line}' was read as '${_read}', not refused")
                    math(EXPR _failures "${_failures} + 1")
                endif()
            elseif(NOT _rc EQUAL 0 OR NOT _read STREQUAL _expected)
                message(SEND_ERROR "'${_line}' was read as '${_read}' (exit ${_rc}), "
                                   "not as '${_expected}'\n${_err}")
                math(EXPR _failures "${_failures} + 1")
            endif()
        endforeach()
        list(LENGTH _cases _count)
        if(_failures GREATER 0)
            message(FATAL_ERROR "ReadQmakeVersion: ${_failures} of ${_count} cases wrong")
        endif()
        message(STATUS "ReadQmakeVersion: all ${_count} cases right")
    endif()
endif()
