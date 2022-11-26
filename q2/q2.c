#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

int n, m, i, c, o, k;

int num_chefs, num_chefs_left, num_orders;
int ingredients[1000];

pthread_mutex_t v_num_chefs, v_num_chefs_left, v_num_orders, v_ingredients, v_order;
sem_t ovens, drive_thru;

typedef struct chef
{
    int id;
    int entry_time;
    int exit_time;
} chef;

typedef struct pizza
{
    int id;
    int prep_time;
    int num_ingred;
    int ingred[1000];
} pizza;

typedef struct customer
{
    int id;
    int entry_time;
    int order_id;
} customer;

typedef struct order
{
    int id;
    int customer_id;
    int num_pizzas;
    int pizzas[50];
    int stage[50]; // 0 - not started, 1 - in preparation, 2 - completed, 3 - ingredient shortage
    int status;    // 0 - not ordered, 1 - ordered, 2 - started processing, 3 - completed, 4 - partially completed
} order;

order orders[1000];
pizza pizzas[1000];

void *chef_func(void *inp)
{

    sleep(10);

    struct timespec ts;

    struct chef *inputs = (struct chef *)inp;
    sleep(inputs->entry_time);
    printf("\033[1;31mChef %d arrives at time %d\n\033[0m", inputs->id, inputs->entry_time);

    clock_gettime(CLOCK_REALTIME, &ts);
    int entry_time_sec = ts.tv_sec;
    int exit_time_sec = entry_time_sec + inputs->exit_time - inputs->entry_time;

    pthread_mutex_lock(&v_num_chefs);
    num_chefs++;
    pthread_mutex_unlock(&v_num_chefs);

    int leave = 0;

    while (1)
    {

        clock_gettime(CLOCK_REALTIME, &ts);

        if (ts.tv_sec >= exit_time_sec)
        {
            leave = 1;
            break;
        }

        for (int j = 1; j <= num_orders; j++)
        {
            clock_gettime(CLOCK_REALTIME, &ts);

            if (ts.tv_sec >= exit_time_sec)
            {
                leave = 1;
                break;
            }

            if (orders[j].status == 0 || orders[j].status == 3 || orders[j].status == 4)
            {
                continue;
            }

            for (int l = 0; l < orders[j].num_pizzas; l++)
            {

                clock_gettime(CLOCK_REALTIME, &ts);

                if (ts.tv_sec >= exit_time_sec)
                {
                    leave = 1;
                    break;
                }

                if ((orders[j].stage[l] == 0) && (ts.tv_sec + pizzas[orders[j].pizzas[l]].prep_time <= exit_time_sec))
                { // pizza prep has not started
                    printf("\033[1;34mPizza %d in order %d assigned to chef %d\n\033[0m", orders[j].pizzas[l], j, inputs->id);

                    pizza p = pizzas[orders[j].pizzas[l]];
                    int flag = 0;

                    pthread_mutex_lock(&v_ingredients);
                    for (int q = 0; q < p.num_ingred; q++)
                    {
                        if (ingredients[p.ingred[q]] == 0)
                        {
                            flag = 1;
                            break;
                        }
                    }
                    pthread_mutex_unlock(&v_ingredients);

                    if (flag == 1)
                    {
                        printf("\033[1;31mChef %d could not prepare pizza %d for order %d due to ingredient shortage\n\033[0m", inputs->id, orders[j].pizzas[l], j);
                        orders[j].stage[l] = 3;
                    }
                    else
                    {

                        printf("\033[1;31mChef %d is preparing pizza %d for order %d\n\033[0m", inputs->id, orders[j].pizzas[l], j);
                        clock_gettime(CLOCK_REALTIME, &ts);
                        ts.tv_sec = exit_time_sec;

                        printf("\033[1;31mChef %d is waiting for oven allocation for pizza %d for order %d\n\033[0m", inputs->id, orders[j].pizzas[l], j);
                        int x = sem_timedwait(&ovens, &ts);
                        if (x == -1)
                        {
                            leave = 1;
                            break;
                        }

                        flag = 0;

                        pthread_mutex_lock(&v_ingredients);
                        for (int q = 0; q < p.num_ingred; q++)
                        {
                            if (ingredients[p.ingred[q]] == 0)
                            {
                                flag = 1;
                                sem_post(&ovens);
                                break;
                            }
                        }
                        pthread_mutex_unlock(&v_ingredients);

                        if (flag == 1)
                        {
                            printf("\033[1;31mChef %d could not prepare pizza %d for order %d due to ingredient shortage\n\033[0m", inputs->id, orders[j].pizzas[l], j);
                            sem_post(&ovens);
                            orders[j].stage[l] = 3;
                            continue;
                        }

                        orders[j].stage[l] = 1;

                        clock_gettime(CLOCK_REALTIME, &ts);
                        printf("\033[1;31mChef %d has put pizza %d for order %d in oven at time %ld\n\033[0m", inputs->id, orders[j].pizzas[l], j, inputs->entry_time + ts.tv_sec - entry_time_sec);
                        pthread_mutex_lock(&v_ingredients);
                        for (int q = 0; q < p.num_ingred; q++)
                        {
                            ingredients[p.ingred[q]]--;
                        }
                        pthread_mutex_unlock(&v_ingredients);

                        sleep(p.prep_time);
                        orders[j].stage[l] = 2;

                        printf("\033[1;31mChef %d has picked up pizza %d from oven for order %d at time %ld\n\033[0m", inputs->id, orders[j].pizzas[l], j, inputs->entry_time + ts.tv_sec - entry_time_sec + p.prep_time);

                        int order_status = 1;

                        pthread_mutex_lock(&v_order);

                        for (int q = 0; q < orders[j].num_pizzas; q++)
                        {
                            if (orders[j].stage[q] == 0 || orders[j].stage[q] == 1)
                            {
                                order_status = 0;
                                break;
                            }
                            if (orders[j].stage[q] == 3)
                            {
                                order_status = 2;
                            }
                        }

                        pthread_mutex_unlock(&v_order);

                        if (order_status == 1)
                        {
                            orders[j].status = 3;
                            printf("\033[1;34mOrder %d placed by customer %d has been processed.\n\033[0m", j, orders[j].customer_id);
                        }
                        else if (order_status == 2)
                        {
                            orders[j].status = 4;
                            printf("\033[1;34mOrder %d placed by customer %d partially processed and remaining couldn't be.\n\033[0m", j, orders[j].customer_id);
                        }
                    }
                }
            }

            if (leave == 1)
                break;
        }

        if (leave == 1)
            break;
    }

    pthread_mutex_lock(&v_num_chefs);
    num_chefs--;
    pthread_mutex_lock(&v_num_chefs_left);
    num_chefs_left++;
    pthread_mutex_unlock(&v_num_chefs_left);
    pthread_mutex_unlock(&v_num_chefs);

    printf("\033[1;31mChef %d exits at time %d.\n\033[0m", inputs->id, inputs->exit_time);
}

