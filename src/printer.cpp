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
