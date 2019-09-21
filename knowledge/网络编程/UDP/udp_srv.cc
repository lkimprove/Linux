#include "udpsock.hpp"

#define CHECK_RET(q) if((q) == false) { return -1; }

int main(int argc, char* argv[]){
    //判断传入参数个数
    if(argc != 3){
        cout << "传入参数有误" << endl;
        return -1;
    }

    //接收 IP 和 Port
    string ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //创建服务端
    UdpSock sock;

    //创建套接字
    CHECK_RET(sock.Sock());

    //绑定地址信息
    //服务端的 IP 及 Port 是固定的，以便客户端给服务端发送数据
    CHECK_RET(sock.Bind(ip, port));

    while(1){
        //服务端先接收数据，从而获取客户端的 IP 及 Port 

        //接收数据
        string buf;         //接收数据的buf
        CHECK_RET(sock.Recv(buf, ip, port));
        cout << "客户端发送的数据为: " << buf << endl;

        //发送数据
        buf.clear();        //清空buf
        cin >> buf;         //发送的数据
        CHECK_RET(sock.Send(buf, ip, port));
    }

    //关闭套接字
    sock.Close();

    return 0;
}
