#ifndef HCLANG_PRINTER_H
#define HCLANG_PRINTER_H

#include <string>
#include <ostream>

class Printer {
    char *_indent, *_indent_max;
    int _indent_capacity;
    std::ostream &out;
public:
    Printer(std::ostream &out);

    void beginBlock(std::string name);
    void endBlock();
    void println(std::string val);

    void indent();
    void deindent();
};

#endif // HCLANG_PRINTER_H
