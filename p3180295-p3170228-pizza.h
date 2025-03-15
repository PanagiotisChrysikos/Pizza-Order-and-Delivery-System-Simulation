#include <time.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

const int N_phones = 2; // number of maximum phones
const int N_cook = 2; //number of maximum cooks
const int N_oven = 5; //number of maximum ovens
const int N_deliverer = 10; //number of maximum deliverers

const int Time_order_low = 1; // [Time_order_low - Time_order_high] : time range in which a new order arrives
const int Time_order_high = 5;

const int N_order_low = 1; // [N_order_low - N_order_high] : range of possible number of pizzas in each order
const int N_order_high = 5;

const float p_margrita = 0.35; // what are the chances of each pizza type being ordered
const float p_peperoni = 0.25;
const float p_special = 0.40;

const double cost_per_pizza_margarita = 10;
const double cost_per_pizza_peperoni = 11;
const double cost_per_pizza_special = 12;
double total_income;

const float payment_fail = 0.05; // chance of a fail payment

const int payment_low = 1; // time that the telephone needs to charge a customer's card
const int payment_high = 3;

const int T_low = 5; // [T_low - T_high] : time range in which a deliverer delivers the order to the customer
const int T_high = 15;

const int T_prep = 1; // preparation time for each pizza

const int T_bake = 10; // baking time for each pizza

const int T_pack = 1; // packing time for each pizza

//Declaration of variables
int no_available_phones;
int no_available_cooks;
int no_available_ovens;
int no_available_deliverers;

double order_time;
double total_order_time;
double max_order_time;

double cold_time;
double total_cold_time;
double max_cold_time;

int * pizza_margarita;
int * pizza_peperoni;
int * pizza_special;

unsigned int seed;

//Declaration of mutexes
pthread_mutex_t mutex_no_available_phones= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_no_available_cooks = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_no_available_ovens = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_no_available_deliverers = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_total_order_time = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_max_order_time = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_total_cold_time = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_max_cold_time = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_total_income = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutex_lock_screen = PTHREAD_MUTEX_INITIALIZER;

//Declaration of conditions
pthread_cond_t cond_no_available_cooks = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_no_available_ovens = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_no_available_deliverers = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_no_available_phones = PTHREAD_COND_INITIALIZER; 