void *customer_func(void *inp)
{

    sleep(10);
    struct timespec ts;

    struct customer *inputs = (struct customer *)inp;
    sleep(inputs->entry_time);
    printf("\033[1;33mCustomer %d arrives at time %d.\n\033[0m", inputs->id, inputs->entry_time);
    printf("\033[1;33mCustomer %d is waiting for drive-thru allocation.\n\033[0m", inputs->id); //since queue size is equal to number of customers, no semaphore required
    printf("\033[1;33mCustomer %d enters drive-thru zone and gives out their order %d.\n\033[0m", inputs->id, inputs->order_id);

    pthread_mutex_lock(&v_num_chefs_left);
    if (num_chefs_left == n)
    {
        printf("\033[1;33mCustomer %d is rejected since no incoming chefs.\n\033[0m", inputs->id);
        pthread_mutex_unlock(&v_num_chefs_left);
    }
    else
    {
        pthread_mutex_unlock(&v_num_chefs_left);
        pthread_mutex_lock(&v_order);
        printf("\033[1;34mOrder %d placed by customer %d has pizzas {", inputs->order_id, inputs->id);
        for (int j = 0; j < orders[inputs->order_id].num_pizzas; j++)
        {
            printf("%d", orders[inputs->order_id].pizzas[j]);
            if (j != orders[inputs->order_id].num_pizzas - 1)
                printf(", ");
            else
            {
                printf("}.\n\033[0m");
            }
        }

        pthread_mutex_lock(&v_num_chefs_left);
        if (num_chefs_left == n)
        {
            printf("\033[1;33mCustomer %d is rejected since no incoming chefs.\n\033[0m", inputs->id);
            pthread_mutex_unlock(&v_num_chefs_left);
        }
        else
        {
            pthread_mutex_unlock(&v_num_chefs_left);
            orders[inputs->order_id].status = 1;
            pthread_mutex_unlock(&v_order);

            printf("\033[1;34mOrder %d placed by customer %d awaits processing.\n\033[0m", inputs->order_id, inputs->id);
            sleep(k);
            printf("\033[1;33mCustomer %d is waiting at pickup spot.\n\033[0m", inputs->id);

            int order_status = 0;
            int tmp[1000] = {0};

            while (order_status == 0)
            {
                order_status = 1;

                pthread_mutex_lock(&v_order);

                for (int q = 0; q < orders[inputs->order_id].num_pizzas; q++)
                {
                    if (orders[inputs->order_id].stage[q] == 0 || orders[inputs->order_id].stage[q] == 1)
                    {
                        order_status *= 0;
                    }
                    else
                    {
                        if (tmp[q] == 0)
                        {
                            if (orders[inputs->order_id].stage[q] == 2)
                                printf("\033[1;33mCustomer %d picks up their pizza %d.\n\033[0m", inputs->id, orders[inputs->order_id].pizzas[q]);
                            tmp[q] = 1;
                        }
                    }
                }

                pthread_mutex_unlock(&v_order);
            }
        }
    }
    printf("\033[1;33mCustomer %d exits the drive-thru zone.\n\033[0m", inputs->id);
}

