#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


typedef struct Threadpool Threadpool;


//�̳߳س�ʼ��
Threadpool* threadpoolcreate(int min, int max, int queuesize);

void* manager(void* arg);


void* work(void* arg);



void threadexit(Threadpool* pool);//�߳��˳�

//�������
void threadpooladdtask(Threadpool* pool, void(*func)(void*), void* arg);

//��ȡ�̳߳��й������̸߳���
int threadpool_get_busynum(Threadpool* pool);

//��ȡ���ŵ��̸߳���
int threadpool_get_livenume(Threadpool* pool);

//�ͷ��̳߳�
int threadpool_destry(Threadpool* pool);


#endif // _THREADPOOL_H
