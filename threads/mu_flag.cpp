#include <stdio.h>
#include <pthread.h>
void * pthread_func_test(void * arg);
pthread_mutex_t mu;
int flag = 0;
int main()
{
    int i;
    pthread_t pt;
    
    pthread_mutex_init(&mu,NULL);
    pthread_create(&pt,NULL,pthread_func_test,NULL);
    for(i = 0; i < 3; i++)
    {
        pthread_mutex_lock(&mu);
        if(flag == 0)
                printf("主线程ID是：%lu ",pthread_self());
        flag = 1;
        pthread_mutex_unlock(&mu);
        sleep(1);
    }
    pthread_join(pt, NULL);
    pthread_mutex_destroy(&mu);
}
void * pthread_func_test(void * arg)
{
    int j;
    for(j = 0; j < 3; j++)
    {
        pthread_mutex_lock(&mu);
        if(flag == 1)
            printf("新线程ID是：%lu ",pthread_self());
        flag == 0;
        pthread_mutex_unlock(&mu);
        sleep(1);
    }
}