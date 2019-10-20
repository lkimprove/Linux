#include "tcpsocket.h"

class Select{
    private:
        fd_set _rfds;
        int _maxfd;
    public:
        //构造函数
        Select():_maxfd(-1){
            FD_ZERO(&_rfds);
        }
        
        //添加文件描述符
        bool Add(TcpSocket sock){
            int fd = sock.GetFd();
            
            FD_SET(fd, &_rfds);

            if(fd > _maxfd){
                _maxfd = fd;
            }

            return true;
        }

        //删除文件描述符
        bool Del(TcpSocket sock){
            int fd = sock.GetFd();

            FD_CLR(fd, &_rfds);
            
            for(int i = _maxfd; i >= 0 ; i--){
                if(FD_ISSET(i, &_rfds)){
                    _maxfd = i;
                }
            }
            
            return true;
        }

        //获取已就绪的文件描述符
        bool Wait(vector<TcpSocket>& _list, int sec = 2){
            timeval tv;
            tv.tv_sec = sec;
            tv.tv_usec = 0;

            fd_set temp =  _rfds;
            int nfds = select(_maxfd + 1, &temp, NULL, NULL, &tv);
            if(nfds < 0){
                cerr << "select error" << endl;
                return false;
            }
            else if(nfds == 0){
                cout << "wait timeout" << endl;
                return false;
            }

            for(int i = 0; i <= _maxfd; i++){
                if(FD_ISSET(i, &temp)){
                    TcpSocket sock;
                    sock.SetFd(i);
                    
                    _list.push_back(sock);
                }
            }

            return true;
        }
};


int main(int argc, char* argv[]){
    //判断参数个数
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
    
    //创建select
    Select s;


    s.Add(listenSock);
    
    while(1){
        //创建一个保存已就绪sock的数组
        vector<TcpSocket> list;
        
        if(s.Wait(list) == false){
            sleep(2);
            continue;
        }

        for(int i = 0; i < list.size(); i++){
            //如果就绪的是监听套接字
            if(list[i].GetFd() == listenSock.GetFd()){
                //监听套接字获取新连接
                TcpSocket newsock;
                int ret = listenSock.Accept(newsock);
                if(ret == false){
                    newsock.Close();
                    continue;
                }
                
                //将已连接的套接字添加到select的监控集合中
                s.Add(newsock);
            }

            //如果就绪的是通信套接字
            else{
                //接收数据
                string buf;
                int ret = list[i].Recv(buf);
                if(ret == false){
                    list[i].Close();
                    s.Del(list[i]);
                    continue;
                }
                cout << "客户端发送的数据为：" << buf << endl;

                //发送数据
                buf.clear();
                cin >> buf;

                ret = list[i].Send(buf);
                if(ret == false){
                    list[i].Close();
                    s.Del(list[i]);
                    continue;
                }
            }
        }

    }

    listenSock.Close();

    return 0;
} 
