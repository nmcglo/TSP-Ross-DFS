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

int is_sender_in_task_q(task *q, tw_lpid sender)
{
     for(int i = 0; i < REQ_Q_MAX_SIZE; i++)
     {
          task ur = q[i];
          if(ur.sender == sender)
               return 1;
     }
     return 0;
}

void add_to_task_queue(task *q, task ur)
{
     int cityID = get_city_from_gid(ur.sender);
     q[cityID] = ur;
}

void* process_next_task(task *q)
{
     for(int i = 0; i < REQ_Q_MAX_SIZE; i++)
     {
          if(q[i].status == QUEUED)
          {
               q[i].status = WORKING;
               return &q[i];
          }
     }
     return NULL;
}

void* get_queued_task(task *q)
{
     for(int i = 0; i < REQ_Q_MAX_SIZE; i++)
     {
          if(q[i].status == QUEUED)
               return &q[i];
     }
     return NULL;
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

void init_downstream_pq(tsp_actor_state *s, heap_t *pq)
{
     for(int i = 0; i < s->num_outgoing_neighbors; i++)
     {
          if(s->outgoingCityWeightPairs[i].cityID != 0) //we don't need to be able to send to zero
               push(pq,s->outgoingCityWeightPairs[i].weight,&(s->outgoingCityWeightPairs[i]));
     }
}

int task_eq(task *req1, task* req2)
{
     if(req1->sender == req2->sender)
     {
          if(req1->key == req2->key)
          {
               return TRUE;
          }
     }
     return FALSE;
}

void clear_finished_from_queue(task *q)
{
     task newQueue[REQ_Q_MAX_SIZE];
     int newSize = 0;
     for(int i = 0; i < REQ_Q_MAX_SIZE; i++)
     {
          if(q[i].status != FINISHED)
          {
               newQueue[newSize] = q[i];
               newSize++;
          }
     }
     q = newQueue;
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

     // s->upstream_req_pq = (heap_t *)calloc(1, sizeof(heap_t)); //init empty

     // s->num_upstream_requests = 0;
     for(int i = 0; i<MAX_TOUR_LENGTH;i++)
     {
          s->min_downstream_complete_path[i] = -1;
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

     s->num_tasks_working = 0;

     s->downstream_pq = (heap_t *)calloc(1, sizeof(heap_t));
     for(int i = 0; i < s->num_outgoing_neighbors; i++)
     {
          if(s->outgoingCityWeightPairs[i].cityID != 0) //we don't need to be able to send to zero
               push(s->downstream_pq,s->outgoingCityWeightPairs[i].weight,&(s->outgoingCityWeightPairs[i]));
     }

     if(s->self_place == total_cities - 1) //then you are a leaf - you know your distance back to city 0 so your downstream is complete
     {
          s->is_all_downstream_complete = 1;
          s->min_downstream_complete_path[s->self_place] = s->self_city;
          s->min_downstream_complete_path[s->self_place+1] = 0;
          s->min_downstream_weight = weight_matrix[0][me];
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
               for(int i = 0; i < NUM_ACTIVE_REQ_PN; i++) //starts as many tasks as allowed
               {
                    tw_stime init_time = tw_rand_unif(lp->rng)*jitter;

                    tw_event *e = tw_event_new(self,init_time,lp);
                    tsp_mess *mess = tw_event_data(e);

                    mess->sender = self;
                    mess->messType = INIT_REQUEST;
                    tw_event_send(e);
               }
          }
     }
}

void tsp_send_heartbeat(tsp_actor_state *s, tw_lp *lp)
{
     tw_stime delay = tw_rand_unif(lp->rng)*jitter;

     tw_event *e = tw_event_new(lp->gid,DELAY+delay,lp);
     tsp_mess *mess = tw_event_data(e);

     mess->messType = SELF;
     tw_event_send(e);
}


void tsp_event_handler(tsp_actor_state *s, tw_bf *bf, tsp_mess *in_msg, tw_lp *lp)
{
     in_msg->saved_rng_count = s->rng_count;

     switch(in_msg->messType)
     {
          case INIT_REQUEST: //you are receiving a self message instructing you to start - only sent in prerun
          {
               printf("%i,%i: Received INIT_REQUEST mess from %i\n",s->self_city,s->self_place,get_city_from_gid(in_msg->sender));

               s->num_tasks_working +=1;
               // s->is_working = TRUE; //now you're working on a problem!

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
               mess->messType = REQUEST;
               tw_event_send(e);

          }break;


          case REQUEST: //add task to your queue
          {
               printf("%i,%i: Received REQUEST mess from %i\n",s->self_city,s->self_place,get_city_from_gid(in_msg->sender));
               int rec_city = get_city_from_gid(in_msg->sender);

               if(rec_city != s->self_city) //don't add yourself to the task queue - you don't need to send back to yourself
               {
                    // if(!is_sender_in_task_q(s->upstream_req_q,in_msg->sender)) //then add to the task queue
                    {
                         // printf("%i,%i: Adding %i To Queue\n",s->self_city,s->self_place,get_city_from_gid(in_msg->sender));
                         task ur;
                         ur.sender = in_msg->sender;
                         ur.downstream_pq = (heap_t*)calloc(1,sizeof(heap_t));
                         init_downstream_pq(s,ur.downstream_pq);
                         ur.tour_weight = in_msg->tour_weight;
                         ur.key = tw_now(lp);
                         for(int i = 0; i < s->self_place;i++)
                         {
                              ur.upstream_proposed_tour[i] = in_msg->tour_dat.upstream_proposed_tour[i];
                         }
                         ur.status = QUEUED;
                         add_to_task_queue(s->upstream_req_q,ur);
                    }
               }

               for(int i = s->num_tasks_working; i < NUM_ACTIVE_REQ_PN; i++)
               {
                    tsp_send_heartbeat(s,lp);
               }

          }break;


          case SELF: //process a task from the queue
          {
               if(get_queued_task(s->upstream_req_q)) //if there is a task that is queued
               {
                    if(s->num_tasks_working < NUM_ACTIVE_REQ_PN)
                    {
                         printf("%i,%i: Received SELF mess\n",s->self_city,s->self_place);
                         s->num_tasks_working +=1;

                         task req = *((task*)process_next_task(s->upstream_req_q));
                         s->active_task = req;

                         int working_tour[MAX_TOUR_LENGTH];
                         for(int i = 0; i < s->self_place;i++)
                         {
                              working_tour[i] = req.upstream_proposed_tour[i];
                         }

                         //get valid next best downstream city
                         city_weight_pair next_best_downstream_city;
                         int isAlreadyInTour = FALSE;
                         void* nbdc_ptr;
                         do {
                              isAlreadyInTour = FALSE;

                              nbdc_ptr = pop(req.downstream_pq);
                              if(nbdc_ptr)
                              {
                                   next_best_downstream_city = *((city_weight_pair *) nbdc_ptr);

                                   for(int i = 0; i < s->self_place; i++)
                                   {
                                        if(next_best_downstream_city.cityID == working_tour[i])
                                             isAlreadyInTour = TRUE;
                                   }
                              }
                         } while(isAlreadyInTour);

                         if(nbdc_ptr)
                         {

                              //get gid of the valid city
                              tw_lpid dest_gid = get_lp_gid(next_best_downstream_city.cityID,s->self_place+1);

                              tw_event *e = tw_event_new(dest_gid,DELAY+tw_rand_unif(lp->rng),lp);
                              tsp_mess *mess = tw_event_data(e);

                              //tour_dat
                              for(int i = 0; i < s->self_place; i++)
                              {
                                   mess->tour_dat.upstream_proposed_tour[i] = working_tour[i];
                              }
                              mess->tour_dat.upstream_proposed_tour[s->self_place] = s->self_city;

                              mess->sender = lp->gid; //sender gid
                              mess->messType = REQUEST;
                              tw_event_send(e);
                         }
                         else
                         {
                              printf("%i,%i: LEAF!\n",s->self_city,s->self_place);

                              task theTask = s->active_task;
                              theTask.status = FINISHED;

                              tw_lpid sender = theTask.sender;

                              printf("%i,%i: Sending COMPLETE to %i\n",s->self_city,s->self_place,get_city_from_gid(sender));

                              tw_event *e = tw_event_new(sender,DELAY+tw_rand_unif(lp->rng),lp);
                              tsp_mess *mess = tw_event_data(e);

                              // tour_dat
                              for(int i = 0; i < s->self_place; i++)
                              {
                                   mess->tour_dat.downstream_min_path[i] = theTask.upstream_proposed_tour[i];
                              }
                              mess->tour_dat.downstream_min_path[s->self_place] = s->self_city;

                              //tour_weight

                              mess->sender = lp->gid; //sender gid
                              mess->messType = COMPLETE;
                              tw_event_send(e);

                              s->is_working = FALSE;
                              s->num_tasks_working = 0;

                              clear_finished_from_queue(s->upstream_req_q);

                              tsp_send_heartbeat(s,lp);
                         }
                    }
               }
          }break;


          case COMPLETE: //you're receiving a complete tour message, stop propogating weaker tours:
          //extract upstream_weight = tour_weight - downstream_weight;
          //if weight of to next best city + upstream weight is less than min_downstream_weight + upstream weight
          {
               task theTask = s->active_task;

               printf("%i,%i: Received COMPLETE from %i\n",s->self_city,s->self_place,get_city_from_gid(in_msg->sender));

               //extract the tour we're working on up to ourselves from the downstream path received
               // int working_tour[MAX_TOUR_LENGTH];
               // for(int i = 0; i < s->self_place; i++)
               // {
               //      working_tour[i] = in_msg->tour_dat.downstream_min_path[i];
               // }

               int working_tour[MAX_TOUR_LENGTH];
               for(int i = 0; i < s->self_place; i++)
               {
                    working_tour[i] = theTask.upstream_proposed_tour[i];
               }

               //TODO do stuff for the weitghts - for now focusing on the traversal
               city_weight_pair next_best_downstream_city;
               void* nbdc_ptr;
               int isAlreadyInTour = FALSE;
               do {
                    isAlreadyInTour = FALSE;

                    nbdc_ptr = pop(s->downstream_pq); //TODO HERES THE PROBLEM
                    if(nbdc_ptr)
                    {
                         next_best_downstream_city = *((city_weight_pair *) nbdc_ptr);

                         for(int i = 0; i < s->self_place; i++)
                         {
                              if(next_best_downstream_city.cityID == working_tour[i]) //only send to one that hasn't been in the tour we're working on
                                   isAlreadyInTour = TRUE;
                         }
                    }
               } while(isAlreadyInTour);

               if(nbdc_ptr)
               {
                    printf("%i,%i: sending REQUEST to %i\n",s->self_city,s->self_place,next_best_downstream_city.cityID);
                    //get gid of the valid city
                    tw_lpid dest_gid = get_lp_gid(next_best_downstream_city.cityID,s->self_place+1);

                    tw_event *e = tw_event_new(dest_gid,DELAY+tw_rand_unif(lp->rng),lp);
                    tsp_mess *mess = tw_event_data(e);

                    //tour_dat
                    for(int i = 0; i < s->self_place; i++)
                    {
                         mess->tour_dat.upstream_proposed_tour[i] = working_tour[i];
                         printf("%i ",mess->tour_dat.upstream_proposed_tour[i]);
                    }
                    mess->tour_dat.upstream_proposed_tour[s->self_place] = s->self_city;
                    printf("%i\n",mess->tour_dat.upstream_proposed_tour[s->self_place]);

                    //tour_weight
                    mess->tour_weight = 0; //TODO WEIGHTS

                    mess->sender = lp->gid; //sender gid
                    mess->messType = REQUEST;
                    tw_event_send(e);
               }
               else //you've exhausted all of your downstream_pq, now you need to send complete upstream
               {
                    printf("%i,%i: Exhausted\n",s->self_city,s->self_place);
                    if(s->self_place > 1)
                    {
                         task theTask = s->active_task;
                         theTask.status = FINISHED;

                         tw_lpid sender = theTask.sender;

                         printf("%i,%i: Sending COMPLETE to %i\n",s->self_city,s->self_place,get_city_from_gid(sender));

                         tw_event *e = tw_event_new(sender,DELAY+tw_rand_unif(lp->rng),lp);
                         tsp_mess *mess = tw_event_data(e);

                         // tour_dat
                         for(int i = 0; i < s->self_place; i++)
                         {
                              mess->tour_dat.downstream_min_path[i] = working_tour[i];
                         }
                         mess->tour_dat.downstream_min_path[s->self_place] = s->self_city;

                         //tour_weight

                         mess->sender = lp->gid; //sender gid
                         mess->messType = COMPLETE;
                         tw_event_send(e);

                         s->is_working = FALSE;
                         s->num_tasks_working = 0;

                         clear_finished_from_queue(s->upstream_req_q);

                         tsp_send_heartbeat(s,lp);
                    }
                    if(s->self_place == 1)
                    {
                         printf("%i,%i: DONE!!!!!!!!!\n",s->self_city,s->self_place);
                         for(int i = 0; i < total_cities+1;i++)
                         {
                              printf("%i ",in_msg->tour_dat.downstream_min_path[i]);
                         }
                         printf("\n");
                    }
               }



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
