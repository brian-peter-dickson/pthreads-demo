/******************************************************************************
* main-threads-readonly.c
******************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int     count = 0;

/* Queue mutex used on heads of queue only (global, tcp, udp) */
/* Might not be necessary (global or specifics or both)? Think about it */
pthread_mutex_t global_queue_head, tcp_queue_head, udp_queue_head;

/* Primary thread pools, standard condition signaling (no lending being used) */
pthread_cond_t global_queue_head_cv, tcp_queue_head_cv, udp_queue_head_cv;

/* Primary thread pools, condition signaling only if lending thread capacity actively being used */
pthread_cond_t tcp_pool_onloan_cv, udp_pool_onloan_cv;

/* Secondary thread pools, condition signaling only if borrowing thread capacity actively being used */
pthread_cond_t tcp_standby_inuse_cv, udp_standby_inuse_cv;

#define GLOBAL_NORMAL 0

#define PRIMARY_NORMAL 0
#define PRIMARY_ONLOAN 1

char *primary_state[] = {"Normal","On Loan"};

/* Does it matter which is 0 vs 1? */
#define SECONDARY_NORMAL 0
#define SECONDARY_INUSE 1

char *secondary_state[] = {"Unused","In Use"};

#define NUM_THREADS 10

struct arg_struct {
  long id;
  long state;
  char *label;
  };

struct arg_struct arg_thing[] = {
  { 0, GLOBAL_NORMAL, "global queue", },
  { 1, PRIMARY_NORMAL, "tcp queue", },
  { 2, PRIMARY_NORMAL, "udp queue", },
};

void *queue_mgr_global(void *t) 
{
  int i;
  struct arg_struct x;
  x = *(struct arg_struct *)t;
  long my_id = x.id;
  /* printf("arg_thing[0] = '%ld'\n",my_id); */

  pthread_exit(NULL);
}

void *queue_mgr_udp(void *t) 
{
  int i;
  struct arg_struct x;
  x = *(struct arg_struct *)t;
  long my_id = x.id;

  pthread_exit(NULL);
}

void *queue_mgr_tcp(void *t) 
{
  int i;
  struct arg_struct x;
  x = *(struct arg_struct *)t;
  long my_id = x.id;

  pthread_exit(NULL);
}

/* reference code only */
/*
void *foo2(void *t) 
{
  long my_id = *(long *)t;

  pthread_mutex_lock(&count_mutex);
  pthread_cond_wait(&count_threshold_cv, &count_mutex);
  pthread_mutex_unlock(&count_mutex);
  pthread_exit(NULL);
}
*/


int main(int argc, char *argv[])
{
  int i, rc; 
  FILE *infile;
  int size=1024;
  char *buf,*ptr;

  /* FIXME
   *
   * [DONE] Change t1/2/3 to array references
   * [DONE] Use array of structs instead of atomic types for thread-specific *arg data
   * 	    (one array element per called routine, ie pass reference to array member == struct)
   * Use set(s) of states for each major flavor - #define (current) or maybe 'enum'?
   * Control thread cond_wait via struct member for each thread (or collectively?)
   * Use #define things for INT values, become index -> strings (implicitly as array elements)
   */


  /* printf("arg_thing[0] = '%ld' (label = '%s')\n",arg_thing[0].id,arg_thing[0].label); */

  /* WAS long t1=1, t2=2, t3=3; */
  pthread_t threads[NUM_THREADS];
  pthread_attr_t attr;

  /* Initialize mutex objects */
  pthread_mutex_init(&global_queue_head, NULL);
  pthread_mutex_init(&tcp_queue_head, NULL);
  pthread_mutex_init(&udp_queue_head, NULL);
  /* Initialize condition objects */
  pthread_cond_init(&global_queue_head_cv, NULL);
  pthread_cond_init(&tcp_queue_head_cv, NULL);
  pthread_cond_init(&udp_queue_head_cv, NULL);
  pthread_cond_init(&tcp_pool_onloan_cv, NULL);
  pthread_cond_init(&udp_pool_onloan_cv, NULL);
  pthread_cond_init(&tcp_standby_inuse_cv, NULL);
  pthread_cond_init(&udp_standby_inuse_cv, NULL);

  /* Be sure threads are joinable */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  

  printf("Test of reading combined input data...\n");
  /* check arguments */
  printf("argc = %d\n",argc);
  if(argc != 2){
    fprintf(stderr,"Bad parameter count, should be one argument, file to open\n");
    exit(1);
    }
  /* do fopen here */
  infile=fopen(argv[1],"r");
  if(!infile || infile == NULL){
    /* check return condition for problem and die if appropriate */
    fprintf(stderr,"Bad file, could not open file %s\n",argv[1]);
    exit(1);
    }

  /* allocate buf only now */
  buf=(char *)malloc(size);
  if(buf == NULL){
    /* something went wrong, just fail out */
    goto cleanup;
    }

  /* Move this into a routine to be called repeatedly, with some flow control perhaps */

  /* read file contents */
  /* but do nothing with them (yet) */
  while(!feof(infile)){
    ptr=fgets(buf,size,infile);
    /* error or EOF, figure out which and do the right thing */
    if(ptr == NULL && feof(infile)){
      /* do nothing */
      }
    else if(ptr == NULL && !feof(infile)){
      /* die */
      fprintf(stderr,"Something bad happened during fgets\n");
      goto cleanup;
      }
    else {
      printf("Read line: %s",ptr);
      /* check for no trailing newline, should only happen on last line */
      /* how? */
      size_t ptrlen;
      char *lastchar;
      ptrlen = strnlen(ptr,size);
      lastchar=&ptr[ptrlen-1];
      if(*lastchar != '\n'){
        printf("\n");
	}
      }
    }
  /* End of thing to move */

  /* Add in threads for passing off main input to queue runners */
  /* (threads and splitting file to them for ignoring them by type) */
  /* After that, add standard queue handler threads without special logic */
  /* Got about to here */
  /* After that, add lend/borrow thread exchanges between queues */
  /* (No thread create/destroy, rather use locked/unlocked states and secondary pools) */
  /* (E.g. each queue has some spare threads, which can be activated by the queue runner) */
  /* (Activating a spare requires setting lock/block on thread from another queue's main pool.) */
  /* (Spare threads must signal queue runner to confirm other queue doesn't need threads yet.) */

  /* Group these in loops when we are doing a bunch of them... */
  pthread_create(&threads[0], &attr, queue_mgr_global, (void *)&arg_thing[0]);
  pthread_create(&threads[0], &attr, queue_mgr_udp, (void *)&arg_thing[1]);
  pthread_create(&threads[1], &attr, queue_mgr_tcp, (void *)&arg_thing[2]);

  /* Safe for unused thread values, probably */
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

cleanup:
  /* close file */
  fclose(infile);
  
  /* free buf */
  free(buf);

  /* standard clean-up stuff */
  pthread_attr_destroy(&attr);

  /* Clean up mutex objects */
  pthread_mutex_destroy(&global_queue_head);
  pthread_mutex_destroy(&tcp_queue_head);
  pthread_mutex_destroy(&udp_queue_head);
  /* Clean up condition objects */
  pthread_cond_destroy(&global_queue_head_cv);
  pthread_cond_destroy(&tcp_queue_head_cv);
  pthread_cond_destroy(&udp_queue_head_cv);
  pthread_cond_destroy(&tcp_pool_onloan_cv);
  pthread_cond_destroy(&udp_pool_onloan_cv);
  pthread_cond_destroy(&tcp_standby_inuse_cv);
  pthread_cond_destroy(&udp_standby_inuse_cv);

  pthread_exit (NULL);

}
