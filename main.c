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

sem_t requestMutex, requestEmpty, requestFull;
sem_t responseMutex, responseEmpty, responseFull;

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


Response responseQ[SIZE];
int responseQFront = -1;
int responseQRear = -1;

// Check if the queue is full
int responseQIsFull() {
  if ((responseQFront == responseQRear + 1) || (responseQFront == 0 && responseQRear == SIZE - 1)) return 1;
  return 0;
}

// Check if the queue is empty
int responseQIsEmpty() {
  if (responseQFront == -1) return 1;
  return 0;
}

// Adding an element
void responseQInsert(int element) {
  if (responseQIsFull())
    printf("\n Queue is full!! \n");
  else {
    if (responseQFront == -1) responseQFront = 0;
    responseQRear = (responseQRear + 1) % SIZE;
    responseQ[responseQRear] = element;
    //printf("\n Inserted -> %d \n", element);
  }
}

// Removing an element
int responseQDelete() {
  int element;
  if (responseQIsEmpty()) {
    printf("\n Queue is empty !! \n");
    return (-1);
  } else {
    element = responseQ[responseQFront];
    if (responseQFront == responseQRear) {
      responseQFront = -1;
      responseQRear = -1;
    } 
    // Q has only one element, so we reset the 
    // queue after dequeing it. ?
    else {
      responseQFront = (responseQFront + 1) % SIZE;
    }
    // printf("\n Deleted element -> %d \n", element);
    return (element);
  }
}

// responseQDisplay the queue
void responseQDisplay() {
  int i;
  if (responseQIsEmpty())
    printf(" \n Empty Queue\n");
  else {
    printf("\n responseQFront -> %d ", responseQFront);
    printf("\n responseQ -> ");
    for (i = responseQFront; i != responseQRear; i = (i + 1) % SIZE) {
      printf("%d ", responseQ[i]);
    }
    printf("%d ", responseQ[i]);
    printf("\n responseQRear -> %d \n", responseQRear);
  }
}

int testresponseQ() {
  // Fails because responseQFront = -1
  responseQDelete();

  responseQInsert(1);
  responseQInsert(2);
  responseQInsert(3);
  responseQInsert(4);
  responseQInsert(5);

  // Fails to responseQInsert because responseQFront == 0 && responseQRear == SIZE - 1
  responseQInsert(6);

  responseQDisplay();
  responseQDelete();

  responseQDisplay();

  responseQInsert(7);
  responseQDisplay();

  // Fails to responseQInsert because responseQFront == responseQRear + 1
  responseQInsert(8);
  responseQDisplay();

  return 0;
}

int testrequestQ() {
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
  sem_init(&requestMutex, 0, 1);
  sem_init(&requestEmpty, 0, SIZE);
  sem_init(&requestFull, 0, 0);
  sem_init(&responseMutex, 0, 1);
  sem_init(&responseEmpty, 0, SIZE);
  sem_init(&responseFull, 0, 0);
}

void destroySemaphores(){
  sem_destroy(&requestMutex);
  sem_destroy(&requestEmpty);
  sem_destroy(&requestFull);
  sem_destroy(&responseMutex);
  sem_destroy(&responseEmpty);
  sem_destroy(&responseFull);
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
    // testRequestQ();
    // testResponseQ();
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