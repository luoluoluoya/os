//
// Created by Administrator on 2021/7/5.
//

#include "pthread_pool.h"


// 初始化线程池, 创建thread_num个线程. 返回0表示成功
int init_pool(pthread_pool *pool, unsigned int thread_num) {
    assert(!pthread_mutex_init(&pool->mutex, NULL));
    assert(!pthread_cond_init(&pool->cond, NULL));
    pool->shut_down = 0;
    assert((pool->task_list = malloc(sizeof(task))));
    pool->task_list->next = NULL;
    assert((pool->tids = malloc(thread_num * sizeof(pthread_t))));
    pool->active_threads = thread_num;
    pool->max_waiting_tasks = MAX_WAITING_TASKS;
    pool->cur_waiting_tasks = 0;
    for (int i = 0; i < thread_num; ++i) {
        assert(!pthread_create(&pool->tids[i], NULL, routine, (void *) pool));
    }
}

// 调度程序
void *routine(void *arg) {
    pthread_pool *pool = (pthread_pool *) arg;
    printf("pthread create: %ld\n", pthread_self());
    while (1) {
        pthread_mutex_lock(&pool->mutex);
        while (!pool->cur_waiting_tasks && !pool->shut_down) {
            printf("pthread wait: %ld\n", pthread_self());
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }
        if (pool->shut_down) {
            printf("pthread exit: %ld\n", pthread_self());
            pthread_mutex_unlock(&pool->mutex);
            pthread_exit(NULL);
        }
        printf("pthread exec: %ld\n", pthread_self());
        assert(pool->cur_waiting_tasks > 0);
        assert(pool->task_list->next);
        task *work = pool->task_list->next;
        pool->task_list->next = work->next;
        pool->cur_waiting_tasks--;
        pthread_mutex_unlock(&pool->mutex);
        (*work->process)(work->arg);
        free(work);
        work = NULL;
    }
}

// 销毁线程池，销毁前要保证所有的任务已经完成
int destroy_pool(pthread_pool *pool) {  // todo 此处应当使用条件变量等待任务被执行完成
    if (pool->shut_down == 1)
        return -1;
    pool->shut_down = 1;
    pthread_cond_broadcast(&pool->cond);
    for (int i = 0; i < pool->active_threads; ++i) {
        assert(!pthread_join(pool->tids[i], NULL));
    }
    free(pool->tids);
    for (task *head = pool->task_list, *curr = head; head; curr = head) {
        head = head->next;
        free(curr);
    }
    return 0;
}

// 给任务队列增加任务，把process指向的任务(函数指针)和arg指向的参数保存到一个任务结点，添加到pool任务队列中。
int add_task(pthread_pool *pool, void *(*process)(void *), void *arg) {
    task *work = malloc(sizeof(task));
    assert(work);
    work->process = process;
    work->arg = arg;
    work->next = NULL;
    pthread_mutex_lock(&pool->mutex);
    assert(pool->cur_waiting_tasks + 1 <= pool->max_waiting_tasks);
    pool->cur_waiting_tasks++;
    task *head;
    for (head = pool->task_list; head->next; head = head->next);
    head->next = work;
    pthread_mutex_unlock(&pool->mutex);
    pthread_cond_signal(&pool->cond);
    return 0;
}

