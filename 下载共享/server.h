#include "tcpsocket.h"
#include "epoll_wait.h"
#include "threadpool.h"
#include "http.h"

class Server{
    private:
        TcpSocket _listenSock;  //用于监听的套接字
        ThreadPool _pool;   //线程池
        Epoll _epoll;   //epoll监控
    public:
        bool Start(int port){
            bool ret = true;

            //初始化服务端
            //创建套接字 + 绑定地址信息 + 监听
            ret = _listenSock.SocketInit(port);
            if(rer == false){
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
                    sleep(2);
                    continue;
                }

                //如果就绪的是通信套接字
                for(int i = 0; i < list.size(); i++){
                    //若就绪的是监听套接字
                    if(list[i].GetFd() == listenSock.GetFd()){
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
                        _epool.DelSock(list[i]);
                    }
                }
            }
        
            //关闭监听套接字
            listenSock.Close();

            return true;
        }

        //数据的处理方式
        static void ThreadHandler(int sockfd){
            //获取通信套接字
            TcpSocket cliSock;
            cliSock.SetFd(sockfd);

            HttpResquest req;   //HTTP解析类
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
            
            //根据解析的HTTP请求，组织HTTP响应，并将其填充到HTTP响应类rep中
            HttpProcess(req, rep);
             
            //发送HTTP响应
            rep.SendResponse(cliSock);

            //采用短链接，直接关闭客户端
            cliSock.Close();
            
            return;
        }
       
        
        //HTTP解析
        static void HttpProcess(HttpRequest& req, HttpResponse& rep){
               
              

        }

};
