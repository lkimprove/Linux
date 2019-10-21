#include <iostream>
#include <vector>
#include <string>
#include <sys/select.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
using namespace std;

#define CHECK_RET(q) if((q) == false) { return -1; }


class TcpSocket{
    private:
        int _sockfd;
    public:
        TcpSocket():_sockfd(-1){
        }

        ~TcpSocket(){}

        //获取sockfd
        int GetFd(){
            return _sockfd;
        }

        //改变sockfd
        void SetFd(int sockfd){
            _sockfd =  sockfd;
        }

        //创建套接字
        bool Sock(){
            _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(_sockfd < 0){
                cerr << "socket error" << endl;
                return false;
            }
    
            return true;
        }
        

        //创建sockaddr_in结构体
        sockaddr_in GetAddr(const string& ip, uint16_t& port){
            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = inet_addr(ip.c_str()); 
            addr.sin_port = htons(port);
            
            return addr;
        }

        //绑定地址信息
        bool Bind(const string& ip, uint16_t& port){
            sockaddr_in addr = GetAddr(ip, port);
            socklen_t len = sizeof(addr);
            
            int ret = bind(_sockfd, (sockaddr*)&addr, len);
            if(ret < 0){
                cerr << "bind error" << endl;
                return false;
            }

            return true;
        }

        //服务端开始监听
        bool Listen(int backlog = 5){
            int ret = listen(_sockfd, backlog);
            if(ret < 0){
                cerr << "listen error" << endl;
                return false;
            }

            return true;
        }

        //客户端发送连接请求
        bool Connect(const string& ip, uint16_t port){
            sockaddr_in addr = GetAddr(ip, port);
            socklen_t len = sizeof(addr);
            
            int ret = connect(_sockfd, (sockaddr*)&addr, len);
            if(ret < 0){
                cerr << "connect error" << endl;
                return false;
            }

            return true;
        }

        //服务端获取已建立的连接
        bool Accept(TcpSocket& newsock){
            sockaddr_in addr;
            socklen_t len = sizeof(addr);

            int ret = accept(_sockfd, (sockaddr*)&addr, &len);
            if(ret < 0){
                cerr << "accept error" << endl;
                return false;
            }
            
            newsock.SetFd(ret);

            return true;
        }

        //发送数据
        bool Send(const string& data){
            int ret = send(_sockfd, &data[0], data.size(), 0);
            if(ret < 0){
                cerr << "send error" << endl;
                return false;
            }

            return true;
        }

        //接收数据
        bool Recv(string& buf){
            char temp[1024] = { 0 };
            int ret = recv(_sockfd, temp, 1023, 0);
            if(ret < 0){
                cerr << "recv  error" << endl;
                return false;
            }
            
            buf.assign(temp, ret);

            return true;
        }

        //关闭套接字
        bool Close(){
            if(_sockfd >= 0){
                close(_sockfd);
                _sockfd = -1;
            }

            return true;
        }

};
