/*
tsp_driver.c
Traveling Salesman Problem Solver
4-26-2017
Neil McGlohon
*/


//Includes
#include "tsp.h"


int is_in_array(int* arr, int len, int input)
{
     for(int i = 0; i<len; i++)
     {
          if(arr[i] == input)
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

     s->num_upstream_requests = 0;
     for(int i = 0; i<MAX_TOUR_LENGTH;i++)
     {
          s->min_downstream_complete_path[i] = 0;
          s->upstream_requests[i] = -1;
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

     if(s->self_city != 0)
     {
          if(s->self_place == 1) //start with the non-zero cities with tours that have 0 already in them
          {
               tw_stime init_time = tw_rand_unif(lp->rng)*jitter;

               tw_event *e = tw_event_new(self,init_time,lp);
               tsp_mess *mess = tw_event_data(e);

               mess->sender = self;

               for(int i = 0; i<MAX_TOUR_LENGTH; i++)
               {
                    mess->tour_dat.upstream_proposed_tour[i] = 0;
               }
               for(int i = 0; i < total_cities; i++)
               {
                    if(s->incomingCityWeightPairs[i].cityID == 0)
                    {
                         mess->tour_weight = s->incomingCityWeightPairs[i].weight;
                    }
               }
               mess->messType = TOUR;
               tw_event_send(e);
          }
     }
}


void tsp_event_handler(tsp_actor_state *s, tw_bf *bf, tsp_mess *in_msg, tw_lp *lp)
{
     in_msg->saved_rng_count = s->rng_count;

     switch(in_msg->messType)
     {
          case TOUR: //add and propogate the tour message
          {
               printf("%i: Received TOUR mess\n",s->self_city);
               int rec_city = get_city_from_gid(in_msg->sender);


               if(s->is_all_downstream_complete) //if all downstream complete -- send back a complete mess with min path/weight
               {

               }
               else //all downstream is complete is not complete
               {
                    if(!is_in_array(s->upstream_requests,s->num_upstream_requests,rec_city)) //add to the request queue
                    {
                         s->upstream_requests[s->num_upstream_requests] = rec_city;
                         s->num_upstream_requests++;
                    }

                    if(s->is_working) //currently working on subproblem
                    {
                         printf("%i,%i: Received a request while working\n",s->self_city,s->self_place);
                    }
                    else //haven't started working on a subproblem
                    {
                         s->is_working = TRUE; //now you're working on a problem!


                    }
               }


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
