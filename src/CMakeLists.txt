cmake_minimum_required (VERSION 2.8)

project( HCLang )

# USING c++11 and includes
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -fPIC" )
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pedantic -g" )
include_directories(${HCLang_SOURCE_DIR}/)

# LLVM CONFIG
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs core mcjit native irreader all)


# SOURCES

set ( HCLANG_SRCS
    frontend/lexer.cpp
    frontend/parser.cpp

    ast/ast.cpp
    ast/ast.print.cpp
    base.cpp
    printer.cpp

    stexecutor.cpp
    mtexecutor.cpp

    ast/declarations/system.cpp
    ast/declarations/slot.cpp
    ast/declarations/procedure.cpp
    ast/declarations/function.cpp

    ast/types/array.cpp
    ast/types/tuple.cpp
    ast/types/union.cpp
    ast/types/string.cpp

    ast/statements/for.cpp
    ast/statements/var.cpp
    ast/statements/let.cpp
    ast/statements/send.cpp
    ast/statements/expr.cpp
    ast/statements/return.cpp

    ast/expressions/binop.cpp
    ast/expressions/call.cpp
    ast/expressions/cond.cpp
    ast/expressions/getchild.cpp

    corelib/stdout.cpp
    corelib/async_io.cpp
)

set ( HCLANG_HEADERS
    frontend/lexer.h
    frontend/parser.h

    ast/ast.h
    base.h
    printer.h

    ast/declarations/system.h
    ast/declarations/slot.h
    ast/declarations/procedure.h
    ast/declarations/function.h

    ast/types/array.h
    ast/types/tuple.h
    ast/types/union.h
    ast/types/string.h

    ast/statements/for.h
    ast/statements/var.h
    ast/statements/let.h
    ast/statements/send.h
    ast/statements/expr.h
    ast/statements/return.h

    ast/expressions/binop.h
    ast/expressions/call.h
    ast/expressions/cond.h
    ast/expressions/getchild.h

    corelib/stdout.h
    corelib/async_io.h
)

add_library( HCLang STATIC ${HCLANG_SRCS} ${HCLANG_HEADERS} )


# AST codegen debug

add_custom_command(
    OUTPUT decl.ll
    COMMAND ${CMAKE_C_COMPILER} -cc1 -emit-llvm -o decl.ll ${CMAKE_CURRENT_SOURCE_DIR}/base.h -DEXPORTLL
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/base.h
)

add_custom_command(
    OUTPUT decl.cpp
    COMMAND xxd -i decl.ll decl.cpp
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/decl.ll
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable( lexdebug lexdebug_main.cpp)
target_link_libraries( lexdebug HCLang ${llvm_libs} )

add_executable( pardebug pardebug_main.cpp)
target_link_libraries( pardebug HCLang ${llvm_libs} )

# TODO move out
find_library(AIO_LIBRARY
        NAMES aio
        )

if (AIO_LIBRARY)
    message(STATUS "AIO Library found: ${AIO_LIBRARY}")
else()
    message(FATAL_ERROR "AIO Library not found")
endif()


add_executable( astdebug astdebug_main.cpp decl.cpp)
target_link_libraries( astdebug HCLang ${AIO_LIBRARY} ${llvm_libs} rt pthread )
