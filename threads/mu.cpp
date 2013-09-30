#include <stdio.h>
#include <pthread.h>

using namespace std;

void * pthread_func_test(void * arg);
pthread_mutex_t mu;
int main()
{
    int i;
    pthread_t pt;
    
    pthread_mutex_init(&mu,NULL); //声明mu使用默认属性，此行可以不写
    pthread_create(&pt,NULL,pthread_func_test,NULL);
    for(i = 0; i < 3; i++)
    {
        pthread_mutex_lock(&mu);
        printf("主线程ID是：%lu ",pthread_self()); //pthread_self函数作用：获得当前线程的id
        pthread_mutex_unlock(&mu);
        sleep(1);
    }
}
void * pthread_func_test(void * arg)
{
    int j;
    for(j = 0; j < 3; j++)
    {
        pthread_mutex_lock(&mu);
        printf("新线程ID是：%lu ",pthread_self());
        pthread_mutex_unlock(&mu);
        sleep(1);
    }
}