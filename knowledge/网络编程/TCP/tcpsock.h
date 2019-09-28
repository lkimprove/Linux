#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
using namespace std;

#define CHECK_RET(q) if((q) ==false) { return -1; }

class TcpSock{
    private:
        int _sockfd;
    public:
        //构造函数
        TcpSock():
            _sockfd(-1){}
        
        //析构函数
        ~TcpSock(){
            Close();
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
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = inet_addr(ip.c_str());

            return addr;
        }

        //绑定地址信息
        bool Bind(const string& ip, uint16_t& port){
            //创建sockaddr_in结构体
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

        //客户端发起连接请求
        bool Connect(const string& ip, uint16_t& port){
            //创建sockaddr_in结构体
            sockaddr_in addr = GetAddr(ip, port);
            socklen_t len = sizeof(addr);
                
            int ret = connect(_sockfd, (sockaddr*)&addr, len);
            if(ret < 0){
                cerr << "connect error" << endl;
                return false;
            }
            return true;
        }

        //获取已连接的客户端的socket描述符 -- sockfd
        void GetFd(int sockfd){
            _sockfd = sockfd;
        }

        //服务端获取已连接的客户端
        bool Accept(TcpSock& newsock){
            //创建sockaddr_in结构体
            sockaddr_in addr;
            socklen_t len = sizeof(addr);
            
            int ret = accept(_sockfd, (sockaddr*)&addr, &len);
            if(ret < 0){
                cerr << "accept error" << endl;
                return false;
            }
            
            //获取sockfd
            newsock.GetFd(ret);

            return true;
        }

        //因为内核已完成三次握手建立连接
        //所以发送数据和接收数据不需要指明 ip 和 port
        
        //发送数据
        bool Send(string& data){
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
                cerr << "recv error" << endl;
                return false;
            }
            
            //获取数据
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
