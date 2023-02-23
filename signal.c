/**
 *  Signal example
 *  Based on Operating Systems: Three Easy Pieces by R. Arpaci-Dusseau and A. Arpaci-Dusseau
 *
 *  This example uses locks and a condition to synchronize a child thread and
 *  the parent.  The child generates matricies and the parent computes the
 *  average when the matrix is ready for processing.
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include "matrix.h"
#include "counter.h"
#include "prodcons.h"
#include "pcmatrix.h"


//#define NUMTHREADS 8
#define OUTPUT 0
#define NUMBER_OF_MATRICES 10000

Matrix ** bigmatrix;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

int fill_ptr = 0;
int use_ptr = 0;
int count = 0;
int matrix_produced = 0;
int matrix_consumed = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *producer(void *arg)
{
    int i;
    Matrix* matrix = malloc(sizeof(Matrix *));
    while (matrix_produced < NUMBER_OF_MATRICES){
      Pthread_mutex_lock(&mutex);
      while (count == MAX)
        Pthread_cond_wait(&empty, &mutex);
      put(matrix);
      Pthread_cond_signal(&fill);
      Pthread_mutex_unlock(&mutex);
    }
    }

    for (i=0; i<*loops; i++)
    {
      bm = AllocMatrix(r,c);
      GenMatrix(bm, r, c);

      pthread_mutex_lock(&mutex);
      bigmatrix = bm;
      // signal parent
      count = count + 1;

      pthread_cond_signal(&cond);
      while (count == MAX)
        pthread_cond_wait(&cond, &mutex);
      pthread_mutex_unlock(&mutex);
    }

  return NULL;
}

int main (int argc, char * argv[])
{
  pthread_t p1;
  pthread_t p2;
  bigmatrix = (Matrix **) malloc(sizeof(Matrix *) * BOUNDED_BUFFER_SIZE);

  int loops = NUMBER_OF_MATRICES;
  int i;

  pthread_create(&p1, NULL, producer, NULL);  // CREATE MATRIX PRODUCER THREAD
  pthread_create(&p2, NULL, consumer, NULL);  // CREATE MATRIX CONSUMER THREAD

  pthread_join(p1, NULL);
  pthread_join(p2, NULL);
  return 0;
}

void *producer(void *arg) {
  int i;
  for (i = 0; i < NUMBER_OF_MATRICES; i++) {
  Pthread_mutex_lock(&mutex);
  while (count == MAX)
    Pthread_cond_wait(&empty, &mutex);
  put(i);
  Pthread_cond_signal(&fill);
  Pthread_mutex_unlock(&mutex);
  }
}

void *consumer(void *arg) {
  int loops = local_counter;
  int i;
  for(i=0;i<loops;i++)
  
  {
    pthread_mutex_lock(&mutex);
    while (count==0)
      pthread_cond_wait(&cond, &mutex);
    int ** bm = *&bigmatrix;
    count = count - 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    int avgele = AvgElement(bm, r, c);
    printf("Matrix #%d: avg element value=%d\n",i,avgele);
    FreeMatrix(bm, r, c);

  }
}
