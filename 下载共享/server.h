#ifndef __M_SRV_H__
#define __M_SRV_H__
#include "tcpsocket.h"
#include "epoll_wait.h"
#include "threadpool.h"
#include "http.h"
#include <boost/filesystem.hpp>
#include <sstream>
#define _MYROOT "./myroot"

class Server{
    private:
        TcpSocket _listenSock;  //用于监听的套接字
        ThreadPool _pool;   //线程池
        Epoll _epoll;   //epoll监控
    public:
        bool Start(uint16_t port){
            bool ret = true;

            //初始化服务端
            //创建套接字 + 绑定地址信息 + 监听
            ret = _listenSock.SocketInit(port);
            if(ret == false){
                return false;
            }

            //将监听套接字设置为非阻塞
            _listenSock.SetNonBlock();

            //初始化线程池pool
            ret = _pool.PoolInit();
            if(ret == false){
                return false;
            }
            
            //初始化epoll监控
            ret = _epoll.EpollInit();
            if(ret == false){
                return false;
            }

            //将监听描述符添加到epoll监控中
            ret = _epoll.AddSock(_listenSock);
            if(ret == false){
                return false;
            }

            //开始epoll监控
            while(1){
                //储存已就绪的套接字
                vector<TcpSocket> list;
                
                //判断是否有就绪的套接字
                if(_epoll.WaitSock(list) == false){
                    continue;
                }

                //如果就绪的是通信套接字
                for(int i = 0; i < list.size(); i++){
                    //若就绪的是监听套接字
                    if(list[i].GetFd() == _listenSock.GetFd()){
                        //获取已连接的客户端
                        TcpSocket cliSock;
                        ret = list[i].Accept(cliSock);
                        if(ret == false){
                            cliSock.Close();
                            continue;
                        }    

                        //添加到监控中
                        ret = _epoll.AddSock(cliSock);
                        if(ret == false){
                            cliSock.Close();
                            continue;
                        }
                        
                        //将通信套接字设置为非阻塞
                        cliSock.SetNonBlock();
                    }
                    //若就绪的是通信套接字
                    else{
                        //创建任务
                        ThreadTask task;
                        task.SetTask(list[i].GetFd(), ThreadHandler);

                        //将任务添加到任务中
                        _pool.AddTask(task);

                        //将该套接字从监控中移除，避免该套接字因为一直处于就绪状态而一直被处理
                        _epoll.DelSock(list[i]);
                    }
                }
            }
        
            //关闭监听套接字
            _listenSock.Close();

            return true;
        }

        //数据的处理方式
        static void ThreadHandler(int sockfd){
            //获取通信套接字
            TcpSocket cliSock;
            cliSock.SetFd(sockfd);

            HttpRequest req;   //HTTP解析类
            HttpResponse rep;   //HTTP响应类

            //解析HTTP
            int status = req.ParseHttp(cliSock);
            //若HTTP解析失败，则会返回400，直接返回错误信息
            if(status == 400){
                rep._status = status;
                
                //发送失败响应
                rep.SendError(cliSock);
                //关闭客户端
                cliSock.Close();
                
                return;
            }
            
            //解析成功HTTP请求成功
            cout << "------------------" << endl;

            //根据解析的HTTP请求，组织HTTP响应，并将其填充到HTTP响应类rep中
            HttpProcess(req, rep);
             
            //发送HTTP响应
            rep.SendResponse(cliSock);

            //采用短链接，直接关闭客户端
            cliSock.Close();
            
            return;
        }
       
        
        //根据客户端请求的功能，组织HTTP响应
        static void HttpProcess(HttpRequest& req, HttpResponse& rep){
            rep._version = req._version;    //协议版本
            
            //rep._status = 200;  //状态码
            //rep._version = req._version;    //协议版本
            //rep._body = "<html> Hello World </html>";       //正文
            //rep.SetHeader("Content-Type", "text/html");    //告诉浏览器需要将正文通过html渲染出来
            
            //上传文件(POST) --- 多进程CGI处理
            //查看文件(GET) --- 请求一个目录
            //下载文件(GET) --- 请求一个文件
           
            //获取相对路径，定义实际路径
            //查看路径是否存在 filesystem::exists(string)
            string realPath = _MYROOT + req._path; 
            if(!filesystem::exists(realPath)){
                rep._status = 404;
                return;
            }
            
            //若请求方法位POST，则为上传文件请求，进行多进程CGI处理
            //若请求方法时GET，但查询字符出不为空，说明客户端提交了数据需要服务端进行处理，也直接进行多进程CGI处理
            if((req._method == "POST") || (req._method == "GET" && req._queryString.size() != 0)){
                //多进程CGI处理
            }
            else if(req._method == "GET" && req._queryString.size() == 0){
                //若请求方法为GET，且查询字符串为空，此时需要判断客户端请求的路径 filesystem::is_directory
                //若请求的路径是目录 --- 查看目录
                if(filesystem::is_directory(realPath)){
                    ListShow(realPath, rep._body);
                    rep.SetHeader("Content-Type", "text/html");
                }
                //若请求的路径是文件 --- 文件下载
                else{
                    
                }
            }
            

            
            rep._status = 200;  //状态码
        }

        //目录展示
        static void ListShow(string& path, string& body){
            stringstream tmp;
            tmp << "<heml>Hello</html>";
            body = tmp.str();
        }

};
#endif
