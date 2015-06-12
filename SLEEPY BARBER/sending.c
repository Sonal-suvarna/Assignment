#include "sending.h"

void
finit_customer(customer **cust)
{

        if(!cust)
                return;
        if(!*cust)
                return;
        if((*cust)->name)
                free((*cust)->name);
        free(*cust);
        *cust=NULL;

}

customer*
init_customer(const char *name,period_t period)
{

        customer *cust=NULL;
        int ret=-1;
        
        assert(name);
        assert(period!=0);
        cust=calloc(1,sizeof(customer));
        if(cust==NULL)
        {
                fprintf(stderr,"failed creating customer\n");
                goto out;
        }
        ret=asprintf(&cust->name,"%s",name);
        if(ret==-1)
        {
                fprintf(stderr,"failed to initializr=e customer name %s",name);
                goto out;
        }
        cust->hc_time=period;
        ret=0;
        out:
        if(ret)
        {
                finit_customer(&cust);
        }
        return cust;

}


void *
customer_thread (void * unused)
{

        customer *cust = NULL;
        int ret               = -1;
        char customer_name[CUST_NAME_MAX];
        size_t length=10;
        period_t hc_period = 0;

        /* generate random customer name */
        strcpy(customer_name,randstring(length));
        /* Range from 1 to CUST_MAX_HC_PERIOD */
        hc_period = (rand() % CUST_MAX_HC_PERIOD) + 1;
        cust = init_customer (customer_name, hc_period);
        if (!cust) 
        {
                fprintf (stderr, "Failed creating customer\n");
                goto out;
        }
        ret = add_customer_to_queue (cust);
        if (ret)
        {
                /* This is the customer trashing case */
                fprintf (stderr, 
                "Failed adding customer to the waiting queue\n");
        }
out :
         return NULL;

}

char *
randstring(size_t length)
{

        int n;
        static char charset[] = "abcdefghijklmnopqrstuvwxyz"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        char *randomString = NULL;

        if (length)
        {
                randomString = malloc(sizeof(char) * (length +1));
                if (randomString)
                {            
                        for (n = 0;n < length;n++)
                        {            
                                int key = rand() %
                                        (int)(sizeof(charset) -1);
                                randomString[n] = charset[key];
                        }
                        randomString[length] = '\0';
                }
        }
        return randomString;

}


int
add_customer_to_queue (customer *cust)
{
        int ret = -1;
        assert (cust);

      /* Acquire the queue lock */
        ret = pthread_mutex_lock (&waiting_queue_lock);
        if (ret)
        {
                fprintf (stderr, "Cannot obtain queue lock \n");
                goto out;
        }

        if (cur_queue_len == max_queue_size)
        {
		printf ("Customer : %s is getting trashed! :(\n", cust->name);
		ret = -1;
                goto unlock; 
        }
      /* queue is empty */
        if (queue_tail == NULL)
        {
                queue_tail = cust;
                queue_head = cust;

		/* Before changing the value of a globle variable take a lock*/
		pthread_mutex_lock (&data_found_lock);
                data_found = 1;
		pthread_mutex_unlock (&data_found_lock);

        }
        else
        {
                cust->next = queue_tail;
                queue_tail->prev = cust;
                queue_tail = cust;
        }
        
      /* aal izz well :) */
        cur_queue_len++;
        ret = 0;
	printf ("Customer : %s in the waiting queue! :) \n", cust->name);

unlock:

        /* Release the queue lock */
        ret = pthread_mutex_unlock (&waiting_queue_lock);
        if (ret)
        {
                 fprintf(stderr, "Cannot unlock mutex lock\n");
                 abort ();
        }

out:
        return ret;

}

customer*
fetch_customer_from_queue ()
{
        int ret = -1;
        customer *cust = NULL;

        /* Acquire the queue lock */
        ret = pthread_mutex_lock (&waiting_queue_lock);
        if (ret)
        {
                fprintf (stderr, "Cannot obtain queue lock \n");
                goto out;
        }

        /* if queue is empty */
        if (cur_queue_len == 0)
        {
		ret = -1;
                goto unlock;
        }

        /* The last customer in the queue */
        if (queue_tail == queue_head)
        {
                cust = queue_head;
                queue_tail = queue_head = NULL;
 

                /* Before changing the value of a globle variable take a lock*/
                pthread_mutex_lock (&data_found_lock);
                data_found = 0;
                pthread_mutex_unlock (&data_found_lock);

        }
        else
        {
               cust = queue_head;
               queue_head = queue_head->prev;
               cust->prev = NULL;
        }

        /* aal izz well */
        cur_queue_len--;
        ret = 0;

unlock:
        /* Release the queue lock */
        ret = pthread_mutex_unlock (&waiting_queue_lock);
        if (ret)
        {
                 fprintf(stderr, "Cannot unlock mutex lock\n");
                 abort ();
        }

out: 

        return cust;

}

void*
service(void *unused)
{
       int data_found_flag = 0;
       while(1)
        {       
		
                pthread_mutex_lock(&count_mutex);

                /* Before reading the value of a globle variable take a lock*/
                pthread_mutex_lock (&data_found_lock);
                data_found_flag = data_found;
                pthread_mutex_unlock (&data_found_lock);


                /* If data is available in the queue and barber is sleeping signal the barber thread */
                if (data_found_flag == 1 && glob_awake == 0)
                {
                        pthread_cond_signal(&count_threshold_cv);

                }

                pthread_mutex_unlock(&count_mutex);
 
       }
}

void*
barber(void *unused)
{

        customer *cust;
        while(1)
        {
                pthread_mutex_lock(&count_mutex);
                printf("Barber is Sleeping\n");
		glob_awake = 0;
                pthread_cond_wait(&count_threshold_cv,&count_mutex);
                printf("Barber is waked !!\n");

		// Indicate working
                glob_awake = 1;

                cust = fetch_customer_from_queue();
                
                if (cust!=NULL) {

               		printf("Barber is working on %s for %ld secs \n",cust->name, cust->hc_time);
			sleep (cust->hc_time);               
                } else {

		       printf ("NULL Customer!");	    

		}
                pthread_mutex_unlock(&count_mutex);
        }

}

int main()
{

        int i;
        pthread_t barber_thread;
        pthread_t service_thread;
        pthread_t customer_threads[11];

        pthread_create(&barber_thread, NULL, &barber,NULL);
        pthread_create(&service_thread, NULL, &service,NULL);

        for (i=0; i<10; i++ )
        {
                pthread_create (&customer_threads[i], NULL, &customer_thread, NULL);
        }

        pthread_join(barber_thread,NULL);
        pthread_join(service_thread,NULL);

        return 0;

}
