#ifndef Peer_h
#define Peer_h

#include <string>
#include <vector>
#include <set>
#include <map>
#include "commit.h"

typedef std::map<std::string, std::vector<Commit*> > peermap;

class Peer{
public:
    Peer(){
        cur_commit = new Commit();
        version = 0;
    }
    // config
    void addPeers(string& peer);
    void setHostName(string name); 
    void setRepoName(string name) {repo = name;}
    int getPeersNum() {return peer_updated.size(); }
    string getHostName() {return host_socket; }
    string getRepoName() {return repo;}
    
    // inter-peer
    void communicate(bool login);
    void needUpdate(bool b, string s);
    void send_commit(Commit* cmt, int fd);
    void recv_commit(int fd);
    void request();

    // parse
    bool readRecord(string dir);
    void splitCommit();
    
    // command
    Commit* getCommit() {return cur_commit;}
    bool fsAccess(string src, string dest, bool from_fs);
    bool logging(int times, string dir);
    void mergeLogging(int times, string dir);
    void replayCommits(string path, bool print);
    void formLogicView(Commit* cmt);
    void compare();
    bool commit(string dir);
    bool status(string dir);
    
    // util
    uint32_t getNum32(int pos);
    string getName(int &cur);
    void listFiles(string path);
private:
    // config
    string repo;
    string host_socket;
    string file_content;
    
    // command
    int version;
    int file_size;
    char* buffer;
    Commit* cur_commit;
    vector<Commit*> split;
    vector<Commit*> total_commits;
    // vector<string> peer_sockets;
    map<string, bool> peer_updated;          // whether peer socket have updated
    peermap peer_split;                      // peer name mapping to peer's commit
    stringmap fileSystem;                    // logical file system
};

#endif