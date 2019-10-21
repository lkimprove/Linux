#include "tcpsocket.h"

int main(int argc, char* argv[]){
    //判断参数个数
    if(argc != 3){
        cout << "传入的参数有误" << endl;
        return -1;
    }

    //获取ip和port
    string ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //创建客户端
    TcpSocket sock;

    //创建套接字
    CHECK_RET(sock.Sock());

    //客户端不用手打绑定地址信息
    
    //发送连接请求
    CHECK_RET(sock.Connect(ip, port));

    while(1){
        //发送数据
        string buf;
        cin >> buf;
        int ret = sock.Send(buf);
        if(ret == false){
            break;
        }

        //接收数据
        buf.clear();
        ret = sock.Recv(buf);
        if(ret == false){
            break;
        }
        cout << "服务端发送的数据为：" << buf << endl; 
    
    }
    
    sock.Close();

    return 0;
}
