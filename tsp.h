/*
tsp.h
Traveling Salesman Solver
4-26-17
Neil McGlohon
*/


#ifndef _tsp_h
#define _tsp_h

#include "ross.h"
#include "globals.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdint.h>




//STRUCTS ------------------------------

typedef struct
{
     int cityID;
     int weight;
} city_weight_pair;

//TODO order the declarations to optimize memory usage
typedef struct
{
     city_weight_pair* incomingCityWeightPairs;
     city_weight_pair* outgoingCityWeightPairs;
     int min_downstream_complete_path[MAX_TOUR_LENGTH];
     int self_place;
     int self_city;
     int rng_count;
     int min_downstream_weight;
     int num_incoming_neighbors;
     int num_outgoing_neighbors;
} tsp_actor_state;

typedef enum
{
     TOUR = 1,
     COMPLETE
} tsp_msg_type;


typedef struct
{
     union{
          int upstream_proposed_tour[MAX_TOUR_LENGTH]; //defined if messType == TOUR
          int downstream_min_path[MAX_TOUR_LENGTH]; //defined if messType == COMPLETE
     } tour_dat; //For simplicity of understanding
     int tour_weight;
     tw_lpid sender;
     int saved_rng_count;
     tsp_msg_type messType;
} tsp_mess;


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


//IO STUFF --------------------------------


//MAIN STUFF-----------------------------

extern int rand_range(int low, int high);


extern tw_lptype model_lps[];








#endif
