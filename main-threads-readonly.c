/******************************************************************************
* main-only.c
******************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int     count = 0;
pthread_mutex_t count_mutex;
pthread_cond_t count_threshold_cv;

void *foo1(void *t) 
{
  int i;
  long my_id = (long)t;

  pthread_exit(NULL);
}

void *foo2(void *t) 
{
  long my_id = (long)t;

  pthread_mutex_lock(&count_mutex);
  pthread_cond_wait(&count_threshold_cv, &count_mutex);
  pthread_mutex_unlock(&count_mutex);
  pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  int i, rc; 
  FILE *infile;
  int size=1024;
  char *buf,*ptr;

  long t1=1, t2=2, t3=3;
  pthread_t threads[3];
  pthread_attr_t attr;

  /* Initialize objects */
  pthread_mutex_init(&count_mutex, NULL);
  pthread_cond_init (&count_threshold_cv, NULL);

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

  /* Add in threads for passing off main input to queue runners */
  /* (threads and splitting file to them for ignoring them by type) */
  /* After that, add standard queue handler threads without special logic */
  /* After that, add lend/borrow thread exchanges between queues */
  /* (No thread create/destroy, rather use locked/unlocked states and secondary pools) */
  /* (E.g. each queue has some spare threads, which can be activated by the queue runner) */
  /* (Activating a spare requires setting lock/block on thread from another queue's main pool.) */
  /* (Spare threads must signal queue runner to confirm other queue doesn't need threads yet.) */

  /* No threads yet... commented out the following as placeholders */
  /*
  pthread_create(&threads[0], &attr, foo1, (void *)t1);
  pthread_create(&threads[1], &attr, foo2, (void *)t2);
  pthread_create(&threads[2], &attr, foo2, (void *)t3);

  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  */

cleanup:
  /* close file */
  fclose(infile);
  
  /* free buf */
  free(buf);

  /* standard clean-up stuff */
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&count_mutex);
  pthread_cond_destroy(&count_threshold_cv);
  pthread_exit (NULL);

}

