#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
using namespace std;

class UdpSock{
    private:
        int _sockfd;
    public:
        //构造函数
        UdpSock():
            _sockfd(-1){}

        //析构函数
        ~UdpSock(){
            //关闭套接字
            Close();
        }

        //创建套接字
        bool Sock(){
            //创建套接字
            _sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

            //判断套接字是否创建成功
            if(_sockfd < 0){
                cerr << "socket error" << endl;
                return false;
            }
            return true;
        }
        
        //创建sockaddr_in 结构体
        sockaddr_in GetIpPort(const string& ip, uint16_t& port){
            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = inet_addr(ip.c_str());

            return addr;
        }

        //绑定地址信息
        bool Bind(const string& ip, uint16_t& port){
            //sockaddr_in 结构体
            sockaddr_in addr;
            addr = GetIpPort(ip, port);
            socklen_t len = sizeof(addr);

            //绑定地址信息
            int ret = bind(_sockfd, (sockaddr*)&addr, len);

            //判断是否创建成功
            if(ret < 0){
                cerr << "bind error" << endl;
                return false;
            }
            return true;
        }

        //发送数据
        bool Send(string& data, string& ip, uint16_t& port){
            //sockaddr_in 结构体
            sockaddr_in addr;
            addr = GetIpPort(ip, port);
            socklen_t len = sizeof(addr);

            //发送数据
            int ret = sendto(_sockfd, &data[0], data.size(), 0, (sockaddr*)&addr, len);

            //判断是否创建成功
            if(ret < 0){
                cerr << "sendto error" << endl;
                return false;
            }
            return true;
        }

        //接收数据
        bool Recv(string& buf, string& ip, uint16_t& port){
            //sokcaddr_in 结构体
            char temp[1024] = { 0 };
            sockaddr_in addr;
            socklen_t len = sizeof(addr);

            //接收数据
            int ret = recvfrom(_sockfd, temp, 1023, 0, (sockaddr*)&addr, &len);

            //判断是否接收成功
            if(ret < 0){
                cerr << "recvfrom error" << endl;
                return false;
            }

            //解析 sockadder_in 结构体
            buf.assign(temp, ret);
            port = ntohs(addr.sin_port);
            ip = inet_ntoa(addr.sin_addr);

            return true;
        }

        //关闭套接字
        bool Close(){
            //判断是否需要关闭套接字
            if(_sockfd <= 0){
                close(_sockfd);
                _sockfd = -1;
            }

            return true;
        }
};
