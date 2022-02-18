#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h> // https://stackoverflow.com/questions/822323/how-to-generate-a-random-int-in-c

#define SIZE 5
#define PRODUCER_COUNT 3
#define CONSUMER_COUNT 3
#define TOTAL_ITEMS 2

sem_t mutex, empty, full;

// Circular Queue implementation in C
// https://www.programiz.com/dsa/circular-queue

int items[SIZE];
int front = -1, rear = -1;

// Check if the queue is full
int isFull() {
  if ((front == rear + 1) || (front == 0 && rear == SIZE - 1)) return 1;
  return 0;
}

// Check if the queue is empty
int isEmpty() {
  if (front == -1) return 1;
  return 0;
}

// Adding an element
void enQueue(int element) {
  if (isFull())
    printf("\n Queue is full!! \n");
  else {
    if (front == -1) front = 0;
    rear = (rear + 1) % SIZE;
    items[rear] = element;
    //printf("\n Inserted -> %d \n", element);
  }
}

// Removing an element
int deQueue() {
  int element;
  if (isEmpty()) {
    printf("\n Queue is empty !! \n");
    return (-1);
  } else {
    element = items[front];
    if (front == rear) {
      front = -1;
      rear = -1;
    } 
    // Q has only one element, so we reset the 
    // queue after dequeing it. ?
    else {
      front = (front + 1) % SIZE;
    }
    // printf("\n Deleted element -> %d \n", element);
    return (element);
  }
}

// Display the queue
void display() {
  int i;
  if (isEmpty())
    printf(" \n Empty Queue\n");
  else {
    printf("\n Front -> %d ", front);
    printf("\n Items -> ");
    for (i = front; i != rear; i = (i + 1) % SIZE) {
      printf("%d ", items[i]);
    }
    printf("%d ", items[i]);
    printf("\n Rear -> %d \n", rear);
  }
}

int testQeue() {
  // Fails because front = -1
  deQueue();

  enQueue(1);
  enQueue(2);
  enQueue(3);
  enQueue(4);
  enQueue(5);

  // Fails to enqueue because front == 0 && rear == SIZE - 1
  enQueue(6);

  display();
  deQueue();

  display();

  enQueue(7);
  display();

  // Fails to enqueue because front == rear + 1
  enQueue(8);
  display();

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
  for(int i = 0; i < TOTAL_ITEMS; i++){
    resource = rand() % 100;
    sem_wait(&empty);
    sem_wait(&mutex);
    enQueue(resource);
    printf("Producer %d inserted item %d in queue.\n", *(int*)arg, resource); // TODO: print Q ?
    sem_post(&mutex);
    sem_post(&full);
  }
}

void* consumerStartRoutine(void *arg){
  int resource;
  for(int i = 0; i < TOTAL_ITEMS; i++){
    sem_wait(&full);
    sem_wait(&mutex);
    resource = deQueue();
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