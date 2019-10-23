#include "tcpsocket.h"

#define MAX_EPOLL 1024

class Epoll{
    private:
        int _epfd;
    public:
        //构造函数
        Epoll():_epfd(-1){}

        //创建epoll监控
        bool Init(){
            _epfd = epoll_create(MAX_EPOLL);
            if(_epfd < 0){
                cerr << "epoll_create error" << endl;
                return false;
            }

            return true;
        }

        //添加文件描述符
        bool Add(TcpSocket& sock){
            //获取文件描述符
            int fd = sock.GetFd();

            //创建事件结构
            epoll_event ev;
            ev.events = EPOLLIN;
            ev.data.fd =fd;

            int ret = epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev);
            if(ret < 0){
                cerr << "add fd error" << endl;
                return false;
            }

            return true;
        }

        //删除文件描述符
        bool Del(TcpSocket& sock){
            //获取文件描述符
            int fd = sock.GetFd();

            int ret = epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL);
            if(ret < 0){
                cerr << "del fd error" << endl;
                return false;
            }

            return true;
        }

        //获取已经就绪的文件描述符
        bool Wait(vector<TcpSocket>& list, int timeout = 2000){
            //创建事件结构体数组
            epoll_event evs[MAX_EPOLL];
            int nfds = epoll_wait(_epfd, evs, MAX_EPOLL, timeout);
            if(nfds < 0){
                cerr << "epoll_wait error" << endl;
                return false;
            }
            else if(nfds == 0){
                cerr << "wait timeout" << endl;
                return false;
            }
    
            for(int i = 0; i < nfds; i++){
                int fd = evs[i].data.fd;
                TcpSocket newSock;
                newSock.SetFd(fd);

                list.push_back(newSock);
            }

            return true;
        }
};


int main(int argc, char* argv[]){
    //判断传入的参数个数
    if(argc != 3){
        cout << "传入的参数有误" << endl;
        return -1;
    }
        
    //获取ip和port
    string ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //创建服务端
    TcpSocket listenSock;

    //创建套接字
    CHECK_RET(listenSock.Sock());

    //绑定地址信息
    CHECK_RET(listenSock.Bind(ip, port));

    //开始监听
    CHECK_RET(listenSock.Listen());
    
    //将listenSock设置为非阻塞
    listenSock.SetNonBlock();

    //创建epoll监听
    Epoll e;
    CHECK_RET(e.Init());

    //将监听套接字添加到监控中
    CHECK_RET(e.Add(listenSock));

    while(1){
        //储存已就绪的TcpSocket
        vector<TcpSocket> list;

        //判断监听结果
        if(e.Wait(list) == false){
            continue;
        }
        
        for(int i = 0; i < list.size(); i++){
            //如果就绪的是监听套接字
            if(list[i].GetFd() == listenSock.GetFd()){
                //获取新连接
                TcpSocket newSock;

                int ret = list[i].Accept(newSock);
                if(ret == false){
                    continue;
                }

                //将通信套接字添加到epoll监控中
                ret = e.Add(newSock);
                if(ret == false){
                    newSock.Close();
                    continue;
                }

                //将通信套接字设置为非阻塞
                newSock.SetNonBlock();
            }
            //如果就绪的是通信套接字
            else{
                //接收数据
                string buf;
                int ret = list[i].Recv(buf);
                if(ret == false){
                    e.Del(list[i]);
                    list[i].Close();
                    cout << "关闭一个连接" << endl;

                    continue;
                }
                cout << "客户端发送的数据为：" << buf << endl;

                //发送数据
                buf.clear();
                cin >> buf;
                ret = list[i].Send(buf);
                if(ret == false){
                    e.Del(list[i]);
                    list[i].Close();

                    continue;
                }
            }
        }
    }
    
    listenSock.Close();

    return 0;
}
