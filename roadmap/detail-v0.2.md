---
layout: page
title: Version 0.2 detail
---
This development release is focused on full featured non-blocking concurrent programming.

## Concurency Model

Concurency model is based on [Actor model](https://en.wikipedia.org/wiki/Actor_model), and extended with features for easy construction of asynchronous sequential processes.

### Current design

**Shared state is encapsulated in Systems**. System can be considered as an object with following characteristics:

 * it can store internal variables
 * it has slots
    * each slot is identified by a name
    * message handling is atomically executed

Slots are defined by:

 * input arguments
 * non-blocking code - code that is executed without blocking
 * optional return type - for synchronized procedures

Concurrency model also supports **procedures to allow easy implementation of processes and tasks in background**. There are two types of procedures - **synchronous and asynchronous**.

Procedures are defined by:

 * input arguments
 * code
    * synchronous procedure:
        * can execute only synchronous operations
        * can call only synchronous procedures, but can spawn asynchronous procedure without waiting for a result
    * asynchronous procedure can execute also asynchronous operations and
        * can use asynchronous operations - waiting for return value of slot
        * can use asynchronous procedures
 * return value

### Future extensions

It is planned to extend concurrency model features to support system interfaces and various types executors (planning, execution targets - CPU, GPU, [HSA](https://en.wikipedia.org/wiki/Heterogeneous_System_Architecture)).

## Functional requirements

### REQ01: Concurrency

Implementation of concurrent model. Following features are to be supported:

 * system definition:
 * asynchronous processing of messages on fixed count of threads
 * multithreaded execution
 * synchronous and asynchronous procedures

### REQ02: Data types

Following value types are to be supported:

 * int - regular 32-bit integer
 * String - reference-counted C-string

Aggregate types:

 * array
 * tuple - almost like a C-struct

Encapsulation types:

 * rc - reference counted malloc'd value
 * ... todo ...

### REQ03: Constructs

Following constructs are to be supported:

 * Expressions:
    * basic arithmetics: +, -, \*, /
    * string concat: +
    * system instantiation: spawn \<system type>
 * Assign statement
 * Control structures:
    * conditional branches: if, if-else, if-elif, if-elif-else
    * for iteration cycle: for \<item> in \<array>
    * while loop: while \<cond>


### REQ04: Standard I/O

The language should implement a way to work with standard I/O. Can be hardcoded into language and may not be properly designed.

### REQ05: Compiler usage

Compiler can compile a single file containing a code. Optional LLVM IR dumping can be turned on by commandline option.
