int pthread_cond_destroy(pthread_cond_t *cond);
#include <stdio.h>
#include <pthread.h>
pthread_t pt1,pt2;
pthread_mutex_t mu;
pthread_cond_t cond;
int i = 1;
void * decrease(void * arg)
{
    while(1)
    {
        pthread_mutex_lock(&mu);
        if(++i)
        {
            printf("%d ",i);
            if(i != 1) printf("Error ");
            pthread_cond_broadcast(&cond);
            pthread_cond_wait(&cond,&mu);
        }
        sleep(1);
        pthread_mutex_unlock(&mu);
    }
}
void * increase(void * arg)
{
    while(1)
    {
        pthread_mutex_lock(&mu);
        if(i--)
        {
            printf("%d ",i);
            if(i != 0) printf("Error ");
            pthread_cond_broadcast(&cond);
            pthread_cond_wait(&cond,&mu);
        }
        sleep(1);
        pthread_mutex_unlock(&mu);
    }
}
int main()
{
    pthread_create(&pt2,NULL,increase,NULL);
    pthread_create(&pt1,NULL,decrease,NULL);
    pthread_join(pt1,NULL);
    pthread_join(pt2,NULL);
}