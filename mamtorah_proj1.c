/*
* Details:
*   Author: Hemal Ketan Mamtora
*   Affiliation: Master of Computer Science @ Texas A&M University
*   Term: Spring 2022
*   Course: CSCE 611 Operating Systems
*   Project 1: Multiple Producer Consumer Problem
* 
* Instructions to compile on Texas Cyberrange Machine (Ubuntu 20.04) (user@box):
*   Go To folder:
*   $ cd /home/user/Desktop/producer_consumer
*   Compile:
*   $ gcc mamtorah_proj1.c -lpthread -lm -o mamtorah_proj1.exe
*   Run:
*   $ ./mamtorah_proj1.exe
*   Stop:
*   Press ctrl + c (^C), to stop
* 
* Modify Parameters:
*   SIZE: to change the size of the message queues
*   PRODUCER_COUNT: to change the number of producers
*   CONSUMER_COUNT: to change the number of consumers
* 
* Brief Description of the solution:
*   1. correct solution" to the multiple dimensioned, multiple producer/consumer
*      (That is multiple producers > 3 and multiple consumers > 3) problem:
*         
*          I've implemented it using pthreads and semaphores and queues.
*          
*          The consumer
*            Each consumer is made of 2 threads. The consumerRequestThreads
*            create request for the consumer and insert the request message to the request queue
*            The consumerResponseThreads checks the response message queue for acknowledgement and resource.
*          
*          The producer
*            The producer is made of 1 thread. The producerThreads reads the request message from the request message queue.
*            It allocates the resource and puts the resource in the response message queue. (which is checked by the consumer).
*          
*          Correctly implemented the following:
*            Producer:
*              1: if msg from Consumer, place msg on queue w/timestamp
*              2: send ack to Consumer (I.e., I have a resource)
*            Consumer:
*              1: determines resource need.
*              2: receives resource and processes it, return to step 1
*              
*          Professor said: Anything that explicitly is not disallowed is allowed.
*          So I've the following ASSUMPTIONS:
*            1. There is unlimited resource available.
*            2. The resource allocation from producer process happens instantaneously.
*            3. All producers and consumers deal with the same resource type. There is only 1 resource type.
*          
*          How does my solution address the conditions?
*               
*          1. Each message is received by one and only one consumer
*              By using mutual exclusion semaphores: requestMutex, responseMutex,
*              the push operation to the queue and the pop operation from the queue is done by 1 and only 1 producer or consumer at a time. 
*              Due to this, each message in the queue is received by only 1 entity (producer or consumer)
*                 
*          2. Every producer and consumer is treated in a fair manner (no starvation).
*              Every producer and consumer thread run independently without blocking any other thread.
*              The thread has to wait only if the message queue is full or empty depending on the push or pop operation respectively.
*              By not relying on CPU to treat it in fair manner, I've implemented an exponential backoff - like solution.
*                  
*              Suppose Producer 1 is given preference a lot of time. And other producers did not get a chance to produce.
*              This condition is mitigated by the fact that the producer  has to wait(sleep) for 1 millisecond after the first production.
*              Then it sleeps for 2, 4, 8 ... milliseconds. Meanwhile, other producer threads have to wait less 1, 2, 4 ... milliseconds while the first producer is waiting for say 16 seconds.
*              Hence, other producer get a fair chance in producing.
*              I acknowledge that a backoff of 2048 or more milliseconds means that the producer 1 itself is starving.
*              Hence, I've put a modulo 10 operator, so max wait would be 512 milliseconds.
*              This parameter can be modified with respect to the producer-consumer counts.
*              The above logic is implemented for consumer threads as well.
*          
*          3. No centralized task manager is allowed. That is, you cannot simply queue all requests from each producer and then sequentially assign them to consumers. 
*              There is no use of centralized  task manager. I use queue, but the queue is not managed centrally.
*              Any producer thread can insert into the queue at any time. Similarly any consumer thread can read and write to the thread at any time.
*          
*          4. No common shared memory
*              There is no common OS managed shared memory from where the data is being read.
*              The threads share a common data structure, the request and response message queues.
*              But, as discussed in class, it is  allowed to use common data structures as long as I'm implementing message passing.
*           
*           
* Example 1: (PRODUCERS 5, CONSUMERS 5, BUFFER SIZE 4)
*   Consumer 2 inserted request with timestamp 1645593757065618 microseconds in request queue.
*   Consumer 4 inserted request with timestamp 1645593757065611 microseconds in request queue.
*   Producer 1 inserted ack and resource 86 in response queue.
*   Producer 0 inserted ack and resource 77 in response queue.
*   Consumer 1 consumed item 86 from response queue
*   Producer 3 acknowledged request with timestamp 1645593757064997 microseconds from request queue
*   Consumer 0 consumed item 77 from response queue
*   Producer 4 inserted ack and resource 83 in response queue.
*   Producer 2 acknowledged request with timestamp 1645593757065007 microseconds from request queue
*   Consumer 3 inserted request with timestamp 1645593757065623 microseconds in request queue.
*   Consumer 0 inserted request with timestamp 1645593757065462 microseconds in request queue.
*   Consumer 1 consumed item 83 from response queue
*   Producer 1 acknowledged request with timestamp 1645593757065618 microseconds from request queue
*   Consumer 2 inserted request with timestamp 1645593757065727 microseconds in request queue.
*   Producer 0 acknowledged request with timestamp 1645593757065611 microseconds from request queue
*   Consumer 4 inserted request with timestamp 1645593757065741 microseconds in request queue.
*   Producer 3 inserted ack and resource 15 in response queue.
*   Producer 4 acknowledged request with timestamp 1645593757065623 microseconds from request queue
*   Producer 2 inserted ack and resource 93 in response queue.
*   Consumer 0 inserted request with timestamp 1645593757065863 microseconds in request queue.
*   Consumer 1 consumed item 15 from response queue
*   Consumer 2 consumed item 93 from response queue
*   Producer 1 inserted ack and resource 35 in response queue.
*   Consumer 3 consumed item 35 from response queue
*   Producer 0 inserted ack and resource 86 in response queue.
*
*   Comment:
*   In above example,
*   There is intermingling of occurrence of producers and consumers which
*   suggest none of the threads is starving.
*   The required format is also followed:
*   Request from consumer -> ack from producer -> resource from producer.
* 
* Example 2: (PRODUCERS 4, CONSUMERS 4, BUFFER SIZE 3)
*   Consumer 0 inserted request with timestamp 1645594105562886 microseconds in request queue.
*   Consumer 1 inserted request with timestamp 1645594105562940 microseconds in request queue.
*   Producer 2 acknowledged request with timestamp 1645594105562886 microseconds from request queue
*   Producer 3 acknowledged request with timestamp 1645594105562940 microseconds from request queue
*   Producer 3 inserted ack and resource 86 in response queue.
*   Consumer 0 consumed item 86 from response queue
*   Producer 2 inserted ack and resource 83 in response queue.
*   Consumer 1 inserted request with timestamp 1645594105563123 microseconds in request queue.
*   Consumer 0 inserted request with timestamp 1645594105563114 microseconds in request queue.
*   Consumer 1 consumed item 83 from response queue
*   Consumer 2 inserted request with timestamp 1645594105563087 microseconds in request queue.
*   Producer 1 acknowledged request with timestamp 1645594105563123 microseconds from request queue
*   Producer 0 acknowledged request with timestamp 1645594105563114 microseconds from request queue
*   Producer 1 inserted ack and resource 77 in response queue.
*   Consumer 3 consumed item 77 from response queue
*   Producer 0 inserted ack and resource 15 in response queue.
*   Consumer 2 consumed item 15 from response queue
*/

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

