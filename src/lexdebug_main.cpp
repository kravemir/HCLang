#include <cctype>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "lexer.h"

using namespace std;

int main(int argc, char **argv) {
    vector<Token> tokens = lexerFile(argv[1]);

    for(auto t : tokens) {
        printToken(t);
        cout << ", ";
    }

    cout << endl;

    return 0;
}
