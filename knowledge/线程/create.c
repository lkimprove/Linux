//实现线程的创建和终止

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>


void* thr_start(void* arg){
   //线程的tid --> 线程地址空间的首地址
   pthread_t tid = pthread_self();

    while(1){
        printf("It is a %s pthread --- %p\n", (char*)arg, tid);
        sleep(2);
    }
}

int main(){
    //接收 pthread_create 的返回值
    int ret;

    //创建的线程
    pthread_t tid;

    char temp[] = "ordinary";

    //主线程ID ----> 线程组ID/进程ID
    pthread_t mtid = pthread_self();

    //创建线程
    ret = pthread_create(&tid, NULL, thr_start, (void*)temp);
    //判断线程是否创建成功
    if(ret != 0){
        printf("create thread error");
        return -1;
    }

    int i = 0;
    while(1){
        i++;
        printf("It is a main thread --- %p --- %p\n", mtid, tid);
        sleep(2);

        //终止普通线程
        if(i == 2){
             pthread_cancel(tid);
        }

        //终止主线程 ---> 进程不会终止
        if(i == 5){
            pthread_exit(NULL);
        }
    }

    return 0;
}
