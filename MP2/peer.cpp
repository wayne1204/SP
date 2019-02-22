#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <sys/socket.h>
#include <sys/un.h>

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <map>
#include <set>

#include "peer.h"
#include "hash.h"
#include "dirent.h"
using namespace std;

void Peer::setHostName(string name){
    string s = "/tmp/mp2-";
    string s2 = ".sock";
    host_socket = s + name + s2;
}

void Peer::addPeers(string& peers)
{
    vector<Commit*> v;
    string s = "/tmp/mp2-";
    string s2 = ".sock";
    peers = s + peers + s2;
    peer_split.emplace(peers,v);
    peer_updated.emplace(peers, false);
}

void Peer::splitCommit()
{
    uint32_t begin = 0, com_size = 0;
    string s;
    while (begin < file_size)
    {
        int cur = begin + 28;
        Commit* com = new Commit();
        com_size = getNum32(begin + size_o);
        // parse add, mod, copy, del
        for(int i = 0; i < 4; ++i){
            for(int j = 0; j < getNum32(begin + 4*i + 8); ++j){
                s = getName(cur);
                com->addFileType(i, s);
                // case: copy
                if(i == 2){
                    s = getName(cur);
                    com->addFileType(4, s);
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
        com->setTime(getNum32(cur));
        begin += com_size;
        split.push_back(com);
    }
}

bool Peer::readRecord(string dir){
    split.clear();
    cur_commit = new Commit();
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
    ++version;
    return true;
}

// access by file system, return true if need to commit files
bool Peer::fsAccess(string src, string dest, bool from_fs){
    string md5;

    if(from_fs){
        if(fileSystem.find(src) == fileSystem.end()){
            return false;
        }
        md5 = fileSystem.find(src)->second;
    }
    else{
        fstream fs(src, ios::in);
        string content((istreambuf_iterator<char>(fs)),
                        (istreambuf_iterator<char>()));
        file_content = content;
        MD5 md5mgr(file_content);
        fstream ofs(dest, ios::out);
        ofs << file_content;
        md5 = md5mgr.md5();
    }

    cur_commit->addCurFile(dest, md5);
    compare();
    if(cur_commit->update()){
        commit(getRepoName());
    }
    return true;
}


void Peer::compare(){
    replayCommits(getRepoName() ,false);
    stringmap fs_files = cur_commit->getMap();

    for (stringmap::iterator iter = fileSystem.begin(); iter != fileSystem.end();++iter)
    {
        stringmap::iterator it = fs_files.find(iter->first);
        if(it != fs_files.end()){
            if (it->second != iter->second)   // modified
                cur_commit->addFileType(modified, iter->first);

            fs_files.erase(iter->first);
        }
        // [ delete ]
        // else{
            // cur_commit->addFileType(deleted, iter->first);
            // iter = fileSystem.erase(iter);
            // cur_commit->addCurFile(iter->first, iter->second);
        // }
    }

    for (stringmap::iterator it = fileSystem.begin(); it != fileSystem.end();++it){
        for (stringmap::iterator iter = fs_files.begin(); iter != fs_files.end();)
        {
            string md5 = fs_files.find(iter->first)->second;
            if (md5 == it->second){
                // copied
                cur_commit->addFileType(copied, it->first);
                cur_commit->addFileType(copied_to, iter->first);
                cur_commit->removeCurFile(iter->first);
                iter = fs_files.erase(iter);
            } 
            else    
                ++iter;
        }
    }
    for (stringmap::iterator iter = fs_files.begin(); iter != fs_files.end(); ++iter)
        cur_commit->addFileType(newfile, iter->first);
}

bool Peer::logging(int times, string dir){
    if(!readRecord(dir))
        return false;
    int n = split.size();
    int loop = times < n ? times : n;
    for(int i = 0; i < loop; ++i){
        split[i]->printStat(true, i+1);
        if(i != loop-1)
            cout << "\n";
    }
    delete [] buffer;
    return true;
}

void Peer::mergeLogging(int times, string dir){
    readRecord(dir);
    total_commits = split;
    for(map<string, vector<Commit*> >::iterator it = peer_split.begin(); it != peer_split.end(); ++it){
        total_commits.insert(total_commits.end(), it->second.begin(), it->second.end());
    }

    for(int i = 1; i < total_commits.size(); ++i){
        Commit* key = total_commits[i];
        int j = i - 1;
        while(j >= 0 and total_commits[j]->getTime() > key->getTime()){
            total_commits[j+1] = total_commits[j];
            --j;
        }
        total_commits[j+1] = key;
    }

    for(int i = 0; i < total_commits.size(); ++i){
        total_commits[i]->printStat(true, i+1);
        if(i != total_commits.size()-1)
            cout << '\n';
    }
}

bool Peer::status(string path){
    bool success = readRecord(path);
    listFiles(path);
    compare();
    cur_commit->printStat(false, 0);
    return true;
}

void Peer::replayCommits(string path, bool print){
    fileSystem.clear();
    total_commits = split;
    for(map<string, vector<Commit*> >::iterator it = peer_split.begin(); it != peer_split.end(); ++it){
        total_commits.insert(total_commits.end(), it->second.begin(), it->second.end());
    }

    for(int i = 1; i < total_commits.size(); ++i){
        Commit* key = total_commits[i];
        int j = i - 1;
        while(j >= 0 and total_commits[j]->getTime() > key->getTime()){
            total_commits[j+1] = total_commits[j];
            --j;
        }
        total_commits[j+1] = key;
    }

    for(int i = 0; i < total_commits.size(); ++i){
        formLogicView(total_commits[i]);
    }
    
    if(print)
    {
        for(stringmap::iterator it = fileSystem.begin(); it != fileSystem.end(); ++it){
            cout << it->first << '\n';
        }
        cout << "(MD5)\n";
        for(stringmap::iterator it = fileSystem.begin(); it != fileSystem.end(); ++it){
            cout << it->first << ' ' << it->second << '\n';
        }
    }
}

void Peer::formLogicView(Commit* cmt){
    stringmap cmt_files = cmt->getMap();
    vector<vector<string> > cmt_stat = cmt->getStat();
    string fname, md5 = "";
    stringmap::iterator it;

    for(int i = 0; i < 5; ++i){
        for(int j = 0; j < cmt_stat[i].size(); ++j){
            fname = cmt_stat[i][j];

            if(i == 0){ // add 
                md5 = cmt_files.find(fname)->second;
                fileSystem.emplace(fname, md5);
            }
            else if (i == 1){ // mod
                md5 = cmt_files.find(fname)->second;
                if(fileSystem.find(fname) != fileSystem.end()){
                    fileSystem.find(fname)->second = md5;
                }
            }
            else if(i == 2){ // copy
                if(fileSystem.find(fname) != fileSystem.end()){
                    md5 = fileSystem.find(fname)->second; 
                    fileSystem.emplace(cmt_stat[4][j], md5);
                }
            }
            else if(i == 3){
                fileSystem.erase(fname);
            }
        }
    }
}

bool Peer::commit(string path){
    fstream fs(path + "/.loser_record", ios::out | ios::binary | ios::app);
    fs.seekp(0, ios::end);
    int n = split.size() + 1;
    cur_commit->forward(fs, n);

    split.push_back(cur_commit);
    return true;
}

// **************************************************
// *************[ utility function ]*****************
// **************************************************
uint32_t Peer::getNum32(int pos){
    uint32_t s = 0;
    for(int i = 0; i < 4; ++i){
        uint8_t bit = buffer[pos + i];
        s += pow(Byte_size, i) * bit;
    }
    return s;
}

// from name size(8-bit) get file name
string Peer::getName(int& cur){
    string str;
    uint8_t name_size = buffer[cur++];
    for(int i = 0; i < name_size; ++i, ++cur){
        str += buffer[cur];
    }
    return str;
}

void Peer::listFiles(string path){
    DIR* dir;
    struct dirent *ent;
    set<string> exclude {".", "..", ".loser_record", ".peer_record"};
    
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
                cur_commit->addCurFile(name, md5mgr.md5());
            }
        }
        closedir(dir);
    }
}

