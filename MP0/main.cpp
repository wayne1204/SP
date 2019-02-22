#include <iostream>
#include <unordered_set>
#include <fstream>
#include "fileMgr.h"
using namespace std;

int main(int argc, char** argv){
    fileManager* mgr = new fileManager();
    mgr->parseSet(argv[1]);
    ios_base::sync_with_stdio(false);

    //from FILE
    if (argc == 3){ 
        fstream fs(argv[2], ios::in);
        if (fs){
            mgr->readFiles(fs);
        }
        else{
            cerr << "error\n";
            return -1;
        }
    }

    // from STD IN
    else{  
        mgr->readFiles(cin);
    }
    mgr->show();
    return 0;
}



