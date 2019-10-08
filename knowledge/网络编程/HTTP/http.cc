#include "tcpsock.h"

int main(){
    //创建服务端
    TcpSock sock;
    
    //创建套接字
    CHECK_RET(sock.Sock());

    uint16_t port = 7777; 

    //绑定地址信息
    CHECK_RET(sock.Bind("0.0.0.0", port));

    //开始监听
    CHECK_RET(sock.Listen());

    while(1){
        //用于通信的sock
        TcpSock newsock;
        
        //获取已连接的客户端
        int ret = sock.Accept(newsock);
        if(ret == false){
            continue;
        }

        //接收http请求
        string buf;
        ret = newsock.Recv(buf);
        cout << "client :" << buf << endl;

        //组织http响应
        string first = "HTTP/1.1 200 OK\r\n";   //首行
        string blank = "\r\n";  //空行
        string body = "<html><body><h1> 你好 </h1></body></html>";  //正文
        body += "<meta http-equiv='content-type' content='text/html;charset=utf-8'>";
        stringstream ss;
        ss << "Content-Lenth: " << body.size() << "\r\n";
        string head = ss.str();  //头部

        //发送
        newsock.Send(first);
        newsock.Send(head);
        newsock.Send(blank);
        newsock.Send(body);

        //关闭套接字
        newsock.Close();
    }
    
    //关闭套接字
    sock.Close();

    return 0;
}
