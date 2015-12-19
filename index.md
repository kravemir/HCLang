---
layout: page
---
The HCLang stands for **H**ighly **C**oncurrent **Lang**uage. Therefore it's obvious that primary focus of the language is on massive parallelization, concurrency and distributed computing.

## Goals and Features
There are three primary goals forming the design of the language. It must be:

 1. highly concurrent and distributed
 2. easy, simple and expressive
 3. safe and secure

The language will have these features:

 1. powerful memory control and invariants for safety

## Development
The language is still under heavy development. Roadmap is described in [separate page](roadmap).

## Fast start

Clone, compile and execute examples:
{% highlight bash %}
git clone https://github.com/kravemir/HCLang.git HCLang
mkdir HCLang/src/build
cd HCLang/src/build
cmake ..
make -j2
./astdebug ../../examples/v0.1/01-helloworld.hcl
{% endhighlight %}

Tested on ArchLinux 64-bit with Clang v3.7.0-4, LLVM v3.7.0-4 and CMake v3.3.2-3.
