//利用posix信号量实现生产者消费者模型

#include <iostream>
#include <thread>
#include <vector>
#include <semaphore.h>
using namespace std;

class RingQueue{
    private:
        //安全队列
        vector<int> _queue;
        int _capacity;  //容量
        int _write;     //写索引
        int _read;      //读索引

        //条件变量
        sem_t _mutex;   //锁
        sem_t _idle_space;    //空闲位置
        sem_t _data_space;    //已有数据位置
    public:
        //构造函数
        RingQueue(const int capacity = 10):
            _capacity(capacity), _queue(capacity),
            _write(0), _read(0){
                //初始化条件变量
                sem_init(&_mutex, 0, 1);
                sem_init(&_data_space, 0, 0);
                sem_init(&_idle_space, 0, capacity);
            }

        //析构函数
        ~RingQueue(){
            sem_destroy(&_mutex);
            sem_destroy(&_idle_space);
            sem_destroy(&_data_space);
        }

        //入队
        bool Push(const int data){
            //判断是否有空闲位置
            sem_wait(&_idle_space);
            //加锁
            sem_wait(&_mutex);
            //入队
            _queue[_write] = data;
             _write = (_write + 1) % _capacity;
            //解锁
            sem_post(&_mutex);
            //唤醒
            sem_post(&_data_space);

            return true;
        }

        //出队
        bool Pop(int& data){
            //判断是否有数据
            sem_wait(&_data_space);
            //加锁
            sem_wait(&_mutex);
            //出队
            data = _queue[_read];
            _read = (_read + 1) % _capacity;
            //解锁
            sem_post(&_mutex);
            //唤醒
            sem_post(&_idle_space);

            return true;
        }
};

//生产者线程入口函数
void Producer(RingQueue* q){
    int data = 0;
    while(1){
        q->Push(data);
        cout << "生产了一个数据-----" << data++ << endl;
    }

    return;
}

//消费者线程入口函数
void Consumer(RingQueue* q){
    int data;
    while(1){
        q->Pop(data);
        cout << "消费了一个数据---------" << data << endl;
    }

    return;
}

int main(){
    RingQueue q;
    vector<thread> con_list(4);
    vector<thread> pro_list(4);

    //消费者线程
    for(int i = 0; i < 4; i++){
        con_list[i] = thread(Consumer, &q);
    }

    //生产者线程
    for(int i = 0; i < 4; i++){
        pro_list[i] = thread(Producer, &q);
    }

    //线程等待
    for(int i = 0; i < 4; i++){
        con_list[i].join();
        pro_list[i].join();
    }

    return 0;
}
