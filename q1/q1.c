#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

typedef struct thread_details
{
    int t,w,p,id;
} td;

int numFree = 0;
sem_t sem;
int start = 0;
int count = 0;
int waiting = 0;
int startTimes[1000]; // Maximum 1000 students can be simulate for

void* wash(void* arg)
{
    int studentID = ((struct thread_details *)arg)->id;
    int startTime = ((struct thread_details *)arg)->t;
    int runTime = ((struct thread_details *)arg)->w;
    int patience = ((struct thread_details *)arg)->p;

    sleep(startTime);
    int now = time(NULL);
    printf("%d: Student %d arrives\n",now-start,studentID);

    int k=0;
    int val;
    sem_getvalue(&sem,&val);
    if(val>0)
    {
        sem_wait(&sem);
    }
    else
    {
        int temp = waiting;
        while(!val)
        {
            sleep(1);
            k++;
            waiting++;
            sem_getvalue(&sem,&val);
            if(k>patience)
            {
                now=time(NULL);
                waiting--;
                printf("%d: \033[0;31m\x1B[1mStudent %d leaves without washing\033[0m\x1B[0m\n",now-start,studentID);
                count++;
                return NULL;
            }
        }
        if(temp!=waiting)
        {
            waiting--;
        }
        sem_wait(&sem);
    }
    numFree--;
    now = time(NULL);
    printf("%d: \033[0;32m\x1B[1mStudent %d starts washing\033[0m\x1B[0m\n",now-start,studentID);
    sleep(runTime);
    sem_post(&sem);
    numFree++;
    now = time(NULL);
    printf("%d: \033[0;33m\x1B[1mStudent %d leaves after washing\033[0m\x1B[0m\n",now-start,studentID);

    return NULL;
}

int main()
{
    int n,m;
    scanf("%d%d",&n,&m);

    int t[n], w[n], p[n];
    numFree = m;

    pthread_t th[n];

    sem_init(&sem, 0, m);

    for(int i=0; i<n; i++)
    {
        scanf("%d%d%d",&t[i],&w[i],&p[i]);
    }
    
    start = time(NULL);

    for(int i=0; i<n; i++)
    {
        td *thread_input = (td *)(malloc(sizeof(td)));
        thread_input->id = i+1;
        thread_input->t = t[i];
        thread_input->w = w[i];
        thread_input->p = p[i];
        pthread_t a;
        pthread_create(&a, NULL, wash, (void *)(thread_input));
        th[i] = a;
    }

    for(int i=0; i<n; i++)
    {
        pthread_join(th[i],NULL);
    }

    sem_destroy(&sem);
    printf("%d\n%d\n",count,waiting+1);
    if((count/n)<0.25)
    {
        printf("No\n");
    }
    else
    {
        printf("Yes\n");
    }

    return 0;
}
