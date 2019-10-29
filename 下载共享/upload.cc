#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <unordered_map>
using namespace std;

//Boundary类
class Boundary{
    public:
        int64_t _start_addr;    //每一个boundary的起始位置
        int64_t _data_len;  //每一个boundary长度
        string name;   
        string filename;
};

//解析body每一个boundary块
bool BoundaryParse(unordered_map<string, string>& bodyEnv, string& body, vector<Boundary>& list){
    //获取Content-Type，并从中拿出boundary
    string sign = "boundary=";
    string type;
    auto it = bodyEnv.find("Content-Type");
    if(it != bodyEnv.end()){
        string type =  it->second;
    }
    else{
        return false;
    }

    size_t pos = type.find(sign);
    if(pos == string::npos){
        return false;
    }
    string boundary = type.substr(pos + sign.size());
    string dash = "--";
    string craf = "\r\n";
    string tail = "\r\n\r\n";


    return true;
}

int main(int argc, char* argv[], char* env[]){
    cerr << "=============CGImain==============" << endl;

    unordered_map<string, string> bodyEnv;
    //获取每一个环境变量(HTTP请求头部)
    for(int i = 0; env[i] != NULL; i++){
        //每一个环境变为[key=val]的形式，通过=进行分割
        string tmp = env[i];    
        size_t pos = tmp.find("=");
        if(pos == string::npos){
            cerr << "环境变量设置出错" << endl;
            exit(-1);
        }
        
        string key = tmp.substr(0, pos);
        string val = tmp.substr(pos + 1);
        
        bodyEnv[key] = val;
    }

    cerr << "环境变量" << endl;
    for(auto it : bodyEnv){
        cerr << it.first << ": " << it.second << endl;
    }

    //获取正文
    string body;
    //正文大小
    int64_t bodyLen;
    auto it = bodyEnv.find("Content-Length") ;
    if(it != bodyEnv.end()){
        stringstream tmp;
        tmp << it->second;
        tmp >> bodyLen;
    }
    else{
        cerr << "缺少Content-Length环境变量" << endl;
        exit(-1);
    }

    //接受正文
    body.resize(bodyLen);
    int rlen = 0;
    while(rlen < bodyLen){
        int ret = read(0, &body[0] + rlen, bodyLen - rlen );
        if(ret <= 0){
            exit(-1);
        }

        rlen += ret;
    }
    cerr << "body" << endl;
    cerr << body << endl;



    return 0;
}
