//服务端（多线程版）
#include "tcpsock.h"

int main(int argc, char* argv[]){
    //判断传入参数个数
    if(argc != 3){
        cout << "传入的参数个数有误" << endl;
        return -1;
    }
    
    //获取 ip 和 port
    string ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //创建服务端
    TcpSock sock;

    //创建套接字
    CHECK_RET(sock.Sock());
    
    //绑定地址信息
    CHECK_RET(sock.Bind(ip, port));
    
    //服务端开始监听
    CHECK_RET(sock.Listen());
    
    //创建于客户端通信的socket
    TcpSock newsock;

    while(1){
        //获取已连接的客户端
        int ret = sock.Accept(newsock);
        //获取失败，直接返回循环，进行获取下一个
        if(ret == false){
            continue;
        }
        
        //创建子进程与客户端进行通信
        //优点：1.分摊任务：一个进程与一个客户端进行通信
        //      2.稳定性高：即使某一次通信出现问题，也只会影响一个子进程
        
        if(fork() == 0){
            //为了防止父进程在外部一直等待该子进程，而影响父进程与其他客户端通信
            //子进程可以创建一个孙子进程，然后直接退出子进程，
            //孙子进程就会由init进程处理，而父进程也不会在外阻塞等待子进程
            //if(fork() > 0){
            //    exit(0);
            //}
            //ps：最好处理办法：直接去处理信号
            
            while(1){
                //接收数据
                string buf;
                int ret = newsock.Recv(buf);
                if(ret == false){
                    newsock.Close();
                    exit(0);
                }
                cout << "客户端发送的数据: " << buf << endl;

                //发送数据
                buf.clear();    //清空buf
                cin >> buf;
                ret = newsock.Send(buf);
                if(ret == false){
                    newsock.Close();
                    exit(0);
                }
            }
            
            //关闭套接字，并结束
            newsock.Close();
            exit(0);
        }    
        
        //父进程关闭套接字，断开与soket的连接
        //子进程复制了文件描述表，拥有自己与socket的连接，
        newsock.Close();

        //父进程无需等待子进程退出
        //自定义信号SIGCHLD的处理方式
        //wait(NULL);
    }   
    
    //关闭套接字
    sock.Close();

    return 0;
}
