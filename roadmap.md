---
layout: page
title: RoadMap
---

Roadmap for the development. 

Feature list for future versions will be updated and expanded.


## Current and planned goals

### v0.2 - Parallelization

Sub-goal of the 1st primary goal.
Features:

 * threaded execution of programs
 * parallel processing of system messages
 * parallel execution of procedures
 * communication between systems

[Details](roadmap/detail-v0.2)

### v0.3 - Types and Basic Memory Management
Features:

 * allocation of data on stack
 * dynamic (de)allocation of data
 * multiple storage options:
   *  C-struct like
   *  dynamic with unique pointer
   *  dynamic with reference counting

### v0.4 - Concurrency, I/O
Features:
 * synchronization of procedures - await
 * I/O: std in/out, files

### v0.5 - Expressivity extensions
The 2nd primary goal. 
Features:

 * match statements
 * closures
 * type inference where possible


## Future goals

### Advanced Memory management
Sub-goal of 3rd primary goal.

Memory management features for better parallelization possibilities.
Targeted for distributed execution.

### Friendly compiler
Features:

 * nice error reporting
 * type checking

### Intermediate representation

### Distributed execution

### Code organization (packages,libraries,...)

### Standard Library


## Previous versions

### v0.1 - Prototype
Features:

 * prototype lexer, parser, AST
 * translation of AST to LLVM IR
