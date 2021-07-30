#include "threadpool.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


void taskl(void* arg) {

    int num=*(int*)arg;
    printf("thresd task %ld is working,number=%d\n",pthread_self(),num);
    sleep(1);
}

int main()
{
    Threadpool* pool = threadpoolcreate(5, 10, 100);

    for (int i = 100; i < 150; i++) {
    
        int* num = (int*)malloc(sizeof(int));
        *num = i;
        threadpooladdtask(pool, taskl, num);      
    }
    sleep(30);
    threadpool_destry(pool);
    return 0;
}