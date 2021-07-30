#include "threadpool.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include<unistd.h>


const int NUMBRE = 2;
//任务结构体
typedef struct Task
{

	void(*function)(void*arg);
	void* arg;

}Task;


//线程池结构体

struct Threadpool {


	Task* taskQ;
	int queuecapacity;  //任务的容量
	int queue_cur_size;//当前的任务
	int queuefront;
	int queuerear;



	//管理者
	pthread_t managerid;
	pthread_t* threadids;//工作的线程
	int minnum;//最小线程数量
	int maxnum;


	int busynum;//忙的线程数量
	int livenum;//活着的线程数量
	int exitnum;//要销毁的线程数量

	pthread_mutex_t mutexpool;
	pthread_mutex_t mutexbusy;

	pthread_cond_t notfull;
	pthread_cond_t notempty;

	int shoutdownpool;

};  




Threadpool* threadpoolcreate(int min, int max, int queuesize)
{
	Threadpool* pool = (Threadpool*)malloc(sizeof(Threadpool));
	do {
	
		if (pool == NULL) {
		
			printf("malloc Threadpool fail...\n");
			break;
		}

		pool->threadids = (pthread_t*)malloc(sizeof(pthread_t) * max);
		if (pool->threadids == NULL) {
		
			printf("malloc pool->threadids fail..\n");
			break;
		}

		memset(pool->threadids, 0, sizeof(pthread_t) * max);
		pool->maxnum = max;
		pool->minnum = min;
		pool->busynum = 0;
		pool->livenum = min;
		pool->exitnum = 0;


		if (pthread_mutex_init(&pool->mutexbusy, NULL) ||

			pthread_mutex_init(&pool->mutexpool, NULL) ||
			pthread_cond_init(&pool->notempty, NULL) ||
			pthread_cond_init(&pool->notfull,NULL)) {


						printf("mutexpool or cond fail...\n");

			break;
		
		}


		pool->taskQ = (Task*)malloc(sizeof(Task) * queuesize);
		pool->queuecapacity = queuesize;
		pool->queuefront = 0;
		pool->queue_cur_size = 0;
		pool->queuerear = 0;

		pool->shoutdownpool = 0;


		//创建线程
		pthread_create(&pool->managerid, NULL, manager, pool);
		

		for (int i = 0; i < min; ++i) {
		
			pthread_create(&pool->threadids[i], NULL, work, pool);
			
		}
		return pool;
	} while (0);


	if (pool && pool->threadids) {
	
		free(pool->threadids);
	}
	if (pool && pool->taskQ) {
	
		free(pool->taskQ);
	
	}

	if (pool) {
	
	free(pool);
	}

	

	return NULL;
}









void* manager(void* arg)
{
	Threadpool* pool = (Threadpool*)arg;
	while (!pool->shoutdownpool) {
	
		sleep(3);
		pthread_mutex_lock(&pool->mutexpool);
		int queue_cur_size = pool->queue_cur_size;
		int livenum = pool->livenum;
		pthread_mutex_unlock(&pool->mutexpool);


		pthread_mutex_lock(&pool->mutexbusy);
		int busynum = pool->busynum;
		pthread_mutex_unlock(&pool->mutexbusy);


		//创建线程

		if (queue_cur_size > livenum && livenum < pool->maxnum) {
		
		
			pthread_mutex_lock(&pool->mutexpool);
			int current = 0;
			for (int i = 0; i < pool->maxnum&&current<NUMBRE&&pool->livenum<pool->maxnum; ++i) {
			
				if (pool->threadids[i] == 0) {
				
					pthread_create(&pool->threadids[i], NULL, work, pool);
					current++;
					pool->livenum;

			
				}
			
			}

			pthread_mutex_unlock(&pool->mutexpool);
		
		
		}

		if (busynum * 2 < livenum && livenum > pool->minnum) {
		
		
			pthread_mutex_lock(&pool->mutexpool);
			pool->exitnum = NUMBRE;
			pthread_mutex_unlock(&pool->mutexpool);
			for (int i = 0; i < NUMBRE; i++) {
			
				pthread_cond_signal(&pool->notempty);
			
			}
				
		}
	
	}

	return NULL;
}












