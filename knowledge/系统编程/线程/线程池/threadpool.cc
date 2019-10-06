#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
using namespace std;

typedef void(*handler_t)(int);
//任务类
//数据 + 数据的处理方式
class MyTask{
    private:
        int _data;
        handler_t _handler;
    public:
        //构造函数
        MyTask(int data, handler_t handler):
            _data(data), _handler(handler){}

        //处理
        void Run(){
            _handler(_data);
        }
};

class ThreadPool{
    private:
        pthread_mutex_t _mutex;     //锁变量
        pthread_cond_t _consumer;   //消费者
        pthread_cond_t _producer;   //生产者
        
        queue<MyTask> _queue;   //任务队列
        int _capacity;  //容量
        
        vector<thread> _list;    //储存线程的描述符
        int _max;   //线程的最大数量
        int _cur;   //当前线程数量
        bool _flag;     //结束标志
        
        //线程入口函数（出队）
        void Thr_start(){
            while(1){
                //加锁
                pthread_mutex_lock(&_mutex);
                //潘顿是否有需要被处理的任务
                while(_queue.empty()){
                    //判断是否需要退出
                    if(_flag == true){
                        cout << "thread exit" << pthread_self() << endl;

                        //解锁
                        pthread_mutex_unlock(&_mutex);
                        _cur--;
                        return;
                    }

                    //唤醒 + 解锁
                    pthread_cond_wait(&_consumer, &_mutex);
                }

                //出队
                MyTask task =  _queue.front();
                _queue.pop();
                
                //解锁
                pthread_mutex_unlock(&_mutex);

                //唤醒
                pthread_cond_signal(&_producer);
                
                //先解锁再处理数据
                //避免在此处阻塞
                task.Run();
            }
        }   

    public:
        //构造函数
        ThreadPool(int capacity = 10, int max = 5):
            _capacity(capacity), _max(max),
            _list(max), _flag(false){
                pthread_mutex_init(&_mutex, NULL);
                pthread_cond_init(&_consumer, NULL);
                pthread_cond_init(&_producer, NULL);
            }

        //析构函数
        ~ThreadPool(){
            pthread_mutex_destroy(&_mutex);
            pthread_cond_destroy(&_consumer);
            pthread_cond_destroy(&_producer);
        }
        
        //创建线程
        bool Init(){
            _cur = 0;
            for(int i = 0; i < _max; i++){
                _list[i] = thread(&ThreadPool::Thr_start, this);
                
                _cur++;

                //线程分离
                _list[i].detach();
            }
            return true;
        }

        //添加任务（入队）
        bool AddTask(MyTask& task){
            //加锁
            pthread_mutex_lock(&_mutex);
            
            //判断队列中是否有多余的位置
            while(_queue.size() == _capacity){
                //等待 + 解锁
                pthread_cond_wait(&_producer, &_mutex);
            }

            //入队
            _queue.push(task);
            
            //解锁
            pthread_mutex_unlock(&_mutex);

            //唤醒
            pthread_cond_signal(&_consumer);

            return true;
        }


        //结束线程
        void Stop(){
            //加锁
            pthread_mutex_lock(&_mutex);

            //改变标志位
            _flag = true;

            //解锁
            pthread_mutex_unlock(&_mutex);
        
            //避免有未被处理的线程，广播唤醒
            while(_cur > 0){
                pthread_cond_broadcast(&_consumer);

                usleep(1000);
            }
        }
};


//测试
void Test(int data){
    srand(time(NULL));
    int n = rand() % 5;
    stringstream ss;
    ss << "thread: " << pthread_self() << " processing data: " << data;
    ss << " and sleep: " << n << endl;
    cout <<ss.str();

    sleep(n);
}

int main(){
    //创建线程池
    ThreadPool tp;
    //初始化
    tp.Init();

    //添加任务
    for(int i = 0; i < 5; i++){
        MyTask task(i, Test);
        tp.AddTask(task);
    }
    
    //停止
    tp.Stop();
    
    return 0;
}
