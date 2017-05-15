/*
tsp.h
Traveling Salesman Solver
4-26-17
Neil McGlohon
*/


#ifndef _tsp_h
#define _tsp_h

#define FALSE 0
#define TRUE 1

#define MAX_TOUR_LENGTH 10
#define NUM_ACTIVE_REQ_PN 1
#define REQ_Q_MAX_SIZE MAX_TOUR_LENGTH*NUM_ACTIVE_REQ_PN
#define MIN_CITY_SEPARATION 1
#define MAX_CITY_SEPARATION 100

#define DELAY 100

#include "ross.h"
#include "structs.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdint.h>




//MAPPING -----------------------------
extern tw_peid tsp_map(tw_lpid gid);
extern tw_lpid get_lp_gid(int city, int place);
extern int get_city_from_gid(tw_lpid gid);
extern int get_place_from_gid(tw_lpid gid);

//TOUR STUFF ------------------------------



//DRIVER STUFF -----------------------------
extern void tsp_init (tsp_actor_state *s, tw_lp *lp);
extern void tsp_prerun(tsp_actor_state *s, tw_lp *lp);
extern void tsp_event_handler(tsp_actor_state *s, tw_bf *bf, tsp_mess *in_msg, tw_lp *lp);
extern void tsp_RC_event_handler(tsp_actor_state *s, tw_bf *bf, tsp_mess *in_msg, tw_lp *lp);
extern void tsp_commit(tsp_actor_state*s,tw_bf *bf, tsp_mess *in_msg, tw_lp *lp);
extern void tsp_final(tsp_actor_state *s, tw_lp *lp);


//IO STUFF -------------------------------


//MAIN STUFF-----------------------------

extern int rand_range(int low, int high);


extern tw_lptype model_lps[];



double jitter;

tw_stime lookahead;
unsigned int nlp_per_pe;
unsigned int custom_LPs_per_pe;
int total_actors;
int total_cities;
int max_num_required_to_send_out; //maximum number of downstream requests to make at the top level to ensure that all possible tours are enumerated. (ceil(total_cities/2))

//GLOBALS

int** weight_matrix;








#endif
