#ifndef globals_h
#define globals_h

#define FALSE 0
#define TRUE 1

#define MAX_TOUR_LENGTH 16
#define MIN_CITY_SEPARATION 1
#define MAX_CITY_SEPARATION 100


double jitter;

tw_stime lookahead;
unsigned int nlp_per_pe;
unsigned int custom_LPs_per_pe;
int total_actors;
int total_cities;

//GLOBALS

int** weight_matrix;


#endif
