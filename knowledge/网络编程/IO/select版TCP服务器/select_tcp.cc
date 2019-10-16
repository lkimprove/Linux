#include "tcpsocket.cc"

class Select{
    private:
        fd_set _rfds;   //监控集合
        int _maxfd;     //被监控的最大文件描述符
    public:
        //构造函数
        Select():_maxfd(-1){
            FD_ZERO(&_rfds);
        }

        //添加文件描述符
        bool Add(TcpSocket& sock){
            //获取文件描述符
            int fd = sock.GetFd();
            
            //添加文件描述符
            FD_SET(fd, &_rfds);
            
            //更新最大文件描述符
            if(fd > _maxfd){
                _maxfd = fd;
            }

            return true;
        }


        //删除文件描述符
        bool Del(TcpSocket& sock){
            //获取文件描述符
            int fd = sock.GetFd();
            
            //删除文件描述符
            FD_DEL(fd, &_rfds);

            //更新文件描述符
            for(int i = _maxfd; i >= 0; i--){
                if(FD_ISSET(i, &_rfds)){
                    _maxfd = i;
                }
            }
            
            return true;
        }


        //获取已经就绪的文件描述符
        bool Wait(vector<TcpSocket>& _list, int sec = 2){
            //创建等待时间结构体
            timeval tv;
            tv.tv_sec = se;
            tv.tv_usec = 0;
            
            int nfds = select(_maxfd + 1, &_rfds, NULL, NULL, &tv);
            if(nfds < 0){
                cerr << "select error" << endl;
                return false;
            }
            else if(nfds == 0){
                cout << "wait timeout" << endl;
                return false;
            }
        
            for(int i = 0; i <= _maxfd; i++){
                if(FS_ISSET(i, &_rfds)){
                    TcpSocket sock;
                    sock.SetFd(i);

                    _list.push_back(sock);
                }                
            }

            return true;
        }
};

int main(int argc, cahr* argv[]){
    //判断传入的参数个数
    if(argc != 3){
        cout << "传入的参数个数有误" << endl;
        return -1;
    }

    //获取IP和port
    string ip = argv[1];
    uint16_t port = argv[2];

    //创建服务端
    TcpSocket lisSock;
    
    //创建套接字
    CHECK_RET(lisSock.Sock());

    //绑定地址信息
    CHECK_RET(lisSock.Bind(ip, port));

    //开始监听
    CHECK_RET(lisSock.Listen());

    //创建select监控
    Select s;
    s.Add(lisSock);
    //创建就绪队列
    vector<TcpSocket> list;

    while(1){
        if(s.Wait() == false){
            sleep(1);
            continue;
        }

        for(int i = 0; i < list.size(); i++){
            //就绪的是监听sock
            if(list[i].GetFd() == lisSock.GetFd()){
                //获取新连接
                TcpSocket newsock;
                if(lisSock.Accept(newsock) == false){
                    continue;
                }
                                
                s.Add(newsock);
            }
            //就绪的是通信sock
            else{
                //接收数据
                string buf;
                int ret = list[i].Recv(buf);
                if(ret == false){
                    s.Del(i);
                    list[i].Close();
                    continue;
                }
                cout << "客户端发送的数据为：" << buf << endl;
                
                //发送数据
                buf.clear();    //清空数据
                cin >> buf;
                ret = list[i].Send(buf);
                if(ret == false){
                    s.Del(i);
                    list[i].Close();
                    continue;
                }
            }
        }
    }
    
    sock.Close();
    return 0;
}