// infinite loop: press ctrl + C to stop
// https://stackoverflow.com/questions/4800404/how-to-avoid-memory-leak-when-user-press-ctrlc-under-linux

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
  return 0;
}

// https://www.ibm.com/docs/en/i/7.4?topic=ssw_ibm_i_74/apis/users_14.htm
// https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_create.html
void* producerStartRoutine(void *arg){
  useconds_t sleep_time;
  Request request;
  Response response;
  int i = 0;
  while(1){

    // TODO: explain in comments, how sleeping implements aging and avoids starvation
    sleep_time = (useconds_t) pow(2, i % 10); // + (useconds_t) (rand() % 500);
    i += 1;
    // printf("Producer %d sleeps for %u microseconds\n", *(int*)arg, sleep_time);
    usleep(sleep_time);

    sem_wait(&requestFull); // wait if there are no requests
    sem_wait(&requestMutex);
    request = requestQDelete();
    printf("Producer %d acknowledged request with timestamp %ld microseconds from request queue\n", *(int*)arg, request.timestamp); // TODO: print Q ?
    sem_post(&requestMutex);
    sem_post(&requestEmpty);

    // NOTE: I assume that
    // 1. There are unlimited resources
    // 2. The access to resource is instantaneous
    // Hence, as soon as the request arrives to the producer, the producer can serve the request.
    response = (Response){1, (rand() % 100)};// ack, resource

    // TODO: explain in comments, how sleeping implements aging and avoids starvation
    sleep_time = (useconds_t) pow(2, i % 10);//+ (useconds_t) (rand() % 500);
    // printf("Producer %d sleeps for %u microseconds\n", *(int*)arg, sleep_time);
    usleep(sleep_time);

    sem_wait(&responseEmpty); // wait till empty slots become available
    sem_wait(&responseMutex);
    responseQInsert(response);
    printf("Producer %d inserted ack and resource %d in response queue.\n", *(int*)arg, response.resource);
    sem_post(&responseMutex);
    sem_post(&responseFull);
  }
}

