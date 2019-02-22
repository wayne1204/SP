#include <iostream>
#include <istream>
#include <sstream>
#include "fileMgr.h"

using namespace std;

void reverse(char *s){
    for(int i = 0,j = strlen(s)-1;i<j;i++,j--){
        int c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(int n,char *s){
    int i = 0;
    while(n != 0){
        s[i++] = n%10+'0';
        n = n/10;
    }
    s[i] = '\0';
    reverse(s);
}

void fileManager::parseSet(string s){
    for(int i = 0; i < s.size(); ++i){
        char_set.insert(s[i]);
    }
}

void fileManager::readFiles(istream& fs){
    string line;
    while(getline(fs, line)){
        int cnt = 0;
        char buffer [10];
        for(int i = 0, j = line.size();i < j ;++i){
            if(char_set.find(line[i]) != char_set.end())
                ++cnt;
        }
        itoa(cnt, buffer);
        s += buffer;
        s += '\n';
        // ss << cnt << '\n';
    }
}

void fileManager::show(){
    // for(int i = 0, j = char_count.size(); i < j; ++i){
    //     cout << char_count[i] << '\n';
    // }
    // cout << ss.str(); 
    cout << s;   
}