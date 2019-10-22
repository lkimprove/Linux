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

        //设置为非阻塞
        void SetNonBlock(){
            //F_SETFD是一种覆盖设置，因为参数设置是一个宏，其内容是一个二进制位的标志位
            //若直接进行设置，套接字的其他属性会丢失，
            //所以需要先获取套接字原来的属性，在 |
            int flag = fcntl(_sockfd, F_GETFL, 0);
            fcntl(_sockfd, F_SETFL, O_NONBLOCK | flag);
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

        //客户端接收数据
        bool Recv(string& buf, int flag){
            char temp[1024] = { 0 };
            
            int ret = recv(_sockfd, temp, 1023, 0);
            if(ret < 0){
                cerr << "recv error" << endl;
                return false;
            }
            else if(ret == 0){
                cerr << "wait timoeout" << endl;
                return false;
            }

            buf.assign(temp, ret);

            return true;

        }

        //服务端接收数据
        bool Recv(string& buf){
            char temp[6] = { 0 };
            //不确定要接受数据的大小
            //则循环接收数据
            buf.clear();

            while(1){
                int ret = recv(_sockfd, temp, 6, 0);
                if(ret < 0){
                    //当套接字被设置为非阻塞时，recv不满足IO条件时，会返回-1
                    //但若错误编号errno = EAGAIN / EWOULDBLOCK，表示缓冲区中没有可读的数据，而不是致命错误
                    if(errno == EAGAIN){
                        return true;    
                    }

                    cerr << "recv  error" << endl;
                    return false;
                }
                else if(ret == 0){
                    cerr << "perr shutdown" << endl;
                    return false;
                }
                
                //TCP的send和recv函数对于接收的字符是无边界的，不以'\0'为结束标志
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
