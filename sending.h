#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/types.h>
#include<malloc.h>
#include<string.h>
#include<assert.h>

#define CUST_NAME_MAX 100 /*Length of the name*/
#define CUST_MAX_HC_PERIOD 10 /* In secs */
#define DEFAULT_MAX_QUEUE_SIZE 5 /*Maximum size of the queue*/

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

/* Indicators: 
 *  Data_found : indicates whether there is data in the queue or not 
 *  glob_awake : indicates if the barber is sleeping or not */
int data_found = 0;
int glob_awake = 0;

int flag=0;
int data,y;
customer *queue_tail = NULL;
customer* queue_head = NULL;


pthread_mutex_t data_found_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t waiting_queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t count_threshold_cv = PTHREAD_COND_INITIALIZER;


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
void*
barber(void *unused);

/*
Input:
        customer *cust: Pointer to the structure
Output:
        int: To assure the successful execution
*/
int
add_customer_to_queue (customer *cust);

/*To get the customer details from the queue*/
customer*
fetch_customer_from_queue();

/*To generate the randon string
Input:
        size_t length: The length of the string to generate
Output:
        char*: The pointer of the character
*/
char*
randstring(size_t length);

void*
service(void * unused);

