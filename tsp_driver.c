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

int is_city_in_heap(heap_t *h, int cityID)
{
     for(int i = 0; i < h->len; i++)
     {
          int cityFromHeap = *((int*)getAt(h,i));
          if(cityFromHeap == cityID)
          {
               return 1;
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

     s->upstream_req_pq = (heap_t *)calloc(1, sizeof(heap_t)); //init empty

     // s->num_upstream_requests = 0;
     for(int i = 0; i<MAX_TOUR_LENGTH;i++)
     {
          s->min_downstream_complete_path[i] = 0;
          // s->upstream_requests[i] = -1;
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

     // printf("%i: Num Incoming Neighbors = %i\n",s->self_city,s->num_incoming_neighbors);

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

     s->downstream_pq = (heap_t *)calloc(1, sizeof(heap_t));
     for(int i = 0; i < s->num_outgoing_neighbors; i++)
     {
          if(s->outgoingCityWeightPairs[i].cityID != 0) //we don't need to be able to send to zero
               push(s->downstream_pq,s->outgoingCityWeightPairs[i].weight,&(s->outgoingCityWeightPairs[i]));
     }

     // if(s->self_city == 1)
     // {
     //      for(int i = 0; i < s->num_outgoing_neighbors; i++)
     //      {
     //           city_weight_pair pair = *((city_weight_pair *) getAt(s->downstream_pq, i));
     //           printf("(%i,%i) ",pair.cityID, pair.weight);
     //      }
     //      printf("\n");
     // }

     // printf("%i: Num Outgoing Neighbors = %i\n",s->self_city,s->num_outgoing_neighbors);



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
               mess->messType = SELF;
               tw_event_send(e);
          }
     }
}


void tsp_event_handler(tsp_actor_state *s, tw_bf *bf, tsp_mess *in_msg, tw_lp *lp)
{
     in_msg->saved_rng_count = s->rng_count;

     switch(in_msg->messType)
     {
          case SELF: //you are receiving a self message instructing you to start - only sent in prerun
          {
               printf("%i,%i: Received SELF mess from %i\n",s->self_city,s->self_place,get_city_from_gid(in_msg->sender));

               s->is_working = TRUE; //now you're working on a problem!

               city_weight_pair next_best_downstream_city = *((city_weight_pair *) pop(s->downstream_pq));

               tw_lpid dest_gid = get_lp_gid(next_best_downstream_city.cityID,s->self_place+1);

               tw_event *e = tw_event_new(dest_gid,DELAY+tw_rand_unif(lp->rng)*jitter,lp);
               tsp_mess *mess = tw_event_data(e);

               mess->sender = lp->gid;

               //initialize the proposed tour so far
               for(int i = 0; i<MAX_TOUR_LENGTH; i++)
               {
                    mess->tour_dat.upstream_proposed_tour[i] = -1;
               }
               mess->tour_dat.upstream_proposed_tour[0] = 0; //first one is zero
               mess->tour_dat.upstream_proposed_tour[1] = s->self_city; //the next is yourself

               //initialize the weight of the proposed tour so far
               for(int i = 0; i < s->num_incoming_neighbors; i++)
               {
                    if(s->incomingCityWeightPairs[i].cityID == 0)
                    {
                         mess->tour_weight = s->incomingCityWeightPairs[i].weight; //distance from 0 to you
                    }
               }
               mess->messType = TOUR;
               tw_event_send(e);

          }break;
          case TOUR: //add and propogate the tour message
          {
               printf("%i,%i: Received TOUR mess from %i\n",s->self_city,s->self_place,get_city_from_gid(in_msg->sender));
               int rec_city = get_city_from_gid(in_msg->sender);


               if(s->is_all_downstream_complete) //if all downstream complete -- send back a complete mess with min path/weight
               {
                    printf("%i,%i: all downstream complete\n",s->self_city,s->self_place);
               }
               else //all downstream is complete is not complete
               {
                    if(rec_city != s->self_city) //don't add yourself to the request queue - you don't need to send back to yourself
                    {
                         if(!is_city_in_heap(s->upstream_req_pq,rec_city)) //then add to the request queue
                         {
                              printf("%i,%i: Adding %i To Queue\n",s->self_city,s->self_place,rec_city);
                              push(s->upstream_req_pq,0,&rec_city);
                         }
                    }


                    if(s->is_working) //currently working on subproblem
                    {
                         printf("%i,%i: Received a request from %i while working\n",s->self_city,s->self_place,rec_city);
                    }
                    else //haven't started working on a subproblem
                    {
                         printf("%i,%i: starting work\n",s->self_city,s->self_place);

                         s->is_working = TRUE; //now you're working on a problem!

                         //get valid next best downstream city
                         city_weight_pair next_best_downstream_city;
                         int nbdc_is_undefined = FALSE;
                         int isAlreadyInTour = FALSE;
                         do {
                              isAlreadyInTour = FALSE;

                              void* nbdc_ptr = pop(s->downstream_pq);
                              if(nbdc_ptr != NULL)
                              {
                                   next_best_downstream_city = *((city_weight_pair *) nbdc_ptr);

                                   for(int i = 0; i < s->self_place; i++)
                                   {
                                        if(next_best_downstream_city.cityID == in_msg->tour_dat.upstream_proposed_tour[i])
                                             isAlreadyInTour = TRUE;
                                   }
                              }
                              else
                              {
                                   nbdc_is_undefined = TRUE;
                              }
                         } while(isAlreadyInTour);

                         if(!nbdc_is_undefined)
                         {

                              //get gid of the valid city
                              tw_lpid dest_gid = get_lp_gid(next_best_downstream_city.cityID,s->self_place+1);

                              tw_event *e = tw_event_new(dest_gid,DELAY+tw_rand_unif(lp->rng),lp);
                              tsp_mess *mess = tw_event_data(e);

                              //tour_dat
                              for(int i = 0; i < s->self_place; i++)
                              {
                                   mess->tour_dat.upstream_proposed_tour[i] = in_msg->tour_dat.upstream_proposed_tour[i];
                                   printf("%i ",in_msg->tour_dat.upstream_proposed_tour[i]);
                              }
                              mess->tour_dat.upstream_proposed_tour[s->self_place] = s->self_city;
                              printf("%i\n",mess->tour_dat.upstream_proposed_tour[s->self_place]);

                              //tour_weight
                              for(int i = 0; i < s->num_incoming_neighbors; i++)
                              {
                                   if(s->incomingCityWeightPairs[i].cityID == rec_city)
                                   {
                                        mess->tour_weight = in_msg->tour_weight + s->incomingCityWeightPairs[i].weight;
                                   }
                              }

                              mess->sender = lp->gid; //sender gid
                              mess->messType = TOUR;
                              tw_event_send(e);
                         }
                         else
                         {
                              printf("%i,%i: I'm a leaf - i'd send back the distance and path to 0!\n",s->self_city,s->self_place);
                         }
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
     // int self = lp->gid;
     // if(s->self_place == 1) //the first non-zero cities in the tour report the best they have - this should be allreduced in final version
     // {
     //           printf("%d: Min Tour Weight: %d\n",s->self_city,s->min_downstream_weight);
     // }
}
