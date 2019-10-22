#include "tcpsocket.h"
#include <sys/epoll.h>

#define MAX_EPOLL 1024

class Epoll{
    private:
        //epoll在内核中创建eventpoll结构体，用户通过返回的文件描述符进行操作
        int _epfd;  
    public:
        //构造函数
        Epoll():_epfd(-1){}

        //创建eventpoll结构体
        bool Init(){
            _epfd = epoll_create(MAX_EPOLL);
            if(_epfd < 0){
                cerr << "epoll_create crror" << endl;
                return false;
            }

            return true;
        }

        //添加文件描述符
        bool Add(TcpSocket& sock){
            //获取文件描述符
            int fd = sock.GetFd();

            //创建epoll_event事件结构体
            epoll_event ev;
            ev.events = EPOLLIN;
            ev.data.fd = fd;
            
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

            int ret = epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL);    //若用户不关心被删除文件描述符的信息，最后一个参数可直接置为空
            if(ret < 0){
                cerr << "del fd error" << endl;
                return false;
            }

            return true;
        }

        //获取已经就绪的文件描述符
        bool Wait(vector<TcpSocket>& list, int timeout = 2000){
            //创建epoll_event结构体数组，epoll监控完毕后，会将已就绪文件描述符映射到该数组
            //用户可通过该数组直接对已就绪的文件描述符操作
            epoll_event evs[MAX_EPOLL];
            
            int nfds = epoll_wait(_epfd, evs, MAX_EPOLL, timeout);
            if(nfds < 0){
                cerr << "epoll_wait error" << endl;
                return false;
            }
            else if(nfds == 0){
                cerr << "wait timout" << endl;
                return false;
            }

            //获取已经就绪的文件描述符，并将其添加到就绪队列list中
            for(int i = 0; i < nfds; i++){
                int fd = evs[i].data.fd;
                TcpSocket sock;
                sock.SetFd(fd);

                list.push_back(sock);
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
    
    //将监听套接字设置为非阻塞
    listenSock.SetNonBlock();

    //当服务端主动关闭一些连接时（例如某些客户端不活跃，被服务端关闭），则会出现大量的TIME_WAIT连接，这些连接会占用一些五元组
    //若新的连接IP和port与处于TIME_WAIT状态的连接出现冲突时，就会报错
    //这时通过setsockopt函数来设置该监听套接字，不用经历TIME_WAIT等待时间
    int opt = 1;
    int listenFd = listenSock.GetFd();
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    //创建epoll监控
    Epoll e;
    
    //初始化
    CHECK_RET(e.Init());

    //将监听套接字添加至监控集合中
    CHECK_RET(e.Add(listenSock));

    while(1){
        //储存已经就绪的套接字
        vector<TcpSocket> list;

        //判断是否有文件描述符就绪
        if(e.Wait(list) == false){
            sleep(2);
            continue;
        }

        for(int i = 0; i < list.size(); i++){
            //若就绪的是监听描述符
            if(list[i].GetFd() == listenSock.GetFd()){
                TcpSocket newSock;
                int ret = listenSock.Accept(newSock);
                if(ret == false){
                    newSock.Close();
                    continue;
                }
                
                //将通信套接字设置为非阻塞
                newSock.SetNonBlock();

                //将已连接的文件描述符添加到监控集合
                ret = e.Add(newSock);
                if(ret == false){
                    newSock.Close();          
                    continue;
                }
            }
            //若就绪的是通信套接字
            else{
                //接收数据
                string buf;
                int ret = list[i].Recv(buf);
                if(ret == false){
                    e.Del(list[i]);
                    list[i].Close();
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
    
    //关闭监听套接字
    listenSock.Close();
  
    return 0;
}
