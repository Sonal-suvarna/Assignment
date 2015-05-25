#include "barber_new.h"

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

void
barber(customer *cust)
{

        printf("The barber is now awake\n");
        printf("To cut hair of the customer %s\n",cust->name);
        while(cust->hc_time!=0)
        {
                printf("One hair cut at %d time\n",cust->hc_time);
                cust->hc_time--;
        }
        printf("Hair cut done!!!\n\n\n");

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
                fprintf (stderr, "Failed creating customer");
                goto out;
        }

        ret = add_customer_to_queue (cust);
        if (ret)
        {
                /* This is the customer trashing case */
                fprintf (stderr, 
                "Failed adding customer to the waiting queue");
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
                goto unlock; 
        }

      /* queue is empty */
        if (queue_tail == NULL)
        {
                queue_tail = cust;
                queue_head = cust;
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

void *
fetch_customer_from_queue (void * unused)
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
                goto unlock;
        }

        /* The last customer in the queue */
        if (queue_tail == queue_head)
        {
                cust = queue_head;
                queue_tail = queue_head = NULL;
        }
        else
        {
               cust = queue_head;
               queue_head = queue_head->prev;
               cust->prev = NULL;
        }
        //printf("The barber is now cutting the hair of the customer named %s",cust->name);
        /* aal izz well */
        cur_queue_len--;
        ret = 0;
        barber(cust);

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

main()
{

        int i;
        pthread_t id0[5];
        pthread_t id1[5];

        for (i=0; i<5; i++ )
        {
                pthread_create(&id0[i],NULL,&customer_thread,NULL);
        }
        for (i=0; i<5; i++ )
        {
                pthread_create(&id1[i],NULL,&fetch_customer_from_queue,NULL);
        }
        for (i=0; i<5; i++ )
        {
                pthread_join(id1[i],NULL);
        }

}
