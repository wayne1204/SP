#include <fstream>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <sys/stat.h>
#include "dirent.h"
#include "csie_box.h"
#include "iostream"
#include <set>
using namespace std;




void csie_box::parse(const char* dir_name){
    string line, token;
    size_t pos;
    fstream fs (dir_name, ios::in);
    if(!fs.is_open()){
        exit(EXIT_FAILURE);
    }
    getline(fs, line);
    pos = line.find('=');
    fifo_path = line.substr(pos+2);
    getline(fs, line);
    pos = line.find('=');
    directory = line.substr(pos+2);
}

void csie_box::createDir(string dir_name){
    struct stat status;
    if(stat(dir_name.c_str(), &status) != 0){
        size_t pos = dir_name.find_last_of('/');
        createDir(dir_name.substr(0, pos));
        mkdir(dir_name.c_str(), 0700);
    }
}

void csie_box::removeDir(string dir_name){
    DIR* dir;
    struct dirent *ent;
    string path;

    if ((dir = opendir(dir_name.c_str())) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            path = dir_name + "/" + ent->d_name;
            if(ent->d_type != DT_DIR){
                remove(path.c_str());
                // printf("remove files: %s \n",  path.c_str());
            }
            else if(strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
            {
                removeDir(path);
                rmdir(path.c_str());
                // printf("remove directory: %s \n",  path.c_str());
            }
        }
        closedir(dir);
    }
}

void csie_box::sendFiles(int fd, string dir_path){
    DIR* dir;
    struct dirent *ent;
    string path, subpath;

    if ((dir = opendir(dir_path.c_str())) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            path = dir_path + "/" + ent->d_name;
            if(ent->d_type != DT_DIR){
                subpath =  path.substr(getDirectory().size()+1);
                write(fd, subpath.c_str(), BUFSIZE); 
                fstream fs(path, ios::in);
                string content((istreambuf_iterator<char>(fs)),
                                (istreambuf_iterator<char>()));
                write(fd, content.c_str(), BUFSIZE);
                // cout << subpath << endl;
                // cout << content << endl;
            }
            else if(strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
            {
                sendFiles(fd, path);
            }
        }
        closedir(dir);
    }
}

void csie_box::readFiles(int fd){
    char buffer [BUFSIZE];
    while(read(fd, buffer, BUFSIZE) != 0){
        fstream fs;
        string path = getDirectory() + '/' + buffer;
        // cout << path << endl;
        size_t pos = path.find_last_of('/');
        createDir(path.substr(0, pos));
        fs.open(path.c_str(), ios::out);
        read(fd, buffer, BUFSIZE);
        fs << buffer;
    }

}

