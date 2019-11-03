//获取md5验证
//linux下获取MD5命令：md5sum
//window下获取md5命令：certutil -hashfile name md5

#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <unordered_map>
#include <boost/algorithm/string.hpp>
#include <vector>
using namespace std;
using namespace boost;

#define _MYROOT "./myroot/"

//Boundary类
class Boundary{
    public:
        int64_t _startAddr;    //每一个boundary的起始位置
        int64_t _dataLen;  //每一个boundary长度
        string _name;   
        string _fileName;   //上传文件的文件名
};

//解析boundary头部
bool BoundaryHeaderProcess(string& boundaryHeader, Boundary& file){
    //Content-Disposition: form-data; name="fileupload"; filename="hello.txt"
    //Content-Type: text/plain
    //或者
    //Content-Disposition: form-data; name="submit"
    
    //先通过\r\n分割头部，只取Content-Disposition中的内容
    //再通过: 来进行分割，只需要name和filename
    
    //通过 \r\n 分割boundary头部的信息
    vector<string> list;
    split(list, boundaryHeader, is_any_of("\r\n"), token_compress_on);
    for(int i = 0; i < list.size(); i++){
        string sep = ": ";
        size_t pos = list[i].find(sep);
        if(pos == string::npos){
            cerr << "can not find" << sep << endl;
            return false;
        }   
        string key = list[i].substr(0, pos);
        string val = list[i].substr(pos + sep.size());
        
        if(key != "Content-Disposition"){
            continue;
        }

        string nameField = "fileupload";
        string fileNameSep = "filename=\"";
        //查看该头部是不是fileupload功能
        pos = val.find(nameField);
        if(pos == string::npos){
            continue;
        }
       
        //寻找文件名
        pos = val.find(fileNameSep);
        if(pos == string::npos){
            cerr << "can not find" << fileNameSep << endl;
            return false;
        }

        pos += fileNameSep.size();
        size_t nextPos = val.find("\"", pos);
        if(nextPos == string::npos){
            cerr << "can not find \"" << endl;
            return false;
        }
        
        file._fileName = val.substr(pos, nextPos - pos);
        file._name = "fileupload";
    }

    return true;
}


//解析body每一个boundary块
bool BoundaryParse(unordered_map<string, string>& bodyEnv, string& body, vector<Boundary>& list){
    //获取Content-Type，并从中拿出boundary
    string sign = "boundary=";
    string type;
    auto it = bodyEnv.find("Content-Type");
    if(it != bodyEnv.end()){
        type =  it->second;
    }
    else{
        cerr << "can not find Content-Type" << endl;
        return false;
    }

    size_t pos = type.find(sign);
    if(pos == string::npos){
        cerr << "can not find boundary=" << endl;
        return false;
    }
    
    //------WebKitFormBoundaryASkxiuZYYqFisEUd
    //Content-Disposition: form-data; name="fileupload"; filename="hello.txt"
    //Content-Type: text/plain

    //hello world
    //------WebKitFormBoundaryASkxiuZYYqFisEUd
    //Content-Disposition: form-data; name="submit"

    //&#25552;&#20132;
    //------WebKitFormBoundaryASkxiuZYYqFisEUd--
   
    //从Content-Type中获取的boundary标志，该标志会根据文件生成一个尽量不与文件冲突的标志
    string boundary = type.substr(pos + sign.size());   
    string dash = "--";     
    string craf = "\r\n";
    string tail = "\r\n\r\n";
    

    string firstBoundary = dash + boundary +craf;   //第一个boundary块标志
    string nextBoundary = craf + dash + boundary;   //下一个boundary块标志


    //获取每一个boundary的头部信息，解析出第一个boundary中的name和filename，
    //获取第一个boundary与第二个boundary中的文件信息（上传文件的内容），
    //因为客户端可能会上传多个文件，所有第二个boundary可能会有多个
    size_t curPos, nextPos;
    curPos = body.find(firstBoundary);
    if(curPos == string::npos){
        cerr << "first boundary error" << endl;
        return false;
    }
    
    curPos += firstBoundary.size();     //第一块boundary头部的起始位置

    while(curPos < body.size()){
        nextPos = body.find(tail, curPos);      //第一块boundary头部的结束位置
        if(nextPos == string::npos){
            cerr << "can not find" << tail << endl;
            return false;
        }

        //获取第一块boundary的头部
        string boundaryHeader = body.substr(curPos, nextPos - curPos);

        curPos = nextPos + tail.size();  //数据的起始位置
        nextPos = body.find(nextBoundary, curPos);  //数据的结束位置
        if(nextPos == string::npos){
            cerr << "can not find" << nextBoundary << endl;
            return false;
        }
        
        int64_t offset = curPos;   //获取数据的起始位置
        int64_t length = nextPos - curPos;  //数据长度

        nextPos += nextBoundary.size(); //跳过下一个boundary
        curPos = body.find(craf, nextPos);  //查找 \r\n
        if(curPos == string::npos){
            cerr << "can not find " << craf << endl;
            return false;
        }
        curPos += craf.size(); //指向下一个boundary的位置 
        //若此时boundary指向结尾，说明没有boundary块了

        Boundary file;
        file._startAddr = offset;
        file._dataLen = length;

        //解析头部
        bool ret = BoundaryHeaderProcess(boundaryHeader, file);
        if(ret == false){
            cerr << "boundaryHeader process error" << endl;
            return false;
        }

        //将该boundary添加到链表中
        list.push_back(file);
    }

    return true;
}


//上传文件
bool StorageFile(string& body, vector<Boundary>& list){
    for(int i = 0; i < list.size(); i++){
        if(list[i]._name != "fileupload"){
            continue;
        }

        string realPath = _MYROOT + list[i]._fileName;
        ofstream file(realPath);
        if(!file.is_open()){
            cerr << "open file" << realPath << "failed" << endl;
            return false;
        }
        
        file.write(&body[list[i]._startAddr], list[i]._dataLen);
        if(!file.good()){
            cerr << "write file error" << endl;
            return false;
        }
    
        file.close();
    }

    return true;
}


int main(int argc, char* argv[], char* env[]){
    //向父进程传入CGI处理结果
    string fail = "<html><h1>Fileupload Failed<h1></html>";
    string sucess = "<html><h1>Fileupload Sucess</h1></html>";

    //将父进程传入的环境变量储存在inordered_map中
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
    //在环境变量（头部）中得到正文大小
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
    cerr << "body: " << endl << body << endl;

    //储存每一个boundary块的信息
    vector<Boundary> list;
    bool ret = BoundaryParse(bodyEnv, body, list);
    cerr << "list: " << endl;
    for(int i = 0; i < list.size(); i++){
        cerr << list[i]._fileName << endl;
    }

    if(ret == false){
        cerr << "boundary parse error" << endl;
        cout << fail;
        return -1;
    }
   
    //上传文件
    ret = StorageFile(body, list);
    if(ret == false){
        cerr << "stroge error" << endl;
        cout << fail;
        return -1;
    }
    
    cout << sucess;
    return 0;
}
