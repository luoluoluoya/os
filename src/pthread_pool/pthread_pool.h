//
// Created by Administrator on 2021/7/5.
//

#ifndef CTEST1_PTHREAD_POOL_H
#define CTEST1_PTHREAD_POOL_H

#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

//表示线程池中最多有多少个线程
#define MAX_ACTIVE_THREADS 20

//表示线程池中最多有多少个任务
#define MAX_WAITING_TASKS 1024

// 任务
typedef struct _task {
    void *(*process)(void *);       // 任务结点表示的任务
    void *arg;                      // 任务参数
    struct _task *next;             // 下一个任务
} task;

// 线程池
typedef struct _pthread_pool {
    pthread_mutex_t mutex;          // 互斥锁: 用来保护这个“任务队列”
    pthread_cond_t cond;            // 条件变量: 表示“任务队列”是否有任务
    unsigned short int shut_down;   // 表示是否退出程序：1 / 0
    task *task_list;                // 任务队列(链表)，指向第一个需要指向的任务. 所有的线程都从任务链表中获取任务 "共享资源" (需要进行加锁访问)
    pthread_t *tids;                // 线程池中有多个线程，每一个线程都有tid, 需要一个数组去保存tid
    unsigned int active_threads;    // 线程池中正在服役的线程数，当前线程个数
    unsigned int max_waiting_tasks; // 线程池任务队列最大的任务数量
    unsigned int cur_waiting_tasks; // 线程池任务队列上当前有多少个任务
} pthread_pool;

// 初始化线程池, 创建thread_num个线程. 返回0表示成功
int init_pool(pthread_pool *pool, unsigned int thread_num);

// 调度程序
void *routine(void *);

// 销毁线程池，销毁前要保证所有的任务已经完成
int destroy_pool(pthread_pool *pool);

// 给任务队列增加任务，把process指向的任务(函数指针)和arg指向的参数保存到一个任务结点，添加到pool任务队列中。
int add_task(pthread_pool *pool, void *(*process)(void *), void *arg);


#endif //CTEST1_PTHREAD_POOL_H
