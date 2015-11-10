/*
 * HCLang - Highly Concurrent Language
 * Copyright (c) 2015 Miroslav Kravec
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include "printer.h"

#include <iostream>
#include <cstdlib>

Printer::Printer(std::ostream &out) : out(out) {
    _indent_max = (char*)malloc( sizeof(char) * ( 128 + 1) );
    _indent = _indent_max + 128;
    _indent_capacity = 128;
    int i = 0;
    for( ; i < _indent_capacity; i++ )
        _indent_max[i] = ' ';
    _indent_max[i] = 0;
};

void Printer::beginBlock(std::string name) {
    println(name + ":");
    indent();
}

void Printer::endBlock() {
    deindent();
}

void Printer::println(std::string val) {
    out << _indent << val << std::endl;
}

void Printer::indent() {
    _indent -= 2;
    if( _indent < _indent_max ) {
        free(_indent_max);
        _indent_max = (char*)malloc( _indent_capacity + 128 + 1);
        _indent = _indent_max + 128 - 2;
        _indent_capacity += 128;
        int i = 0;
        for( ; i < _indent_capacity; i++ )
            _indent_max[i] = ' ';
        _indent_max[i] = 0;
    }
}

void Printer::deindent() {
    _indent += 2;
    if( _indent_max + _indent_capacity < _indent )
        _indent = _indent_max + _indent_capacity;
}
