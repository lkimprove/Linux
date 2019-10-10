//创建select模型监控标准输入流(0号描述符)

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>

int main(){
    fd_set set;     //创建select监控集合
    FD_ZERO(&set);  //清空select监控集合
    FD_SET(0, &set);    //将标准输入（0号描述符）添加到set

    int maxfd = 0;

    while(1){
        //阻塞等待时间
        timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        
        //每次监控结束后，select会将未就绪的描述符清除
        //所以需要再次添加描述符
        FD_SET(0, &set);

        //select监控
        int nfds = select(maxfd + 1, &set, NULL, NULL, &tv);
        if(nfds < 0){
            perror("select error\n");
            return -1;
        }
        else if(nfds == 0){
            printf("wait timeout\n");
            continue;
        }

        //标准输入就绪，接受数据
        char temp[1024] = { 0 };
        int ret = read(0, temp, 1023);
        if(ret < 0){
            perror("read error\n");
            return -1;
        }
        printf("input: %s", temp);
    }

    return 0;
}
