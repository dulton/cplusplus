#include <string.h>
#include <pthread.h>
#include <stdio.h>
#define MAXLENGTH 20
void another_func(const char* threadName)
{
    printf("%s is running in another_func\n",threadName);
}
void * thread_func(void *args)
{
    char threadName[MAXLENGTH];
    strncpy(threadName,(char*)args,MAXLENGTH-1);
    printf("%s is running in thread_func\n",threadName);
    another_func(threadName);
}
int main(int argc,char* argv[])
{
    pthread_t pa,pb;
    pthread_create(&pa, NULL, thread_func, "Thread A");
    pthread_create(&pb, NULL, thread_func, "Thread B");
    pthread_join(pa, NULL);
    pthread_join(pb, NULL);
    
    return 0;
}