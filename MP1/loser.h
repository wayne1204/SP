#include <string>
#include <vector>
#include <set>
#include <map>
using namespace std;

class Commit{
public:
    Commit(){
        for (int i = 0; i < 5; ++i)
        {
            vector<string> single;
            stat.push_back(single);
        }
    }
    // print
    void printStat(bool md5);
    
    // access
    void addFileName(int i, string s) { stat[i].push_back(s); }
    void addCurFile(string name, string md5);
    void setTime(uint32_t t) { _timestamp = t;}
    void setNum(uint32_t t) {_ncommit = t;}
    
    uint32_t getSize();
    uint32_t getTime() { return _timestamp; }
    uint32_t getNumCommit() {return _ncommit; }
    map<string, string> getMap() {return cur_file;}
    vector<vector<string> > getStat() {return stat;}

private:
    uint32_t _timestamp; // commit timestamp
    uint32_t _ncommit;   // n-th commit
    vector<vector<string> > stat;
    map<string, string> cur_file;
};



class Loser{
public:
    Loser() {
        cur_commit = new Commit();
    }
    // parse
    bool readRecord(string dir);
    void splitCommit();
    // command
    bool logging(int times, string dir);
    void compare(bool init);
    bool commit(string dir);
    bool status(string dir);
    // util
    uint32_t getNum32(int pos);
    void writeNum32(fstream &fs, uint32_t num);
    void writeMD5(fstream &fs, string md5);
    string getName(int &cur);
    void listFiles(string path);

private:
    vector<Commit*> split;
    // map<string, string> last_commit;
    map<string, string> table;
    set<string> isTake;
    // vector<vector<string> > stat;
    Commit* cur_commit;
    int file_size;
    char* buffer;
};