// excluding s, reset other to false
void Peer::needUpdate(bool b, string s){
    if(!b)
        return;
    for(map<string, bool>::iterator it = peer_updated.begin(); it != peer_updated.end(); ++it){
        if(it->first != s) {  
            it->second = false;
        }
    }
}

void Peer::communicate(bool login){
    int ret;
    const int buffer_size = 1024;
    string s;
    for(map<string, bool>::iterator addr = peer_updated.begin(); addr != peer_updated.end(); ++addr)
    {
        // whether peer has been update
        if(addr->second)
            continue;
        int data_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        if (data_socket == -1) {
            perror("socket");
            exit(EXIT_FAILURE);
        }
        struct sockaddr_un peer_addr;
        memset(&peer_addr, 0, sizeof(struct sockaddr_un));
        peer_addr.sun_family = AF_UNIX;
        strncpy(peer_addr.sun_path, addr->first.c_str(), sizeof(peer_addr.sun_path) - 1);
        ret = connect (data_socket, (const struct sockaddr *) &peer_addr,
                    sizeof(struct sockaddr_un));
        if (ret != -1) {
            // self commit
            if(login)
                ret = write(data_socket, "login", buffer_size);
            else
                ret = write(data_socket, "init", buffer_size);
            ret = write(data_socket, host_socket.c_str(), buffer_size);
            for(int i = 0; i < split.size(); ++i){
                send_commit(split[i], data_socket);
            }
            // cout << "[Send] updated self commit in peer socket " << addr->first <<endl;

            // peer commit
            for(peermap::iterator cmt = peer_split.begin(); cmt != peer_split.end(); ++cmt){
                if(cmt->first == addr->first)
                    continue;
                ret = write(data_socket, "init", buffer_size);
                ret = write(data_socket, cmt->first.c_str(), buffer_size);
                for(int i = 0; i < cmt->second.size(); ++i){
                    send_commit(cmt->second[i], data_socket);
                }
                // cout << "[Send] updated " << cmt->first.substr(9) << "'s commit in peer socket " << addr->first<<endl;
            }
    
            ret = write(data_socket, "end", buffer_size);     
            addr->second = true;
        }
        // else{
        //     cerr << "[Send]" <<(peer_sockets[i]);
        //     perror(" connect");
        // }

        close(data_socket);  
    }
}