void* work(void* arg)

{
	Threadpool* pool = (Threadpool*)arg;


	while (1) {
	
	
		pthread_mutex_lock(&pool->mutexpool);
		while (pool->queue_cur_size == 0 && !pool->shoutdownpool) {
		
		
			pthread_cond_wait(&pool->notempty, &pool->mutexpool);
			if (pool->exitnum > 0) {
			
				pool->exitnum--;
				if (pool->livenum > pool->minnum) {
				
					pool->livenum--;
					pthread_mutex_unlock(&pool->mutexpool);
					threadexit(pool);

				
				}
			
			
			}
			
		}
	
		if (pool->shoutdownpool) {
		
			pthread_mutex_unlock(&pool->mutexpool);
			threadexit(pool);
		
		}

		Task task;
		task.function = pool->taskQ[pool->queuefront].function;
		task.arg = pool->taskQ[pool->queuefront].arg;

		pool->queuefront = (pool->queuefront + 1) % pool->queuecapacity;
		pool->queue_cur_size--;


		pthread_cond_signal(&pool->notfull);

		pthread_mutex_unlock(&pool->mutexpool);

		printf("thread %ld start working...\n",pthread_self());


		pthread_mutex_lock(&pool->mutexbusy);
		pool->busynum++;
		pthread_mutex_unlock(&pool->mutexbusy);

		task.function(task.arg);
		free(task.arg);
		task.arg = NULL;

		printf("thread %ld end working...\n",pthread_self());

		pthread_mutex_lock(&pool->mutexbusy);
		pool->busynum--;
		pthread_mutex_unlock(&pool->mutexbusy);

	}

	return NULL;
}




void threadexit(Threadpool* pool)
{

	pthread_t tid = pthread_self();
	for (int i = 0; i < pool->maxnum; ++i) {
	
	
		if (pool->threadids[i] == tid) {
		
			pool->threadids[i] = 0;
			printf("threadexit() calledback,%ld exiting..\n", tid);
		
			break;
		
		}
	
		
	}
	pthread_exit(NULL);
}





void threadpooladdtask(Threadpool* pool, void(*func)(void*), void* arg)
{
	pthread_mutex_lock(&pool->mutexpool);
	while (pool->queue_cur_size == pool->queuecapacity && !pool->shoutdownpool) {
	
		pthread_cond_wait(&pool->notfull, &pool->mutexpool);

	
	}

	if (pool->shoutdownpool) {
	
	
		pthread_mutex_unlock(&pool->mutexpool);
		return;
	}

	pool->taskQ[pool->queuerear].function = func;
	pool->taskQ[pool->queuerear].arg = arg;
	
	pool->queuerear = (pool->queuerear + 1) % pool->queuecapacity;
	pool->queue_cur_size++;

	pthread_cond_signal(&pool->notfull);

	pthread_mutex_unlock(&pool->mutexpool);



}





int threadpool_get_busynum(Threadpool* pool)
{
	pthread_mutex_lock(&pool->mutexbusy);
	int busynume = pool->busynum;
	pthread_mutex_unlock(&pool->mutexbusy);
	return busynume;
}

int threadpool_get_livenume(Threadpool* pool)
{
	pthread_mutex_lock(&pool->mutexpool);
	int alivenum = pool->livenum;
	pthread_mutex_unlock(&pool->mutexpool);


	return alivenum;
}






int threadpool_destry(Threadpool* pool)
{
	if (pool == NULL) {
		return -1;
	}

	pool->shoutdownpool = 1;

	pthread_join(pool->managerid, NULL);


	for (int i = 0; i < pool->livenum; ++i) {

		pthread_cond_signal(&pool->notempty);

	}

	if (pool->taskQ) {
		free(pool->taskQ);


	}
	if (pool->threadids) {

		free(pool->threadids);


	}

	pthread_mutex_destroy(&pool->mutexbusy);
	pthread_mutex_destroy(&pool->mutexpool);
	pthread_mutex_destroy(&pool->notempty);
	pthread_mutex_destroy(&pool->notfull);



	free(pool);
	pool = NULL;
	return 0;
}