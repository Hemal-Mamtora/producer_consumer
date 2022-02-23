#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h> // https://stackoverflow.com/questions/822323/how-to-generate-a-random-int-in-c
#include <unistd.h> // https://pubs.opengroup.org/onlinepubs/009696799/functions/sleep.html
#include <math.h>
#include <sys/time.h>

#define SIZE 4
#define PRODUCER_COUNT 5
#define CONSUMER_COUNT 5
#define TOTAL_requestQ 5

sem_t requestMutex, requestEmpty, requestFull;
sem_t responseMutex, responseEmpty, responseFull;

typedef struct Request{
  long timestamp;
}Request;

Request nullRequest = {-1};

typedef struct Response{
  int ack; // boolean to store acknowledgement
  int resource;
}Response;

Response nullResponse = {-1, -1};

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
void requestQInsert(Request element) {
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
Request requestQDelete() {
  Request element;
  if (requestQIsEmpty()) {
    printf("\n Queue is empty !! \n");
    return nullRequest;
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
      printf("%ld ", requestQ[i].timestamp);
    }
    printf("%ld ", requestQ[i].timestamp);
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
void responseQInsert(Response element) {
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
Response responseQDelete() {
  Response element;
  if (responseQIsEmpty()) {
    printf("\n Queue is empty !! \n");
    return nullResponse;
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
      printf("%d ", responseQ[i].resource);
    }
    printf("%d ", responseQ[i].resource);
    printf("\n responseQRear -> %d \n", responseQRear);
  }
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
  Request request;
  Response response;

  for(int i = 0; i < TOTAL_requestQ; i++){

    // TODO: explain in comments, how sleeping implements aging and avoids starvation
    sleep_time = (useconds_t) pow(2, i % 10); // + (useconds_t) (rand() % 500);
    printf("Producer %d sleeps for %u microseconds\n", *(int*)arg, sleep_time);
    usleep(sleep_time);

    sem_wait(&requestFull); // wait if there are no requests
    sem_wait(&requestMutex);
    request = requestQDelete();
    printf("Producer %d acknowledged request with timestamp %ld from queue\n", *(int*)arg, request.timestamp); // TODO: print Q ?
    sem_post(&requestMutex);
    sem_post(&requestEmpty);

    // NOTE: I assume that
    // 1. There are unlimited resources
    // 2. The access to resource is instantaneous
    // Hence, as soon as the request arrives to the producer, the producer can serve the request.
    response = (Response){1, (rand() % 100)};// ack, resource

    // TODO: explain in comments, how sleeping implements aging and avoids starvation
    sleep_time = (useconds_t) pow(2, i % 10);//+ (useconds_t) (rand() % 500);
    printf("Producer %d sleeps for %u microseconds\n", *(int*)arg, sleep_time);
    usleep(sleep_time);

    sem_wait(&responseEmpty); // wait till empty slots become available
    sem_wait(&responseMutex);
    responseQInsert(response);
    printf("Producer %d inserted ack and resource %d in queue.\n", *(int*)arg, response.resource);
    sem_post(&responseMutex);
    sem_post(&responseFull);
  }
}

void* consumerRequestStartRoutine(void *arg){
  useconds_t sleep_time;
  struct timeval tv;
  long timestamp;
  Request request;

  // Consumer requests for a resource, puts it into requestQ
  for(int i = 0; i < TOTAL_requestQ; i++){

    // create a new new request
    gettimeofday(&tv,NULL);

    // timestamp in microseconds
    // https://stackoverflow.com/questions/5833094/get-a-timestamp-in-c-in-microseconds
    timestamp = tv.tv_sec*(long)1000000+tv.tv_usec;
    request = (Request){timestamp};

    // TODO: explain in comments, how sleeping implements aging and avoids starvation
    sleep_time = (useconds_t) pow(2, i % 10);//+ (useconds_t) (rand() % 500);
    printf("Consumer %d sleeps for %u microseconds\n", *(int*)arg, sleep_time);
    usleep(sleep_time);

    sem_wait(&requestEmpty); // wait till an empty slot becomes available
    sem_wait(&requestMutex); // mutual exclusion
    requestQInsert(request);
    printf("Consumer %d inserted request with timestamp %ld in queue.\n", *(int*)arg, request.timestamp);
    sem_post(&requestMutex);
    sem_post(&requestFull); // wake producer that there is some request to be processed. 
  }
}

void* consumerResponseStartRoutine(void *arg){
  useconds_t sleep_time;
  Response response;

  // Consumer requests for a resource, puts it into requestQ
  for(int i = 0; i < TOTAL_requestQ; i++){

    // TODO: explain in comments, how sleeping implements aging and avoids starvation
    sleep_time = (useconds_t) pow(2, i % 10); // + (useconds_t) (rand() % 500);
    printf("Consumer %d sleeps for %u microseconds\n", *(int*)arg, sleep_time);
    usleep(sleep_time);

    sem_wait(&responseFull); // wait if there are no resources/acknowledgements
    sem_wait(&responseMutex);
    response = responseQDelete();
    printf("Consumer %d consumed item %d from queue\n", *(int*)arg, response.resource); // TODO: print Q ?
    sem_post(&responseMutex);
    sem_post(&responseEmpty);
  }
}

// https://shivammitra.com/c/producer-consumer-problem-in-c/#
int main(){
    // testSemaphores();
    time_t t = -1;

    pthread_t producerThreads[PRODUCER_COUNT], consumerRequestThreads[CONSUMER_COUNT], consumerResponseThreads[CONSUMER_COUNT];
    initializeSemaphores();

    int producerThreadIndices[PRODUCER_COUNT];
    for(int i = 0; i < PRODUCER_COUNT; i++){
      producerThreadIndices[i] = i;
      pthread_create(&producerThreads[i], NULL, (void *)producerStartRoutine, (void *)&producerThreadIndices[i]);
    }

    int consumerRequestThreadIndices[CONSUMER_COUNT];
    int consumerResponseThreadIndices[CONSUMER_COUNT];
    for(int i = 0; i < CONSUMER_COUNT; i++){
      consumerRequestThreadIndices[i] = i;
      consumerResponseThreadIndices[i] = i;
      pthread_create(&consumerRequestThreads[i], NULL, (void *)consumerRequestStartRoutine, (void *)&consumerRequestThreadIndices[i]);
      pthread_create(&consumerResponseThreads[i], NULL, (void *)consumerResponseStartRoutine, (void *)&consumerResponseThreadIndices[i]);
    }

    for(int i = 0; i < PRODUCER_COUNT; i++) {
        pthread_join(producerThreads[i], NULL);
    }

    for(int i = 0; i < CONSUMER_COUNT; i++) {
        pthread_join(consumerRequestThreads[i], NULL);
        pthread_join(consumerResponseThreads[i], NULL);
    }

    destroySemaphores();

    return 0;
}