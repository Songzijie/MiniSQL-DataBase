/*
 * by Zijie Song
 */

#include "Interpreter.h"

using namespace std;

int main(int argc, char *argv[]) {
    cout<<  "    __  __ _       _ ____   ___  _\n"
            "   |  \\/  (_)_ __ (_) ___| / _ \\| |\n"
            "   | |\\/| | | __ \\| \\___ \\| | | | |\n"
            "   | |  | | | | | | |___) | |_| | |___\n"
            "   |_|  |_|_|_| |_|_|____/ \\__\\_\\_____|\n";

    cout << endl;
    cout << "==================Author================= " << endl;
    cout << "              Zijie Song " << endl;
    cout << "========================================== " << endl<< endl;
    Interpreter In;
    In.pipeline();
    return 0;
}