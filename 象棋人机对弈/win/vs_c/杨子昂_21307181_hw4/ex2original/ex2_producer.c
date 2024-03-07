#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#define NUMS 100 //表示生产，消费的次数
#define CAPACITY 5 //定义缓冲区最大值
int capacity = 0; //当前缓冲区的产品个数
pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;//互斥量
pthread_cond_t cond_prod = PTHREAD_COND_INITIALIZER;//生产者条件变量
pthread_cond_t cond_cons = PTHREAD_COND_INITIALIZER;//消费者条件变量

void *produce(void *args)
{
    int i;
    for (i = 0; i < NUMS; i++) {
        pthread_mutex_lock(&mylock);
        while (capacity == CAPACITY) { // 缓冲区满了，等待消费者消费
            printf("生产者线程等待...\n");
            pthread_cond_wait(&cond_prod, &mylock);
        }
        capacity++;
        printf("生产者生产了一个产品，当前产品数：%d\n", capacity);
        pthread_cond_signal(&cond_cons);//通知消费者可以消费了
        pthread_mutex_unlock(&mylock);
    }
    return NULL;
}

void *consume(void *args)
{
    int i;
    for (i = 0; i < NUMS; i++) {
        pthread_mutex_lock(&mylock);
        while (capacity == 0) { // 缓冲区空了，等待生产者生产
            printf("消费者线程等待...\n");
            pthread_cond_wait(&cond_cons, &mylock);
        }
        capacity--;
        printf("消费者消费了一个产品，当前产品数：%d\n", capacity);
        pthread_cond_signal(&cond_prod);//通知生产者可以生产了
        pthread_mutex_unlock(&mylock);
    }
    return NULL;
}

int main(int argc, char** argv) {
    int err;
    pthread_t produce_tid, consume_tid;
    void *ret;
    err = pthread_create(&produce_tid, NULL, produce, NULL);//创建线程
    if (err != 0) {
        printf("线程创建失败:%s\n", strerror(err));
        exit(-1);
    }
    err = pthread_create(&consume_tid, NULL, consume, NULL);
    if (err != 0)  {
        printf("线程创建失败:%s\n", strerror(err));
        exit(-1);
    }
    err = pthread_join(produce_tid, &ret);//主线程等到子线程退出
    if (err != 0) {
        printf("生产着线程分解失败:%s\n", strerror(err));
        exit(-1);
    }
    err = pthread_join(consume_tid, &ret);
    if (err != 0) {
        printf("消费者线程分解失败:%s\n", strerror(err));
        exit(-1);
    }
    
    return (EXIT_SUCCESS);
}