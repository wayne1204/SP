#include <string>
#include <vector>
#include <set>
#include <map>
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
    void printStat(bool md5, int ncom);
    void forward(fstream& fs, int ncom);
    
    // access
    void addFileType(int i, string s) { stat[i].push_back(s); }
    void addCurFile(string name, string md5);
    void removeCurFile(string name);
    void setTime(uint32_t t) { _timestamp = t;}
    bool update();
    uint32_t getSize();
    uint32_t getTime() { return _timestamp; }
    uint32_t getNumFiles() { return cur_file.size(); }
    map<string, string> getMap() {return cur_file;}
    vector<vector<string> > getStat() {return stat;}

    // util
    void writeNum32(fstream &fs, uint32_t num);
    void writeMD5(fstream &fs, string md5);
private:
    // uint32_t _ncommit;   // n-th commit
    uint32_t _timestamp; // commit timestamp
    vector<vector<string> > stat;
    map<string, string> cur_file;
};