void Peer::send_commit(Commit* cmt, int fd){
    int ret;
    const int buffer_size = 1024;
    vector<vector<string> > stat= cmt->getStat();
    stringmap files = cmt->getMap();
    string s;

    ret = write(fd, "head", buffer_size);
    for(int i = 0; i < 5; ++i){
        for(int j = 0; j < stat[i].size(); ++j){
            s = stat[i][j];
            ret = write(fd, s.c_str(), buffer_size);
        }
        ret = write(fd, "===", buffer_size);
    }
    for(stringmap::iterator it = files.begin(); it != files.end(); ++it){
        s = it->first;
        ret = write(fd, s.c_str(), buffer_size);
        s = it->second;
        ret = write(fd, s.c_str(), buffer_size);
    }
    ret = write(fd, "===", buffer_size);
    s = to_string(cmt->getTime());
    ret = write(fd, s.c_str(), buffer_size);
    ret = write(fd, "===", buffer_size);
    for(int i = 0; i < file_content.size()/(buffer_size); ++i){
        string partition = file_content.substr(i*(buffer_size), buffer_size);
        ret = write(fd, partition.c_str(), buffer_size);
    }
}

void Peer::recv_commit(int fd){
    // peer_split.clear();
    Commit* cmt = NULL;
    const int buffer_size = 1024;
    char buffer [buffer_size];
    bool first = true, addr = true, login = false;
    int ret, cnt = 0;
    string fname, peer_name, sender;
    fstream fs;

    while(true){
        ret = read(fd, buffer, buffer_size);
        // buffer[buffer_size-1] = 0;
        if (!strncmp(buffer, "init", 10)) {
            cnt = 0;
        }
        else if (!strncmp(buffer, "login", 10)) {
            cnt = 0;
            login = true;
        }
        else if (!strncmp(buffer, "head", 10)) {
            cmt = new Commit();
            cnt = 1;
        }
        else if (!strncmp(buffer, "===", 10)) {
            ++cnt;
        }
        else if (!strncmp(buffer, "end", 10)) {
            // cout << "[Recv] terminate\n";
            fs.close();
            break;
        }
        else{
            if(cnt == 0) {
                peer_name = buffer;
                peer_split.find(peer_name)->second.clear();
                if(addr){
                    sender = peer_name;
                    addr = false;
                }
                if(login){
                    needUpdate(true, "");
                    login = false;
                }
            }
            else if(cnt < 6){   // add, modify, copy, delete
                cmt->addFileType(cnt-1, buffer);
            }
            else if(cnt == 6 && first){ // md5
                fname = buffer;
                fs.open(fname, ios::out | ios::binary);
                first = false;
            }
            else if(cnt == 6 && !first){
                cmt->addCurFile(fname, buffer);
                first = true;    
            }
            else if(cnt == 7){  // timestamp
                uint32_t t =  stoul(buffer);
                cmt->setTime(t);
                if(peer_split.find(peer_name) == peer_split.end()){
                    vector<Commit*> v;
                    peer_split.emplace(peer_name, v);
                }
                peer_split.find(peer_name)->second.push_back(cmt);
                needUpdate(true, sender);
            } 
            else{
                fs.write(buffer, buffer_size);
            }
        }
        // if(ret != -1)
        //     printf("[Recv] %s from fd #%d\n", buffer, fd);
    }
}

void Peer::request(){
    
}