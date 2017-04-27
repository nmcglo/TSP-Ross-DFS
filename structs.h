#ifndef _structs_h
#define _structs_h

#include "priority_queue.h"

typedef struct
{
     int cityID;
     int weight;
} city_weight_pair;

typedef enum
{
     TOUR = 1,
     COMPLETE,
     SELF
} tsp_msg_type;


//TODO order the declarations to optimize memory usage
typedef struct
{
     city_weight_pair* incomingCityWeightPairs;
     city_weight_pair* outgoingCityWeightPairs;
     int min_downstream_complete_path[MAX_TOUR_LENGTH];
     heap_t* downstream_pq;
     heap_t* upstream_req_pq;
     int num_upstream_requests;
     int self_place;
     int self_city;
     int rng_count;
     int min_downstream_weight;
     int num_incoming_neighbors;
     int num_outgoing_neighbors;
     int is_all_downstream_complete;
     int is_working;
} tsp_actor_state;


typedef struct
{
     union{
          int upstream_proposed_tour[MAX_TOUR_LENGTH]; //defined if messType == TOUR
          int downstream_min_path[MAX_TOUR_LENGTH]; //defined if messType == COMPLETE
     } tour_dat; //For simplicity of understanding
     int tour_weight;
     tw_lpid sender;
     tsp_msg_type messType;
     int saved_rng_count;
} tsp_mess;


#endif
