#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <math.h>
#include <map>
#include <set>

#include "commit.h"
#include "hash.h"
#include "dirent.h"
using namespace std;

bool Commit::update(){
    int size = 0;
    for(int i = 0; i < 5; ++i){
        size += stat[i].size();
    }
    return (size > 0);
}

uint32_t Commit::getSize(){
    uint32_t ret = 32;
    for(int i = 0; i < 5; ++i){
        for(int j = 0; j < stat[i].size(); ++j){
            ret = ret + 1 + stat[i][j].size();
        }
    }
    for(stringmap::iterator it = cur_file.begin(); it != cur_file.end(); ++it){
        string name = it->first;
        ret = ret + 16 + 1 + name.size();
    }
    return ret;
}

void Commit::addCurFile(string name, string md5){
    cur_file.emplace(name, md5);
}

void Commit::removeCurFile(string name){
    cur_file.erase(name);
}

void Commit::printStat(bool md5, int ncom){
    if(md5)
        cout << "# commit " << dec << ncom << '\n';
    cout << "[new_file]\n";
    for(int i = 0; i < stat[newfile].size(); ++i)
        cout << stat[newfile][i] << '\n';
    
    cout << "[modified]\n";
    for (int i = 0; i < stat[modified].size(); ++i)
        cout << stat[modified][i] << '\n';
    
    cout << "[copied]\n";
    for (int i = 0; i < stat[copied].size(); ++i)
        cout << stat[copied][i] << " => " << stat[copied_to][i] << '\n';

    cout << "[deleted]\n";
    for (int i = 0; i < stat[deleted].size(); ++i)
        cout << stat[deleted][i] << '\n';
    
    if (md5){
        cout << "(MD5)\n";
        for(stringmap::iterator it = cur_file.begin(); it != cur_file.end(); ++it){
            cout << it->first << ' ';
            cout << it->second << '\n';
        }
    }

    cout << "(timestamp)\n";
    cout << dec << getTime() << "000\n";
}

void Commit::forward(fstream& fs, int ncom){
    writeNum32(fs, ncom);
    writeNum32(fs, cur_file.size());
    writeNum32(fs, stat[0].size());
    writeNum32(fs, stat[1].size());
    writeNum32(fs, stat[2].size());
    writeNum32(fs, stat[3].size());
    writeNum32(fs, getSize());

    int size;
    for(int i = 0; i < 4; ++i){
        for(int j = 0; j < stat[i].size(); ++j){
            size = stat[i][j].size();
            fs << (unsigned char)size;
            for(int k = 0; k < size; ++k){
                fs << stat[i][j][k];
            }
        }
        if(i == 2){
            for(int j = 0; j < stat[4].size(); ++j){
                size = stat[4][j].size();
                fs << (unsigned char)size;
                for(int k = 0; k < size; ++k){
                    fs << stat[4][j][k];
                }
            }
        }
    }
    for(stringmap::iterator it = cur_file.begin(); it != cur_file.end(); ++it){
        string name = it->first;
        string md5 = it->second;
        fs << (unsigned char)name.size();
        for(int j = 0; j < name.size(); ++j){
            fs << name[j];
        }
        writeMD5(fs, md5);
    }
    uint32_t time_stamp = (uint32_t)time(NULL);
    writeNum32(fs, time_stamp);
}

void Commit::writeNum32(fstream& fs, uint32_t num)
{
    for(int i = 0; i < 4; ++i){
        uint32_t byte = (num >> (i * 8)) & 0xFF;
        fs << (unsigned char)byte;
    }
}

void Commit::writeMD5(fstream& fs, string s){
    unsigned char byte = 0;
    for(int i = 0; i < 32; ++i){
        if(s[i] >= '0' && s[i] <= '9')
            byte += (s[i] - '0') * pow(16, ((i+1) % 2));
        else
            byte += (s[i] - 'a' + 10) * pow(16, ((i + 1) % 2));
        if(i % 2){
            fs << byte;
            byte = 0;
        }
    }
}