int main()
{

    num_chefs = 0, num_chefs_left = 0, num_orders = 0;

    pthread_mutex_init(&v_num_chefs, NULL);
    pthread_mutex_init(&v_num_chefs_left, NULL);
    pthread_mutex_init(&v_num_orders, NULL);
    pthread_mutex_init(&v_ingredients, NULL);

    printf("\033[1;37mSimulation started\n\n\033[0m");

    scanf("%d %d %d %d %d %d", &n, &m, &i, &c, &o, &k);

    sem_init(&ovens, 0, o);

    for (int j = 0; j < m; j++)
    {
        pizza p;
        scanf("%d %d %d", &p.id, &p.prep_time, &p.num_ingred);
        for (int l = 0; l < p.num_ingred; l++)
        {
            scanf("%d", &p.ingred[l]);
        }
        pizzas[p.id] = p;
    }

    for (int j = 1; j <= i; j++)
    {
        scanf("%d", &ingredients[j]);
    }

    pthread_t p_chef[n + 1];

    for (int j = 1; j < n + 1; j++)
    {
        int entry, exit;
        scanf("%d %d", &entry, &exit);
        struct chef *c = (struct chef *)malloc(sizeof(struct chef));
        c->id = j;
        c->entry_time = entry;
        c->exit_time = exit;
        pthread_create(&p_chef[j], NULL, chef_func, (void *)(c));
    }

    pthread_t p_customer[c + 1];

    for (int j = 1; j < c + 1; j++)
    {

        num_orders++;

        struct customer *c = (struct customer *)malloc(sizeof(struct customer));
        c->id = j;
        c->order_id = num_orders;
        orders[c->order_id].customer_id = c->id;

        scanf("%d %d", &c->entry_time, &orders[c->order_id].num_pizzas);

        for (int l = 0; l < orders[c->order_id].num_pizzas; l++)
        {
            scanf("%d", &orders[c->order_id].pizzas[l]);
        }
        for (int l = 0; l < orders[c->order_id].num_pizzas; l++)
        {
            orders[c->order_id].stage[l] = 0;
        }
        orders[c->order_id].status = 0;

        pthread_create(&p_customer[j], NULL, customer_func, (void *)(c));
    }

    for (int j = 1; j <= n; j++)
    {
        pthread_join(p_chef[j], NULL);
    }
    for (int j = 1; j <= c; j++)
    {
        pthread_join(p_customer[j], NULL);
    }

    printf("\n\n");
    printf("\033[1;37mSimulation ended!\n\033[0m");
}