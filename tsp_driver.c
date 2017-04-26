/*
tsp_driver.c
Traveling Salesman Problem Solver
3-30-2017
Neil McGlohon
*/


//Includes
#include "tsp.h"


int is_in_tour(int* tour, int len, int input)
{
     for(int i = 0; i<len; i++)
     {
          if(tour[i] == input)
          {
               return 1;
               break;
          }
     }

     return 0;
}



//--------------LIF Neuron stuff-------------

void tsp_init (tsp_actor_state *s, tw_lp *lp)
{
     int self = lp->gid;
     s->self_city = (lp->gid)%total_cities;
     s->self_place = (lp->gid)/total_cities;
     s->rng_count = 0;
     s->min_downstream_weight = INT_MAX;

     s->incomingCityWeightPairs = calloc(total_cities, sizeof(city_weight_pair));
     s->outgoingCityWeightPairs = calloc(total_cities, sizeof(city_weight_pair));


     for(int i = 0; i<MAX_TOUR_LENGTH;i++)
     {
          s->min_downstream_complete_path[i] = 0;
     }


     int me = s->self_city;
     s->num_incoming_neighbors = 0;
     for(int j = 0; j < total_cities; j++)
     {
          if(weight_matrix[me][j] > 0)
          {
               s->incomingCityWeightPairs[s->num_incoming_neighbors].cityID = j;
               s->incomingCityWeightPairs[s->num_incoming_neighbors].weight = weight_matrix[me][j];
               s->num_incoming_neighbors +=1;
          }
     }

     printf("%i: Num Incoming Neighbors = %i\n",s->self_city,s->num_incoming_neighbors);

     s->num_outgoing_neighbors = 0;
     for(int j = 0; j < total_cities; j++)
     {
          if(weight_matrix[j][me] > 0)
          {
               s->outgoingCityWeightPairs[s->num_outgoing_neighbors].weight = weight_matrix[j][me];
               s->outgoingCityWeightPairs[s->num_outgoing_neighbors].cityID = j;
               s->num_outgoing_neighbors += 1;
          }
     }

     printf("%i: Num Outgoing Neighbors = %i\n",s->self_city,s->num_outgoing_neighbors);



     if(self == 0)
     {
          for(int i = 0; i < total_cities; i++)
          {
               for(int j = 0; j < total_cities; j++)
               {
                    printf("%d ",weight_matrix[i][j]);
               }
               printf("\n");
          }
     }
}

void tsp_prerun(tsp_actor_state *s, tw_lp *lp)
{
     int self = lp->gid;


}


void tsp_event_handler(tsp_actor_state *s, tw_bf *bf, tsp_mess *in_msg, tw_lp *lp)
{
     in_msg->saved_rng_count = s->rng_count;

     switch(in_msg->messType)
     {
          case TOUR: //add and propogate the tour message
          {


          }break;
          case COMPLETE: //you're receiving a complete tour message, stop propogating weaker tours
          {


          }break;
     }
}

void tsp_RC_event_handler(tsp_actor_state *s, tw_bf *bf, tsp_mess *in_msg, tw_lp *lp)
{
     int cur_rng_count = s->rng_count;
     int prev_rng_count = in_msg->saved_rng_count;
     int total_rollbacks_needed = cur_rng_count - prev_rng_count;

     for(int i = 0; i<total_rollbacks_needed; i++)
     {
          tw_rand_reverse_unif(lp->rng);
     }
     s->rng_count = in_msg->saved_rng_count;

}

void tsp_commit(tsp_actor_state *s, tw_bf *bf, tsp_mess *in_msg, tw_lp *lp)
{

}

void tsp_final(tsp_actor_state *s, tw_lp *lp)
{
     int self = lp->gid;
     if(s->self_place == 1) //the first non-zero cities in the tour report the best they have - this should be allreduced in final version
     {
               printf("%d: Min Tour Weight: %d\n",s->self_city,s->min_downstream_weight);
     }
}
