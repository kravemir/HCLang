cmake_minimum_required (VERSION 2.6)

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

    ast/array.cpp
    ast/tuple.cpp
    ast/union.cpp
    ast/system.cpp

    ast/slot.cpp
    ast/procedure.cpp

    ast/stmt_let.cpp
    ast/stmt_send.cpp
    ast/stmt_expr.cpp

    ast/expr_binop.cpp
)


add_library( HCLang ${HCLANG_SRCS} )


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
