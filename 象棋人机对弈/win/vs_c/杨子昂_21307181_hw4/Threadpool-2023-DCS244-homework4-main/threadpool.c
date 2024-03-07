#include "threadpool.h"

threadpool* Pool_init(int Maxthread)
{

	threadpool* pool;
	pool = (threadpool*)malloc(sizeof(threadpool));

	pool->flag = 1;
	

	/*******************************************************************/

    pool->poolhead = NULL;
    pool->jobnum = 0;
    pool->Maxthread = Maxthread;
	pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * Maxthread);
    for(int i = 0; i < Maxthread; ++i)
    {
        if(pthread_create(&pool->threads[i], NULL, (void*)Job_running, pool))
        {
            perror("pthread_create");
            exit(-1);
        }
    }
    sem_init(&(pool->sem), 0, 1); //初始化互斥信号量
    sem_init(&(pool->empty),0,Maxthread);
    sem_init(&(pool->full),0,0);

   	/******************************************************************/
	
	return pool;
}

void Job_running(threadpool* pool)
{
	while(pool->flag){
        if(pool->flag == 0 || pool->jobnum == 0)pthread_exit(NULL);
	/*******************************************************************/

      // 等待信号量，即等待任务的到来

	

	// 取出任务队列中的一个任务节点
	Jobnode job = Pop(pool);

	// 执行任务
    //if(job.pf != NULL){
	job.pf(job.arg);

	// 删除任务节点
	free(job.arg);
    //}

 
   	/******************************************************************/
	}
	pthread_exit(0);
}


int Add_job(threadpool* pool , function_t pf , void* arg)
{
	/*******************************************************************/

    Jobnode newnode;
    newnode.pf = pf;
    newnode.arg = arg;
    if(Push(pool, newnode)==-1)
    {
        return -1;
    }
      // 将新任务节点加入到任务队列中

 // 唤醒一个线程去处理任务

    return 0;

   	/******************************************************************/
}

int Push(threadpool* pool , Jobnode data )
{
	/*******************************************************************/

    //等待直到有空闲的位置
    
    sem_wait(&(pool->empty));
    //获取锁，保证线程安全
    sem_wait(&(pool->sem));
    //判断任务队列是否已满
    if(pool->jobnum == pool->Maxthread)
    {
        //释放锁
        sem_post(&(pool->sem));
        //释放一个空闲位置信号量
        sem_post(&(pool->empty));
        return -1;//队列如果为满则返回错误
    }
    //创建新的任务节点
    threadjob* cur = (threadjob*)malloc(sizeof(threadjob));

    cur->data = data;
    cur->next = NULL; 
    //将新的任务节点添加到队列中
    if(pool->poolhead == NULL) 
    {
        //如果poolhead为空则直接创建
        pool->poolhead = cur;
    }
    else 
    {
        threadjob* head = pool->poolhead;
        while(head->next) 
        {
            head = head->next;
        }
        head->next = cur;
    }
    pool->jobnum ++ ;
    
    //释放锁和一个空闲位置信号量
    sem_post(&(pool->sem));
    sem_post(&(pool->full));
    return 1;

   	/******************************************************************/
}

Jobnode Pop(threadpool* pool)
{
	/*******************************************************************/

    threadjob* job;
    Jobnode empty_job = {NULL, NULL};
    sem_wait(&(pool->full));
    sem_wait(&(pool->sem));
    if(pool->poolhead == NULL) {
        sem_post(&(pool->sem));
        sem_post(&(pool->full));
        return empty_job;
    }
    job = pool->poolhead;
    pool->poolhead = job->next;
    Jobnode node = job->data;
    free(job);
    pool->jobnum--;
    sem_post(&(pool->sem));
    sem_post(&(pool->empty));
   
    return node;

   	/******************************************************************/
}

int Delete_pool(threadpool* pool)
{
	
	pool->flag = 0;
	/*******************************************************************/
    //sem_post(&(pool->sem));
    sem_destroy(&(pool->sem));
    sem_destroy(&(pool->empty));
    sem_destroy(&(pool->full));
    
    // 删除任务队列中的任务节点
    threadjob* curjob = pool->poolhead;
    while(curjob!=NULL) {
        threadjob* nextjob = curjob;
        curjob = curjob->next;
        free(nextjob->data.arg);
        free(nextjob);
    }

    // 等待所有线程结束
    for(int i = 0; i < pool->Maxthread; ++i) {
        pthread_join(pool->threads[i], NULL);
    }

    // 释放线程池和信号量所占的空间
    free(pool->threads);

    free(pool);

    return 0;

   	/******************************************************************/
	
}
