
cmake_minimum_required (VERSION 3.13)

project (compilium
         LANGUAGES C)

enable_testing ()

function (setvar_default var_)
    if (NOT DEFINED ${var_})
        set (${var_} ${ARGN} PARENT_SCOPE)
    endif ()
endfunction ()

setvar_default (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
setvar_default (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
setvar_default (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set (HEADERS compilium.h)
set (SOURCES ast.c type.c parser.c compilium.c)

set (COMPILER_NAME compilium)

add_executable (${COMPILER_NAME} ${SOURCES} ${HEADERS})
target_compile_features (${COMPILER_NAME} PRIVATE c_std_11)
foreach (w_ IN ITEMS all pedantic extra conditional-uninitialized)
    target_compile_options (${COMPILER_NAME} PRIVATE -W${w_})
endforeach ()
foreach (s_ IN ITEMS address undefined)
    target_compile_options (${COMPILER_NAME} PRIVATE -fsanitize=${s_})
    target_link_options (${COMPILER_NAME} PRIVATE -fsanitize=${s_})
endforeach ()

add_test (NAME list-test
          COMMAND ${COMPILER_NAME} --run-unittest=List)

add_test (NAME type-test
          COMMAND ${COMPILER_NAME} --run-unittest=Type)

add_test (NAME all-test
          COMMAND env COMPILER_BINARY=$<TARGET_FILE:${COMPILER_NAME}> HOST_CC=${CMAKE_C_COMPILER} ./test.sh
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
