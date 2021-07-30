#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


typedef struct Threadpool Threadpool;


//线程池初始化
Threadpool* threadpoolcreate(int min, int max, int queuesize);

void* manager(void* arg);


void* work(void* arg);



void threadexit(Threadpool* pool);//线程退出

//添加任务
void threadpooladdtask(Threadpool* pool, void(*func)(void*), void* arg);

//获取线程池中工作的线程个数
int threadpool_get_busynum(Threadpool* pool);

//获取活着的线程个数
int threadpool_get_livenume(Threadpool* pool);

//释放线程池
int threadpool_destry(Threadpool* pool);


#endif // _THREADPOOL_H
