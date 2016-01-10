---
layout: page
title: About
---

## Why a new language?
I've programmed mostly in C++, Java and Python. Below is my personal opinion based on experience with these languages.

I love to run well-written C++ software, because its very fast and it has low memory footprint. C++11 also brough a lot of useful features. However, writing in C++ is still quite tedious, and C++ is very error prone. So, an inexperienced use of C++ could easily make more harm than good. 

Programming in Java is quite easy, but I personally hate to run Java software (slow startup, consumes huge amount of my precious memory). By default, for an each Java application a separate JVM (Java virtual machine) needs to be created. A JVM consumes 50~100 MB of memory, which makes Java impractical for tiny applications.

Programming in Python is extremely easy, because Python offers a lot of features for great expressivity. I also like forced indentation for blocks, which makes code look cleaner.

|----------|-----------|------------|--------------|----------|-------------|-------------------------|
| Language | Speed     | Memory     | Expressivity | Safety   | Concurrency | Parallelization         |
|----------|-----------|------------|--------------|----------|-------------|-------------------------|
| C++      | **Fast**  | **Small**  | Low          | Low      | 3rd party   | Yes (libraries, unsafe) |
| C++11    | **Fast**  | **Small**  | Middle       | Low      | Little      | Yes (unsafe)            |
| Java     | **Fast**  | Huge       | Low          | **High** | Little      | Yes (unsafe)            |
| Python   | Slow      | Decent     | **High**     | **High** | Little      | Yes (not CPython)       |
|----------|-----------|------------|--------------|----------|-------------|-------------------------|
{: .styled-table .green-strong .centered .hower-row :}

By unsafe parallelization I mean, that it is easily possible to write thread-unsafe code, if programmer didn't care about parallelization.


### Concurency is important

*TODO*

### Parallelization is needed

*TODO*

### The goal(s)

*TODO*

## How to realize the goal?

*TODO*
