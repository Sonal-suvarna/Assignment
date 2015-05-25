#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/types.h>
#include<malloc.h>
#include<string.h>
#include<assert.h>

#define CUST_NAME_MAX 100 /*Length of the name*/
#define CUST_MAX_HC_PERIOD 60 /* In sec i.e 1 min */
#define DEFAULT_MAX_QUEUE_SIZE 10 /*Maximum size of the queue*/

unsigned long max_queue_size = DEFAULT_MAX_QUEUE_SIZE;

typedef unsigned long period_t;

/*Structure to define the customer*/
struct customer {
        char *name;
        period_t hc_time;
        struct customer *next;
        struct customer *prev;
};

typedef struct customer customer;
unsigned long cur_queue_len = 0;
customer *queue_tail = NULL;
customer* queue_head = NULL;
pthread_mutex_t mutex;
pthread_mutex_t waiting_queue_lock;

/*To free the space if some error with the initialization
Input:
        customer **cust: The customer structure
Output:
        void
*/
void
finit_customer(customer **cust);

/*To initialize the customer
Input:
        const char *name: The name
        period_t period: The time duration for hair cut
Output:
        customer*: Pointer to the customer to the fuction calling it
*/
customer*
init_customer(const char *name,period_t period);

/*To generate the randon string and number*/
void *
customer_thread (void * unused);

/*Few printing statement
Input:
        customer *cust: Pointer to the customer structure
Output:
        void
*/
void
barber(customer *cust);

/*
Input:
        customer *cust: Pointer to the structure
Output:
        int: To assure the successful execution
*/
int
add_customer_to_queue (customer *cust);

/*To get the customer details from the queue*/
void *
fetch_customer_from_queue (void * unused);

/*To generate the randon string
Input:
        size_t length: The length of the string to generate
Output:
        char*: The pointer of the character
*/
char*
randstring(size_t length);