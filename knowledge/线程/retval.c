//实现线程等待，线程分离
//被等待的线程必须是joinable属性
//被分离的线程会成为detach属性，自动回收资源

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

//线程入口函数
void* thr_start(void* arg){
    //不能返回一个局部变量
    //char temp[] = "hello"
    char* temp = "hello";
    sleep(3);
    return temp;
}

int main(){
    int ret;
    pthread_t tid;

    //创建线程
    ret = pthread_create(&tid, NULL, thr_start, NULL);
    if(ret != 0){
        printf("create thread error\n");
    }

    //线程分离
    //线程分离后无法被等待
    //pthread_detach(tid);

    //pathread_join(pthread_t tid, void** retval)
    //这里不能用二级指针 void**，因为对NULL或未初始化的指针进行解引用会引起段错误
    void* retval = NULL;     //获取线程返回值
    
    //线程等待
    int err = pthread_join(tid, &retval);
    if(err == EINVAL){
        printf("thread is not a joinable pthread\n");
        return -1;
    }

    //输出线程的返回值
    printf("thread eixt val is : %s\n", (char*)retval);
    return 0;
}
