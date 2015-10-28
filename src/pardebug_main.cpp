#include <iostream>
#include <map>

#include "parser.h"
#include "printer.h"

using namespace std;

int main(int argc, char **argv)
{
    vector<Token> tokens = lexerFile(argv[1]);

    Parser parser(tokens, &std::cout);
    StatementList * list = parser.file();

    Printer p(std::cout);
    cout << "Statements " << list->size() << endl;
    for( Statement *s : *list )
        s->print(p);

    return 0;
}
