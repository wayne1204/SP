#include <unordered_set>
#include <vector>
#include <string>
#include <istream>
#include <sstream>
using namespace std;

class fileManager
{
  public:
    fileManager()
    {
    }
    void parseSet(string s);
    void readFiles(istream& fs);
    void show();

  private:
    unordered_set<char> char_set;
    stringstream ss;
    string s;
};