#include "p3180295-p3170228-pizza.h"

void *initialize(void *order_id) {
    int *oid;
    oid = (int *)order_id;
    unsigned int seedp = seed^(*oid); // Produce a seed unique for each thread
    int rc;
    struct timespec order_start; // When the order is given
    struct timespec order_placed; // When the order is placed
    struct timespec order_prep; // When the order starts being prepared
    struct timespec order_bake; // When the order is inserted in the oven
    struct timespec order_stop; // When the order is out of the oven
    struct timespec order_deliver; // When the order is delivered to the customer

     // Get the time the order was given
    clock_gettime(CLOCK_REALTIME, &order_start);
    printf("Taking order: %d.\n", *oid);

    
    int no_margarita;
    int no_peperoni;
    int no_special;
    int total_orders;
    
    
    // Lock mutex before checking available phones
    rc = pthread_mutex_lock(&mutex_no_available_phones); 
        if (rc != 0) {
            printf("ERROR: Failed to lock mutex %d\n", rc);
            pthread_exit(&rc);
        }
        
        while (no_available_phones <= 0) {
            // Wait until a telephone becomes available
            rc = pthread_cond_wait(&cond_no_available_phones, &mutex_no_available_phones);
            if (rc != 0) {
                printf("ERROR: Failed to wait on condition variable %d\n", rc);
                pthread_exit(&rc);
            }
        }

    
    // Get the time that the order is placed
    clock_gettime(CLOCK_REALTIME, &order_placed);
    
    float payment_probability = (rand_r(&seedp) % 100) / 100.0f;
    if(payment_probability <= payment_fail) {
        printf("Payment for order %d failed, your order has not been placed. %ld\n", *oid, order_placed.tv_sec);
    }else{
       printf("Order %d was placed. %ld\n", *oid, order_placed.tv_sec); 
    }


        // Determine the number and types of pizzas in the order
    int no_pizzas = rand_r(&seedp) % N_order_high + N_order_low;
    for(int i = 0; i < no_pizzas; ++i){
        float pizza_selection = (rand_r(&seedp) % 100) / 100.0f;
        pthread_mutex_lock(&mutex_total_income);
        if (pizza_selection <= p_margrita) {
            no_margarita++;
            total_income += cost_per_pizza_margarita;
        } else if (pizza_selection <= p_peperoni) {
            no_peperoni++;
            total_income += cost_per_pizza_peperoni;
        } else {
            no_special++;
            total_income += cost_per_pizza_special;
        }
        pthread_mutex_unlock(&mutex_total_income);
    }   


    rc = pthread_mutex_lock(&mutex_no_available_cooks);
    if(rc != 0){
        printf("ERROR: Failed to lock mutex %d\n", rc);
        pthread_exit(&rc);
    }

    while(no_available_cooks<=0){
        rc = pthread_cond_wait(&cond_no_available_cooks, &mutex_no_available_cooks);
        if(rc != 0){
            printf("ERROR: Failed to wait on condition variable %d\n", rc);
            pthread_exit(&rc);
        }
    }
    // Available cook

    printf("Your order %d is underway.\n", *oid);
    --no_available_cooks; // Available cook was found and taken for the order
    rc = pthread_mutex_unlock(&mutex_no_available_cooks);
    if(rc != 0){
        printf("ERROR: Failed to unlock mutex %d\n", rc);
        pthread_exit(&rc);
    }


    // The time the order has started being prepared
    clock_gettime(CLOCK_REALTIME, &order_prep);
    
    // Each pizza has to be prepared for T_prep duration
    sleep(no_pizzas*T_prep);
    printf("%d Pizzas are prepared for order %d.\n", no_pizzas, *oid);

    // An oven should be empty before a new pizza starts baking
    rc = pthread_mutex_lock(&mutex_no_available_ovens);
    if(rc != 0){
        printf("ERROR: Failed to lock mutex %d\n", rc);
        pthread_exit(&rc);
    }
    while(no_available_ovens<=0){
        rc = pthread_cond_wait(&cond_no_available_ovens, &mutex_no_available_ovens);
        if(rc != 0){
            printf("ERROR: Failed to wait on condition variable %d\n", rc);
            pthread_exit(&rc);
        }
    }
    // Available oven
    printf("Order %d is currently in the oven...\n", *oid);
    --no_available_ovens; // Empty oven was found and pizzas were inserted for the baking
    rc = pthread_mutex_unlock(&mutex_no_available_ovens);
    if(rc != 0){
        printf("ERROR: Failed to unlock mutex %d\n", rc);
        pthread_exit(&rc);
    }
    // Pizzas where inserted in the oven so cook if free again

    rc = pthread_mutex_lock(&mutex_no_available_cooks);
    if(rc != 0){
        printf("ERROR: Failed to lock mutex %d\n", rc);
        pthread_exit(&rc);
    }
    ++no_available_cooks;

    pthread_cond_signal(&cond_no_available_cooks);
    rc = pthread_mutex_unlock(&mutex_no_available_cooks);
    if(rc != 0){
        printf("ERROR: Failed to unlock mutex %d\n", rc);
        pthread_exit(&rc);
    }

    // The times that the pizzas got in the oven
    clock_gettime(CLOCK_REALTIME, &order_bake);

    sleep(T_bake); // Waiting for pizza to bake
    sleep(T_pack); // Waiting for pizza to be packed
    printf("Order %d is ready, waiting for deliverer!\n", *oid);

    // The baking is done but must wait for the delivery guy to take them
    rc = pthread_mutex_lock(&mutex_no_available_deliverers);
    if(rc != 0){
        printf("ERROR: Failed to lock mutex %d\n", rc);
        pthread_exit(&rc);
    }
    while(no_available_deliverers<=0){
        rc = pthread_cond_wait(&cond_no_available_deliverers, &mutex_no_available_deliverers);
        if(rc != 0){
            printf("ERROR: Failed to wait on condition variable %d\n", rc);
            pthread_exit(&rc);
        }
    }
    // Available deliverer

    printf("The delivery of your order %d will begin shortly \n", *oid);
    --no_available_deliverers; // Available deliverer was found to deliver the order
    
    rc = pthread_mutex_unlock(&mutex_no_available_deliverers);
    if(rc != 0){
        printf("ERROR: Failed to unlock mutex %d\n", rc);
        pthread_exit(&rc);
    }
    // Get the time the pizzas were out of the oven
    clock_gettime(CLOCK_REALTIME, &order_stop); 

    // Now the oven is ready to be released
    rc = pthread_mutex_lock(&mutex_no_available_ovens);
    if(rc != 0){
        printf("ERROR: Failed to lock mutex %d\n", rc);
        pthread_exit(&rc);
    }
    ++no_available_ovens;
    pthread_cond_signal(&cond_no_available_ovens);
    rc = pthread_mutex_unlock(&mutex_no_available_ovens);
    if(rc != 0){
        printf("ERROR: Failed to unlock mutex %d\n", rc);
        pthread_exit(&rc);
    }

    // Delivery time
    int T_delivery = rand_r(&seedp)%T_high + T_low;

    sleep(T_delivery); // Delivery guy is on the way to the customer
    

    // Get the time the order was delivered
    clock_gettime(CLOCK_REALTIME, &order_deliver);
    printf("Order for %d finally delivered at %ld!: ",*oid,order_deliver.tv_sec); // 156789
    total_orders++;
    
    sleep(T_delivery); // Delivery guy is returning to the store

    // Delivery guy is free to take an order again
    rc = pthread_mutex_lock(&mutex_no_available_deliverers);
    if(rc != 0){
        printf("ERROR: Failed to lock mutex %d\n", rc);
        pthread_exit(&rc);
    }

    ++no_available_deliverers;
    pthread_cond_signal(&cond_no_available_deliverers);
    rc = pthread_mutex_unlock(&mutex_no_available_deliverers); 
    if(rc != 0){
        printf("ERROR: Failed to unlock mutex %d\n", rc);
        pthread_exit(&rc);
    }

    // Update total cold time and total order time
    rc = pthread_mutex_lock(&mutex_total_cold_time);
    if(rc != 0) {
        printf("ERROR: Failed to lock mutex %d\n", rc);
        pthread_exit(&rc);
    }
    cold_time = order_deliver.tv_sec - order_stop.tv_sec;
    total_cold_time += cold_time;

    // Update max cold time
    rc = pthread_mutex_lock(&mutex_max_cold_time);
    if(rc != 0) {
        printf("ERROR: Failed to lock mutex %d\n", rc);
        pthread_exit(&rc);
    }
    if(cold_time>max_cold_time){
        max_cold_time = cold_time;
    }
    rc = pthread_mutex_unlock(&mutex_max_cold_time);
    if(rc != 0){
        printf("ERROR: Failed to lock mutex%d\n", rc);
        pthread_exit(&rc);
    }
    rc = pthread_mutex_lock(&mutex_total_order_time);
    if(rc != 0) {
        printf("ERROR: Failed to lock mutex %d\n", rc);
        pthread_exit(&rc);
    }
    rc = pthread_mutex_lock(&mutex_lock_screen);
    if(rc != 0){
        printf("ERROR: Failed to lock mutexe %d\n", rc);
        pthread_exit(&rc);
    }

    order_time = cold_time + ((order_stop.tv_sec - order_bake.tv_sec) + (order_bake.tv_sec - order_prep.tv_sec) + (order_prep.tv_sec - order_start.tv_sec));
    total_order_time += order_time;

    rc = pthread_mutex_unlock(&mutex_lock_screen);
    if(rc != 0){
        printf("ERROR: Failed to unlock mutex%d\n", rc);
        pthread_exit(&rc);
    }

    // Update max waiting time
    rc = pthread_mutex_lock(&mutex_max_order_time);
    if(rc != 0) {
        printf("ERROR: Failed to lock mutex %d\n", rc);
        pthread_exit(&rc);
    }
    if(order_time>max_order_time){
        max_order_time = order_time;
    }

    rc = pthread_mutex_unlock(&mutex_max_order_time);
    if(rc != 0){
        printf("ERROR: Failed to unlock mutex %d\n", rc);
        pthread_exit(&rc);
    }
    rc = pthread_mutex_unlock(&mutex_total_order_time);
    if(rc != 0){
        printf("ERROR: Failed to unlock mutex %d\n", rc);
        pthread_exit(&rc);
    }
    rc = pthread_mutex_unlock(&mutex_total_cold_time);
    if(rc != 0){
        printf("ERROR: Failed to unlock mutex %d\n", rc);
        pthread_exit(&rc);
    }
    pthread_exit(oid);



    int main(int argc, char *argv[]) {
    int seed;

    if(argc != 3) {
        printf("ERROR: Program requires 2 arguments.\n\n");
        exit(-1);
    }

    int N_cust = atoi(argv[1]);
    if(N_cust <0) {
        printf("ERROR: Customer number can't be negative.\n\n");
        exit(-1);
    }

    seed = atoi(argv[2]);

    printf("\nThere are %d customers and the seed is %d.\n\n", N_cust, seed);

    // Initialize variables to be used
    order_time = 0;
    total_order_time = 0;
    max_order_time = 0;
    total_cold_time = 0;
    max_cold_time = 0;
    no_available_phones = N_phones;
    no_available_cooks = N_cook;
    no_available_ovens = N_oven;
    no_available_deliverers = N_deliverer;


    // Initialize order_id for each order
    int order_id[N_cust];
    for(int i=0;i<N_cust;++i)
    {
        order_id[i] = i+1;
    }


    // Initialize threads used for orders
    pthread_t *threads;
    threads = malloc(N_cust*sizeof(pthread_t));
    if(threads==NULL){
        printf("ERROR allocating memory.\n");
        exit(-1);
    }

    int rc;
    for(int i=0; i<N_cust; ++i)
    {
        rc = pthread_create(&threads[i], NULL, &initialize, &order_id[i]);
        int wait = rand_r(&seed) %Time_order_high + Time_order_low;
        sleep(wait);
        if (rc != 0) {
            printf("ERROR: pthread_create() returned %d\n", rc);
            exit(-1);
        }
    }


    void *status;
    for(int i=0; i<N_cust; i++)
    {
        rc = pthread_join(threads[i], &status);
        if (rc != 0) {
            printf("ERROR: pthread_join() returned %d on thread %d\n", rc, order_id[i]);
            exit(-1);
        }
    }

    //Print average order preparation time
    printf("\n\nAverage order time is: %f", total_order_time / N_cust);
    //Print max order preparation time
    printf("\nMax order time is: %f", max_order_time);
    //Print average order freezing time
    printf("\nAverage freezing time is: %f", total_cold_time / N_cust);
    //Print max order freezing time
    printf("\nMax freezing time is: %f", max_cold_time);
    printf("\nTotal income from %d orders is: %.2f\n", total_orders, total_income);
    printf("\nMore specifically we sold:\n%d Margarita\n%d Peperoni\n%d Special\n", no_margarita, no_peperoni, no_special);
    free(threads);
    return 1;
    }
}
