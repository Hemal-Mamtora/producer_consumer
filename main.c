#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h> // https://stackoverflow.com/questions/822323/how-to-generate-a-random-int-in-c
#include <unistd.h> // https://pubs.opengroup.org/onlinepubs/009696799/functions/sleep.html
#include <math.h>
#include <time.h> // https://stackoverflow.com/questions/11765301/how-do-i-get-the-unix-timestamp-in-c-as-an-int

#define SIZE 10
#define PRODUCER_COUNT 10
#define CONSUMER_COUNT 10
#define TOTAL_requestQ 20

sem_t mutex, empty, full;

typedef struct Request{
  time_t time;
}Request;

typedef struct Response{
  int ack; // boolean to store acknowledgement
  int resource;
}Response;

// Circular Queue implementation in C
// https://www.programiz.com/dsa/circular-queue
Request requestQ[SIZE];
int requestQFront = -1;
int requestQRear = -1;

// Check if the queue is full
int requestQIsFull() {
  if ((requestQFront == requestQRear + 1) || (requestQFront == 0 && requestQRear == SIZE - 1)) return 1;
  return 0;
}

// Check if the queue is empty
int requestQIsEmpty() {
  if (requestQFront == -1) return 1;
  return 0;
}

// Adding an element
void requestQInsert(int element) {
  if (requestQIsFull())
    printf("\n Queue is full!! \n");
  else {
    if (requestQFront == -1) requestQFront = 0;
    requestQRear = (requestQRear + 1) % SIZE;
    requestQ[requestQRear] = element;
    //printf("\n Inserted -> %d \n", element);
  }
}

// Removing an element
int requestQDelete() {
  int element;
  if (requestQIsEmpty()) {
    printf("\n Queue is empty !! \n");
    return (-1);
  } else {
    element = requestQ[requestQFront];
    if (requestQFront == requestQRear) {
      requestQFront = -1;
      requestQRear = -1;
    } 
    // Q has only one element, so we reset the 
    // queue after dequeing it. ?
    else {
      requestQFront = (requestQFront + 1) % SIZE;
    }
    // printf("\n Deleted element -> %d \n", element);
    return (element);
  }
}

// requestQDisplay the queue
void requestQDisplay() {
  int i;
  if (requestQIsEmpty())
    printf(" \n Empty Queue\n");
  else {
    printf("\n requestQFront -> %d ", requestQFront);
    printf("\n requestQ -> ");
    for (i = requestQFront; i != requestQRear; i = (i + 1) % SIZE) {
      printf("%d ", requestQ[i]);
    }
    printf("%d ", requestQ[i]);
    printf("\n requestQRear -> %d \n", requestQRear);
  }
}

int testQeue() {
  // Fails because requestQFront = -1
  requestQDelete();

  requestQInsert(1);
  requestQInsert(2);
  requestQInsert(3);
  requestQInsert(4);
  requestQInsert(5);

  // Fails to requestQInsert because requestQFront == 0 && requestQRear == SIZE - 1
  requestQInsert(6);

  requestQDisplay();
  requestQDelete();

  requestQDisplay();

  requestQInsert(7);
  requestQDisplay();

  // Fails to requestQInsert because requestQFront == requestQRear + 1
  requestQInsert(8);
  requestQDisplay();

  return 0;
}

void initializeSemaphores(){
  sem_init(&mutex, 0, 1);
  sem_init(&empty, 0, SIZE);
  sem_init(&full, 0, 0);
}

void destroySemaphores(){
  sem_destroy(&mutex);
  sem_destroy(&empty);
  sem_destroy(&full);
}

int testSemaphores(){
  initializeSemaphores();
  destroySemaphores();
}

// https://www.ibm.com/docs/en/i/7.4?topic=ssw_ibm_i_74/apis/users_14.htm
// https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_create.html
void* producerStartRoutine(void *arg){
  int resource;
  useconds_t sleep_time;
  for(int i = 0; i < TOTAL_requestQ; i++){
    resource = rand() % 100;

    // TODO: explain in comments, how sleeping implements aging and avoids starvation
    sleep_time = (useconds_t) pow(2, i % 10);//+ (useconds_t) (rand() % 500);
    printf("Producer %d sleeps for %u microseconds\n", *(int*)arg, sleep_time);
    usleep(sleep_time);

    sem_wait(&empty);
    sem_wait(&mutex);
    requestQInsert(resource);
    printf("Producer %d inserted item %d in queue.\n", *(int*)arg, resource); // TODO: print Q ?
    sem_post(&mutex);
    sem_post(&full);
  }
}

void* consumerStartRoutine(void *arg){
  int resource;
  useconds_t sleep_time;

  for(int i = 0; i < TOTAL_requestQ; i++){

    // TODO: explain in comments, how sleeping implements aging and avoids starvation
    sleep_time = (useconds_t) pow(2, i % 10); // + (useconds_t) (rand() % 500);
    printf("Consumer %d sleeps for %u microseconds\n", *(int*)arg, sleep_time);
    usleep(sleep_time);

    sem_wait(&full);
    sem_wait(&mutex);
    resource = requestQDelete();
    printf("Consumer %d consumed item %d from queue\n", *(int*)arg, resource); // TODO: print Q ?
    sem_post(&mutex);
    sem_post(&empty);
  }
}

// https://shivammitra.com/c/producer-consumer-problem-in-c/#
int main(){
    // testQeue();
    // testSemaphores();
    pthread_t producerThreads[PRODUCER_COUNT], consumerThreads[CONSUMER_COUNT];
    initializeSemaphores();

    int producerIndices[PRODUCER_COUNT];
    for(int i = 0; i < PRODUCER_COUNT; i++){
      producerIndices[i] = i;
      pthread_create(&producerThreads[i], NULL, (void *)producerStartRoutine, (void *)&producerIndices[i]);
    }

    int consumerIndices[CONSUMER_COUNT];
    for(int i = 0; i < CONSUMER_COUNT; i++){
      consumerIndices[i] = i;
      pthread_create(&consumerThreads[i], NULL, (void *)consumerStartRoutine, (void *)&consumerIndices[i]);
    }

    for(int i = 0; i < PRODUCER_COUNT; i++) {
        pthread_join(producerThreads[i], NULL);
    }

    for(int i = 0; i < CONSUMER_COUNT; i++) {
        pthread_join(consumerThreads[i], NULL);
    }

    destroySemaphores();

    return 0;
}