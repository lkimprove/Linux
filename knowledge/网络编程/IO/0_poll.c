#include <stdio.h>
#include <unistd.h>
#include <poll.h>

int main(){
    //创建pollfd结构体
    struct pollfd _pollfd;
    _pollfd.fd = 0;     //需要监控的文件描述符
    _pollfd.events = POLLIN;    //监控的事件

    while(1){
        int ret = poll(&_pollfd, 1, 3000);  //开始监控
        if(ret < 0){
            perror("poll error\n");
            return -1;
        }
        if(ret == 0){
            printf("wait timeout\n");
            continue;
        }

        //监控的文件描述符已就绪
        if(_pollfd.revents == POLLIN){  
            char buf[1024] = { 0 };
            ret = read(0, buf, 1023);
            if(ret < 0){
                perror("read error\n");
                return -1;
            }
            printf("input: %s", buf);
        }
    }

    return 0;
}
