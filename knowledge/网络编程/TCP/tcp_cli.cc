//创建客户端
#include "tcpsock.h"

int main(int argc, char* argv[]){
    //判断参数的传入个数
    if(argc != 3){
        cout << "传入的参数有误" << endl;
        return -1;
    }

    //获取ip和port
    string ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //创建客户端
    TcpSock sock;

    //创建套接字
    CHECK_RET(sock.Sock());

    //绑定地址信息
    //客户端不需要主动绑定地址信息
    //系统会自动为客户端绑定
    
    //向服务端发送连接请求
    CHECK_RET(sock.Connect(ip, port));

    while(1){
        //发送数据
        string  buf;
        cin >> buf;
        int ret = sock.Send(buf);
        if(ret == false){
            sock.Close();
            return -1;
        }

        //接收数据
        buf.clear();    //清空buf
        ret = sock.Recv(buf);
        if(ret == false){
            sock.Close();
            return -1;
        }
        cout << "服务端发送的数据：" << buf << endl; 
    }

    //关闭套接字
    sock.Close();

    return 0;
}
