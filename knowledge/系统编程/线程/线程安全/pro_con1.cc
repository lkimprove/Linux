//用条件变量、互斥锁和一个线程安全队列
//实现生产者消费者模型

#include <iostream>
#include <queue>
#include <pthread.h>
using namespace std;

//线程安全队列
class BlockQueue{
    private:
        queue<int> _queue;          //队列
        int _capacity;               //队列容量
        pthread_cond_t _consumer;    //消费者条件变量
        pthread_cond_t _producer;    //生产者条件变量
        pthread_mutex_t _mutex;      //互斥锁

    public:
        //构造函数
        BlockQueue(const int capacity = 10):
            _capacity(capacity)
        {
            //初始化条件变量和互斥锁
            pthread_cond_init(&_consumer, NULL);
            pthread_cond_init(&_producer, NULL);
            pthread_mutex_init(&_mutex, NULL);
        }

        //析构函数
        ~BlockQueue(){
            //销毁条件变量和互斥锁
            pthread_cond_destroy(&_consumer);
            pthread_cond_destroy(&_producer);
            pthread_mutex_destroy(&_mutex);
        }

        //入队
        bool Push(const int data){
            //加锁
            pthread_mutex_lock(&_mutex);

            //检查队列容量
            //循环判断临界资源，防止被唤醒后，未判断是否可以操作临界资源，而直接操作临界资源
            while(_queue.size() == _capacity){
                //等待 + 解锁
                pthread_cond_wait(&_producer, &_mutex);
            }

            //入队
            _queue.push(data);
            //打印
            cout << "生产一个数据！" << data << endl;

            //解锁
            pthread_mutex_unlock(&_mutex);
            //唤醒
            pthread_cond_signal(&_consumer);

            return true;
        }

        //出队
        bool Pop(int& data){
            //加锁
            pthread_mutex_lock(&_mutex);

            //检查队列是否为空
            //循环判断临界资源，防止被唤醒后，未判断是否可以操作临界资源，而直接操作临界资源
            while(_queue.empty()){
                //等待 + 解锁
                pthread_cond_wait(&_consumer, &_mutex);
            }

            //出队
            data = _queue.front();
            _queue.pop();
            //打印
            cout  << "消费一个数据！！" << data << endl;

            //解锁
            pthread_mutex_unlock(&_mutex);
            //唤醒
            pthread_cond_signal(&_producer);

            return true;
        }
};


//生产者线程入口函数
void* Produce(void* arg){
    BlockQueue* q = (BlockQueue*) arg;

    int data = 0;
    while(1){
        //生产数据
        q->Push(data);
        data++;
    }

    return NULL;
}

//消费者线程入口函数
void* Consume(void* arg){
    BlockQueue* q = (BlockQueue*) arg;

    int data;
    while(1){
        //消费数据
        q->Pop(data);
    }

    return NULL;
}


int main(){
    //线程安全队列
    BlockQueue bq;
    
    pthread_t ctid, ptid;
    
    //创建生产者线程
    int ret = pthread_create(&ptid, NULL, Produce, (void*) &bq);
    if(ret != 0){
        cout << "生产者线程创建失败！" << endl;
        return -1;
    }

    //创建消费者线程
    ret = pthread_create(&ctid, NULL, Consume, (void*) &bq);
    if(ret != 0){
        cout << "消费者线程创建失败！" << endl;
        return -1;
    }

    //  线程等待
    pthread_join(ptid, NULL);
    pthread_join(ctid, NULL); 

    return 0;
}
