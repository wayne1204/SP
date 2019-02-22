#include <iostream>
#include <stdlib.h>  
#include <string>
#include <fstream>


#include "hash.h"
#include "loser.h"
using namespace std;


int main(int argc, char** argv){
    if(argc < 2 || argc > 4){
        cerr << "[Error] wrong # of command\n";
        return -1;
    }
    string command = argv[1];
    Loser mgr = Loser();

    if (command == "log" && argc == 4)
    {
        int times = atoi(argv[2]);
        string dir = argv[3];
        mgr.logging(times, dir);
    }
    else if (command == "status" ){
        string dir = argv[2];
        mgr.status(dir);
    }
    else if (command == "commit")
    {
        string dir = argv[2];
        mgr.commit(dir);
    }
    return 0;
}
