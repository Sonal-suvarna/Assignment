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

        printf("Custthread\n");
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
printf("Adding\n");
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
        printf("Entring add\n");
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
                data_found = 1;
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
        printf("The name is %s",cust->name);

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
        printf("Entrng fetch\n");
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
                data_found = 0;
        }
        else
        {
               cust = queue_head;
               queue_head = queue_head->prev;
               cust->prev = NULL;
        }
        printf("In fetch %s",cust->name);

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
        while(1)
        {
                pthread_mutex_lock(&count_mutex);
                while(glob_awake==0&&data_found==1)
                {
                        glob_awake=1;
                        printf("Signal\n");
                        pthread_cond_signal(&count_threshold_cv);
                }
                pthread_mutex_unlock(&count_mutex);
        }
}

void*
barber(void *unused)
{

        customer *cust;
        pthread_t fid;
        while(1)
        {
                pthread_mutex_lock(&count_mutex);
                while(glob_awake==0)
                {
                printf("Sleeping\n");
                pthread_cond_wait(&count_threshold_cv,&count_mutex);
                }
                cust=fetch_customer_from_queue();
                printf("Signld\n");
                if(cust!=NULL)
                printf("The barber now wakes up nd starts the hair cut of %s \n",cust->name);
                else
                {
                        glob_awake=0;
                        break;
                }
                pthread_mutex_unlock(&count_mutex);
        }

}

main()
{

        int i;
        pthread_t id0;
        pthread_t id1;
        pthread_t id2[100];
        pthread_create(&id0,NULL,&barber,NULL);
        pthread_create(&id1,NULL,&service,NULL);

        for (i=0; i<3; i++ )
        {
                pthread_create(&id2[i],NULL,&customer_thread,NULL);
        }
        for (i=0; i<3; i++ )
        {
                pthread_join(id2[i],NULL);
        }
        printf("break time\n");
        pthread_join(id1,NULL);
        pthread_join(id0,NULL);

}