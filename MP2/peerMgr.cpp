#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
#include "peerMgr.h"
#include "peer.h"

using namespace std;

bool PeerManager::init_config(Peer* cfg, string path)
{
    fstream fs(path.c_str(), ios::in);
    string line, token;
    size_t pos;
    int cnt = 0;

    if(!fs.is_open())
        return false;

    /* TODO parse the config file */
    while(getline(fs, line)){
        pos = line.find('=') + 1;
        while(split(line, token, pos)){
            switch(cnt){
                case 0:
                    cfg->setHostName(token);
                    break;
                case 1:
                    cfg->addPeers(token);
                    break;
                case 2: 
                    cfg->setRepoName(token);
                    break;
            }
        }
        ++cnt;
    }

    fs.close();
    return true;
}


void PeerManager::destroy_config(Peer* cfg)
{
    /* TODO do something like free() */
}

bool PeerManager::parseCommand(istream& ifs, Peer* cfg){
    string line, token;
    vector<string> cmd {"", "", ""};
    size_t pos = 0;
    int i = 0;
        
    getline(ifs, line);
    while(split(line, token, pos) && i < 3){
        cmd[i++] = token;
    }
    if(line == "")
        return false;
    
    if(cmd[0] == "list"){
        cfg->readRecord(cfg->getRepoName());
        cfg->replayCommits(cfg->getRepoName(), true);
        return false;
    }
    else if(cmd[0] == "cp"){
        if(copy(cmd[1], cmd[2], cfg))
            cout << "success\n";
        else    
            cout << "failed\n";
    }
    else if(cmd[0] == "mv"){
        if(move(cmd[1], cmd[2], cfg))
            cout << "success\n";
        else    
            cout << "failed\n";

    }
    else if(cmd[0] == "rm"){
        removeFile(cmd[1], cfg);
    }
    else if(cmd[0] == "history"){
        if(cmd[1] == "-a")
            cfg->mergeLogging(INT16_MAX, cfg->getRepoName());
        else 
            cfg->logging(INT16_MAX, cfg->getRepoName());
        return false;
    }
    else if(cmd[0] == "exit"){
        cout <<"bye\n";
        exit(EXIT_SUCCESS);
    }
    else{
        cout << "unknown command: " << line << endl;
        return false;
    }
    return true;
}

bool PeerManager::copy(string& src, string& dest, Peer* cfg){
    bool from_fs = false;
    if(src[0] == '@'){
        cfg->replayCommits(cfg->getRepoName(), false);
        src = src.substr(1);
        from_fs = true;
    }
    else{
        fstream fs(src.c_str(), ios::in);
        if(!fs.is_open())
            return false;
        fs.close();
    }

    if(dest[0] == '@'){
        dest = dest.substr(1);
        return cfg->fsAccess(src, dest, from_fs);
    }
    else{
        fstream ifs(src.c_str(), ios::in | ios::binary);
        fstream ofs(dest.c_str(), ios::out | ios::binary);
        ofs << ifs.rdbuf();
    }
    return true;
}

bool PeerManager::move(string& src, string& dest, Peer* cfg){
    bool status, from_fs = false;
    
    if(src[0] == '@'){
        cfg->replayCommits(cfg->getRepoName(), false);
        src = src.substr(1);
        from_fs = true;
    }
    else{
        fstream fs(src.c_str(), ios::in);
        if(!fs.is_open())
            return false;
        fs.close();
    }
    

    if(dest[0] == '@'){
        dest = dest.substr(1);
        status = cfg->fsAccess(src, dest, from_fs);
    }
    else{
        fstream ifs(src.c_str(), ios::in | ios::binary);
        fstream ofs(dest.c_str(), ios::out | ios::binary);
        ofs << ifs.rdbuf();
    }
    if(!from_fs)
        remove(src.c_str());
    return status;
}

bool PeerManager::removeFile(string& src, Peer* cfg){
    if(src[0] == '@')       
        src = src.substr(1);
    cfg->getCommit()->addFileType(deleted, src);
    cfg->commit(cfg->getRepoName());
    return true;
}

bool PeerManager::split(string& line, string& token, size_t& pos){
    size_t begin = line.find_first_not_of(' ', pos);
    if(begin == string::npos)
        return false;
    pos = line.find_first_of(' ', begin);
    token = line.substr(begin, pos - begin);
    return true;
}
