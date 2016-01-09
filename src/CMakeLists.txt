cmake_minimum_required (VERSION 3.0)

project( HCLang )

# USING c++11 and includes
add_definitions( -Wall -g --std=c++11 -fPIC )
include_directories(${HCLang_SOURCE_DIR}/)

# LLVM CONFIG
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs core mcjit native irreader)


# SOURCES

set ( HCLANG_SRCS
    ast/ast.cpp
    ast/ast.print.cpp
    lexer.cpp
    parser.cpp
    base.cpp
    printer.cpp

    stexecutor.cpp
    mtexecutor.cpp

    ast/declarations/system.cpp
    ast/declarations/slot.cpp
    ast/declarations/procedure.cpp

    ast/types/array.cpp
    ast/types/tuple.cpp
    ast/types/union.cpp

    ast/statements/for.cpp
    ast/statements/var.cpp
    ast/statements/let.cpp
    ast/statements/send.cpp
    ast/statements/expr.cpp
    ast/statements/return.cpp

    ast/expressions/binop.cpp
    ast/expressions/call.cpp
    ast/expressions/cond.cpp
)

set ( HCLANG_HEADERS
    ast/ast.h
    lexer.h
    parser.h
    base.h
    printer.h

    ast/declarations/system.h
    ast/declarations/slot.h
    ast/declarations/procedure.h

    ast/types/array.h
    ast/types/tuple.h
    ast/types/union.h

    ast/statements/for.h
    ast/statements/var.h
    ast/statements/let.h
    ast/statements/send.h
    ast/statements/expr.h
    ast/statements/return.h

    ast/expressions/binop.h
    ast/expressions/call.h
    ast/expressions/cond.h
)

add_library( HCLang ${HCLANG_SRCS} ${HCLANG_HEADERS} )


# AST codegen debug

add_custom_target(
    decl.ll
    COMMAND clang -cc1 -emit-llvm -o decl.ll ${CMAKE_CURRENT_SOURCE_DIR}/base.h -DEXPORTLL
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/base.h
)

add_executable( lexdebug lexdebug_main.cpp)
target_link_libraries( lexdebug HCLang ${llvm_libs} )

add_executable( pardebug pardebug_main.cpp)
add_dependencies( pardebug decl.ll )
target_link_libraries( pardebug HCLang ${llvm_libs} )

add_executable( astdebug astdebug_main.cpp)
add_dependencies( astdebug decl.ll )
target_link_libraries( astdebug HCLang ${llvm_libs} )
