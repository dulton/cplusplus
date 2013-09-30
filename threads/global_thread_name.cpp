#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define MAXLENGTH 20
char threadName[MAXLENGTH];
pthread_mutex_t sharedMutex=PTHREAD_MUTEX_INITIALIZER;
void another_func ()
{
  printf ("%s is running in another_func\n", threadName);
}
void * thread_func (void * args)
{
  pthread_mutex_lock(&sharedMutex);
  strncpy (threadName, (char *)args, MAXLENGTH-1);
  printf ("%s is running in thread_func\n", threadName);
  another_func ();
  pthread_mutex_unlock(&sharedMutex);
  
}
int main (int argc, char * argv[])
{
  pthread_t pa, pb;
  pthread_create ( &pa, NULL, thread_func, "Thread A");
  pthread_create ( &pb, NULL, thread_func, "Thread B");
  pthread_join (pa, NULL);
  pthread_join (pb, NULL);

  return 0;
}