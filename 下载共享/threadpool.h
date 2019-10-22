#include <iostream>
#include <vector>
#include <pthread>
using namespace std;

typedef void(Handler_t*)(int);

class ThreadTask{
    private:
        int _data;  //数据
        Handler_t _handler; //数据的处理方式

    public:
        //传入数据和数据的处理方式
        void SetTask(int data, Handler_t handler){
            _data = data;
            _handler = handler;
        }

        //处理数据
        void TaskRun(){
            _handler(_data);
        }
};


class ThreadPool{
    private:
        int _maxThread;     //创建线程的最大数量
        int _maxQueue;  //任务队列的最大数量
        
        vector<ThreadTask> _queue;
        
        pthread_mutex_t _mutex;     //互斥锁
        pthread_cond_t _porducer;   //生产者
        pthtread_cond_t _consumer;  //消费者
    
    public:
        //构造函数
        ThreadPool(int maxThread = 10, int maxQueue = 10):
            _maxThread(maxThread), _maxQueue(maxQueue), _queue(maxQueue){
               
                pthread_mutex_init(&_mutex, NULL);
                pthread_cond_init(&_producer, NULL);
                pthread_cond_init(&_consumer, NULL);
            }

        //析构函数
        ~ThreadPool(){
            pthread_mutex_destroy(&_mutex);
            pthread_mutex_destroy(&_producer);
            pthread_mutex_destroy(&_consumer);
        }

        
        //创建线程
        bool PoolInit(){
            for(int i = 0; i < _maxThread; i++) {
                thread thr;
                
                //线程分离
                thr.join();
            }           
            
            return true;
        }








};
