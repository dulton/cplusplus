#include <string.h>
#include <pthread.h>
pthread_key_t p_key;
void another_func ()
{
  printf ("%s is running in another_func\n", (char *)pthread_getspecific(p_key));
}
void * thread_func (void * args)
{
  pthread_setspecific(p_key, args);
  printf ("%s is running in thread_func\n", (char *)pthread_getspecific(p_key));
  another_func ();
  
}
int main (int argc, char * argv[])
{
  pthread_t pa, pb;
  pthread_key_create(&p_key, NULL);
  
  pthread_create ( &pa, NULL, thread_func, "Thread A");
  pthread_create ( &pb, NULL, thread_func, "Thread B");
  pthread_join (pa, NULL);
  pthread_join (pb, NULL);

  return 0;
}