void* consumerRequestStartRoutine(void *arg){
  useconds_t sleep_time;
  struct timeval tv;
  long timestamp;
  Request request;
  int i = 0;
  // Consumer requests for a resource, puts it into requestQ
  while(1){

    // create a new new request
    gettimeofday(&tv,NULL);

    // timestamp in microseconds
    // https://stackoverflow.com/questions/5833094/get-a-timestamp-in-c-in-microseconds
    timestamp = tv.tv_sec*(long)1000000+tv.tv_usec;
    request = (Request){timestamp};

    // TODO: explain in comments, how sleeping implements aging and avoids starvation
    sleep_time = (useconds_t) pow(2, i % 10);//+ (useconds_t) (rand() % 500);
    i += 1;
    // printf("Consumer %d sleeps for %u microseconds\n", *(int*)arg, sleep_time);
    usleep(sleep_time);

    sem_wait(&requestEmpty); // wait till an empty slot becomes available
    sem_wait(&requestMutex); // mutual exclusion
    requestQInsert(request);
    printf("Consumer %d inserted request with timestamp %ld microseconds in request queue.\n", *(int*)arg, request.timestamp);
    sem_post(&requestMutex);
    sem_post(&requestFull); // wake producer that there is some request to be processed. 
  }
}

void* consumerResponseStartRoutine(void *arg){
  useconds_t sleep_time;
  Response response;
  int i = 0;

  // Consumer requests for a resource, puts it into requestQ
  while(1){

    // TODO: explain in comments, how sleeping implements aging and avoids starvation
    sleep_time = (useconds_t) pow(2, i % 10); // + (useconds_t) (rand() % 500);
    i += 1;
    // printf("Consumer %d sleeps for %u microseconds\n", *(int*)arg, sleep_time);
    usleep(sleep_time);

    sem_wait(&responseFull); // wait if there are no resources/acknowledgements
    sem_wait(&responseMutex);
    response = responseQDelete();
    printf("Consumer %d consumed item %d from response queue\n", *(int*)arg, response.resource); // TODO: print Q ?
    sem_post(&responseMutex);
    sem_post(&responseEmpty);
  }
}

// https://shivammitra.com/c/producer-consumer-problem-in-c/#
int main(){
    // testSemaphores();
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