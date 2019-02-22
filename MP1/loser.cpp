#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <map>
#include <set>

#include "loser.h"
#include "hash.h"
#include "dirent.h"
using namespace std;

#define nfile_o 4
#define add_o 8
#define nmodify_o 12
#define ncopy_o 16
#define ndelete_o 20
#define size_o 24

#define Byte_size 256
typedef std::map<std::string, std::string> stringmap;

enum fileStat{
    newfile,
    modified,
    copied,
    deleted,
    copied_to
};

uint32_t Commit::getSize(){
    uint32_t ret = 28;
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


void Commit::printStat(bool md5){
    if(md5)
        cout << "# commit " << dec << getNumCommit() << '\n';
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

    // cout << "(timestamp)\n";
    // cout << dec << getTime() << "000\n";
}

bool Loser::readRecord(string dir){
    dir += "/.loser_record";
    fstream fs(dir, ios::in | ios::binary);
    if (!fs.is_open()){
        return false;
    }

    // get file size (Bytes)
    fs.seekg(0, ios::end);
    file_size = fs.tellg();
    buffer = new char[file_size+1];
    buffer[file_size] = '\0';
    if(!file_size)
        return false;

    // binary read
    fs.seekg(0, ios::beg);
    fs.read(buffer, file_size);
    fs.close();
    splitCommit();
    // cur_commit->setNum(split.size());
    return true;
}

void Loser::splitCommit(){
    uint32_t ncom = 1, begin = 0, com_size = 0;
    string s;

    while (begin + com_size < file_size)
    {
        int cur = begin + 28;
        Commit* com = new Commit();
        com->setNum(ncom);
        com_size = getNum32(begin + size_o);
        // parse add, mod, copy, del
        for(int i = 0; i < 4; ++i){
            for(int j = 0; j < getNum32(begin + 4*i + 8); ++j){
                s = getName(cur);
                com->addFileName(i, s);
                // case: copy
                if(i == 2){
                    s = getName(cur);
                    com->addFileName(4, s);
                }
            }
        }
        
        // current directory file
        for(int i = 0; i < getNum32(begin + nfile_o); ++i){
            string fname = getName(cur);
            stringstream ss;
            for(int j = 0; j < 16; ++j, ++cur){
                uint8_t c = buffer[cur];
                ss << setw(2) << setfill('0') << hex << (int)c;
            }
            com->addCurFile(fname, ss.str());
        }

        // uint32_t s = getNum32(cur);
        // com->setTime(s);
        begin += com_size;
        ++ncom;
        split.push_back(com);
    }
}


void Loser::compare(bool success){
    stringmap old_files;
    // if read .loser_record success => not first commit
    if(success)
        old_files = split.back()->getMap();

    for (stringmap::iterator iter = old_files.begin(); iter != old_files.end();)
    {
        stringmap::iterator it = table.find(iter->first);
        if(it != table.end()){
            if (it->second != iter->second)   // modified
                cur_commit->addFileName(modified, iter->first);

            isTake.erase(iter->first);
            ++iter;
        }
        else{
            // delete
            cur_commit->addFileName(deleted, iter->first);
            iter = old_files.erase(iter);
        }
    }

    for (stringmap::iterator it = old_files.begin(); it != old_files.end();++it){
        for (set<string>::iterator iter = isTake.begin(); iter != isTake.end();)
        {
            string md5 = table.find(*iter)->second;
            if (md5 == it->second){
                // copied
                cur_commit->addFileName(copied, it->first);
                cur_commit->addFileName(copied_to, *iter);
                iter = isTake.erase(iter);
            } 
            else    
                ++iter;
        }
    }
    for (set<string>::iterator iter = isTake.begin(); iter != isTake.end(); ++iter)
        cur_commit->addFileName(newfile, *iter);
}


bool Loser::logging(int times, string dir){
    if(!readRecord(dir))
        return false;
    uint32_t num = 0;

    int n = split.size();
    int loop = times < n ? times : n;
    for(int i = 0; i < loop; ++i){
        if(i != loop-1)
            cout << "\n";
        split[i]->printStat(true);
    }
    delete [] buffer;
    return true;
}

bool Loser::status(string path){
    bool success = readRecord(path);
    listFiles(path);
    compare(success);
    cur_commit->printStat(false);
    return true;
}

bool Loser::commit(string path){
    bool success = readRecord(path);
    listFiles(path);
    compare(success);
    fstream fs(path + "/.loser_record", ios::out | ios::binary | ios::app);
    fs.seekp(0, ios::end);

    vector<vector<string> >stat = cur_commit->getStat();

    writeNum32(fs, split.size() + 1);
    writeNum32(fs, table.size());
    writeNum32(fs, stat[0].size());
    writeNum32(fs, stat[1].size());
    writeNum32(fs, stat[2].size());
    writeNum32(fs, stat[3].size());
    writeNum32(fs, cur_commit->getSize());

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
    for(stringmap::iterator it = table.begin(); it != table.end(); ++it){
        string name = it->first;
        string md5 = it->second;
        fs << (unsigned char)name.size();
        for(int j = 0; j < name.size(); ++j){
            fs << name[j];
        }
        writeMD5(fs, md5);
    }
    return true;
}

// **************************************************
// *************[ utility function ]*****************
// **************************************************
void Loser::writeNum32(fstream& fs, uint32_t num)
{
    for(int i = 0; i < 4; ++i){
        uint32_t byte = (num >> (i * 8)) & 0xFF;
        fs << (unsigned char)byte;
    }
}

void Loser::writeMD5(fstream& fs, string s){
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

uint32_t Loser::getNum32(int pos){
    uint32_t s = 0;
    for(int i = 0; i < 4; ++i){
        uint8_t bit = buffer[pos + i];
        s += pow(Byte_size, i) * bit;
    }
    return s;
}

// from name size(8-bit) get file name
string Loser::getName(int& cur){
    string str;
    uint8_t name_size = buffer[cur++];
    for(int i = 0; i < name_size; ++i, ++cur){
        str += buffer[cur];
    }
    return str;
}

void Loser::listFiles(string path){
    DIR* dir;
    struct dirent *ent;
    set<string> exclude {".", "..", ".loser_record"};
    
    if ((dir = opendir(path.c_str())) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            string name = ent->d_name;
            if(exclude.find(name) == exclude.end()){
                fstream fs(path + '/' + name, ios::in);
                string content((istreambuf_iterator<char>(fs)),
                               (istreambuf_iterator<char>()));
                MD5 md5mgr(content);
                isTake.emplace(name);
                table.emplace(name, md5mgr.md5());
                cur_commit->addCurFile(name, md5mgr.md5());
            }
        }
        closedir(dir);
    }
}
