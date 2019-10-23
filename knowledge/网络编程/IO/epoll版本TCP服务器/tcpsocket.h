#include <iostream>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <vector>
using namespace std;

#define CHECK_RET(q) if((q) == false) { return -1; }

class TcpSocket{
    private:
        int _sockfd;
    public:
        //构造函数
        TcpSocket():_sockfd(-1){}

        //获取sockfd
        int GetFd(){
            return _sockfd;
        }

        //改变sockfd
        void SetFd(int sockfd){
            _sockfd = sockfd;
        }

        //将套接字设置为非阻塞
        void SetNonBlock(){
            //fcntl函数的参数是一个宏，其内容是二进制数字，该函数通过设置标志位，达到设置状态的目的
            //fcntl函数设置套接字状态时是覆盖设置，则如果直接设置会出现套接字状态只有一个设置的状态
            //所以在设置套接字状态时，必须先获取该套接字的原有状态
            int flag = fcntl(_sockfd, F_GETFL, 0 );
            fcntl(_sockfd, F_SETFL, flag | O_NONBLOCK);
        }

        //创建套接字
        bool Sock(){
            _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(_sockfd < 0) {
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
        bool Listen(int backlog = 10){
            int ret = listen(_sockfd, backlog);
            if(ret < 0){
                cerr << "listen error" << endl;
                return false;
            }

            return true;
        }
        
        //客户端发起连接请求
        bool Connect(const string& ip, uint16_t& port){
            sockaddr_in addr = GetAddr(ip, port);
            socklen_t len = sizeof(addr);
            
            int ret = connect(_sockfd, (sockaddr*)&addr, len);
            if(ret < 0){
                cerr << "accept error" << endl;
                return false;
            }

            return true;
        }
            
        //服务端获取已连接的客户端
        bool Accept(TcpSocket& newSock){
            sockaddr_in addr;
            socklen_t len = sizeof(addr);

            int fd = accept(_sockfd, (sockaddr*)&addr, &len);
            if(fd < 0){
                cerr << "accept error" << endl;
                return false;
            }

            //同一个类的不同对象不存在访问限制
            //获取通信套接字的文件描述符
            newSock._sockfd = fd;


            return true;
        }

        //发送数据
        bool Send(string& buf){
            int ret = send(_sockfd, &buf[0], buf.size(), 0);
            if(ret < 0){
                cerr << "send error" << endl;
                return false;
            }
            
            return true;
        }

        //客户端接收数据
        bool Recv(string& buf, int flag){
            char temp[1024] = { 0 };
            int ret = recv(_sockfd, temp, 1023, 0);
            if(ret < 0){
                cerr << "recv error" << endl;
                return false;
            }
            else if(ret == 0){
                cerr << "peer shutdown" << endl;
                return false;
            }

            buf.assign(temp, ret);

            return true;
        }

        //服务端接收数据
        bool Recv(string& buf){
            char temp[6] = { 0 };
            
            //将套接字设为非阻塞，并将epoll的触发方式设置为边缘触发模式(ET)后
            //需要一次性将缓存区中的数据全部读取
            while(1){
                int ret = recv(_sockfd, temp, 5, 0);
                if(ret < 0){
                    //循环从缓冲区接收数据时，当不满足IO条件时，会返回-1
                    //若返回的errno是EAGAIN或者EWOULDBLOCK时，说明只是此时缓冲区中没有数据，并不是结束数据出错
                    if(errno == EAGAIN){
                        return true;
                    }
                    cerr << "ercv error" << endl;
                    return false;
                }
                else if(ret == 0){
                    cerr << "peer shutdown" << endl;
                    return false;
                }
                    
                temp[ret] = '\0';
                buf += temp;
            }

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
