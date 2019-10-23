#include "tcpsocket.h"
#include <list>
#include <unordered_map>
#include <boost/algorithm/string.h>
using namespace boost;

//HTTP请求类
class HttpRequest{
    private:
        //首行信息
        string _version;    //协议版本
        string _method;     //请求方法
        int _status;    //状态码
        string _path;   //URL中的请求路径
        //URL中的查询字符串，在URL中以 key=val&key=val 的形式存在
        unordered_map<string, string> _queryString;    

        //头部，以 key: val 的形式存在
        unordered_map<string, string> _header;
        string _body;   //正文

        //接收HTTP请求(首行+头部)
        bool RecvRequest(TcpSocket& cliSock, string& request){
            while(1){
                //探测性接收大量数据，判断是否包含头部+首行（/r/n/r/n）
                string tmp;
                if(cliSock.RecvPeek(tmp) == false){
                    return false;
                }

                size_t pos;
                pos = tmp.find_first_of("\r\n\r\n");
                //若接收一定长度后，仍未找到结束标志（\r\n\r\n） 则认为此次接收HTTP请求失败
                if(pos == string::npos && tmp.size() == MAX_REQUEST){
                    return false;
                }
                //找到结束标志
                else if(pos != string::npos){
                    //直接接收首行 + 头部
                    size_t len = pos + 4;
                    cilSock.Recv(request, len);

                    return true;
                }
            }
        }

        //解析首行
        bool ParseFirstLine(){
        
        }    

    public:
        //解析HTTP请求
        int ParseHttp(TcpSocket& cliSock){
            //接收HTTP请求（首行+头部）
            string request;

            //接收HTTP请求
            if(RecvRequest(cliSock, request) == false){
                return 400;
            }

            //对请求的字符串按照\r\n的格式进行分割，得到一个list
             

            //list[0]是首行，对首行解析

            //lit[1]以后都是头部，对头部解析

            //请求信息校验


            return 200;
        }
};


//HTTP响应类
class HttpResponse{
    private:

    public:
        //发送错误
        void SendError(TcpSocket& cliSock, int status){
          
        }

        //发送HTTP响应
        void SendResponse(TcpSocket& cliSock){
        
        }
};
