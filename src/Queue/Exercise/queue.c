#include <stdio.h>
#include <math.h>
#include <stdlib.h>
//#include "rand.h"


#define Q_LIMIT 100
#define BUSY 1
#define IDLE 0

int next_event_type, num_custs_delayed, num_delays_required, num_events, num_in_q, server_status;
float area_num_in_q, area_server_status, mean_interarrival, mean_service, time, time_arrival[Q_LIMIT + 1], time_last_event, time_next_event[3], total_of_delays;

FILE *infile, *outfile;

void initialize(void);
void timing(void);
void arrive(void);
void depart(void);
void report(void);
void update_time_avg_stats(void);
float expon(float mean);

int main()
{
   // Open input and output files
   infile = fopen("num1.in", "r");
   outfile = fopen("num1.out", "w");

   // Specify the number of events for the timing function
   num_events = 2;

   // Read input parameters
   fscanf(infile, "%f %f %d", &mean_interarrival, &mean_service, &num_delays_required);

   // Write report headingg and input parameters
   fprintf(outfile, "Single-server queueing system\n\n");
   fprintf(outfile, "Mean intearrival time %11.3f minutes\n\n", mean_interarrival);
   fprintf(outfile, "Mean service time %16.3f minutes\n\n", mean_service);
   fprintf(outfile, "Number of customers %14d\n\n", num_delays_required);


   // Initialize the simulation
   initialize();

   // Run  the simulation while more delays are still needed

   while (num_custs_delayed < num_delays_required) {

      // Determine the next event
      timing();
      // Update time_average statistical accumulators
      update_time_avg_stats();
      // Invoke the appropriate event function
      switch(next_event_type) {

         case 1:
            arrive();
            break;
         case 2:
            depart();
            break;
      }
   }

   // Invoke the report generator and end the simulation
   report();

   fclose(infile);
   fclose(outfile);

   return 0;
}


void initialize(void)
{
   time = 0.0;

   // Initialize the state variables
   server_status = IDLE;
   num_in_q = 0;
   time_last_event = 0.0;

   // Initialize event list. Since no customers are present, the departure (service completion) event is eliminated from consideration
   time_next_event[1] = time + expon(mean_interarrival);
   time_next_event[2] = 1.0e+30;
}

void timing(void)
{
   int i;
   float min_time_next_event = 1.0e+25;

   next_event_type = 0;

   // Determine the event type of the next event to occur
   for(i = 1; i <= num_events; i++) {
      if (time_next_event[i] < min_time_next_event) {
         min_time_next_event = time_next_event[i];
         next_event_type = i;
      }
   }

   // Check to see whether the event list is empty

   if (next_event_type == 0) {
      // The event list is empty so stop the simulation
      fprintf(outfile, "\nEvent list empty at the time %f", time);
      exit(1);
   }

   // The event list is not empty, so advance the simulation clock
   time = min_time_next_event;
}

/* Arrival event function */
void arrive(void)
{
   float delay;
   
   // Schedule next interval
   time_next_event[1] = time+expon(mean_interarrival);

   //  Check whether the server is busy
   if (server_status == BUSY) {
      // server is busy, so increment number of customers in queue
      ++ num_in_q;

      // check to see whether an overflow condition exists
      if (num_in_q > Q_LIMIT) {
         // The queue has overflowed, so stop the simulation
         fprintf(outfile, "\nOverflow of the array time_arrival at");
         fprintf(outfile, " time %f", time);
         exit(2);
      }

      // There is still room in the queue, so store the timee of arrival of the arriving customer at the (new) end if time_arrival
      time_arrival[num_in_q] = time;
   }
   else {
      // Server is idle, so arriving customer has a delay of zero.
      delay = 0.0;
      total_of_delays += delay;

      // Increment the number of customers delayed, and make server busy
      ++ num_custs_delayed;
      server_status = BUSY;

      // Schedule a departure (servicce completion)
      time_next_event[2] = time + expon(mean_service);
   }
}

void depart(void)
{
   int i;
   float delay;

   // Check to see whether the queue is empty
   if( num_in_q == 0) {
      // The queue is empty so make the server idle and eliminate the departure event from consideration
      server_status = IDLE;
      time_next_event[2] = 1.0e+30;
   }
   else {
      // The queue is nonempty, so decrement the number of customers in queue
      -- num_in_q;

      // compute the delay of the customer delayed, and schedule departure
      ++ num_custs_delayed;
      time_next_event[2] = time + expon(mean_service);

      // Move each customer in queue (if any) up one place
      for (i = 1; i <= num_in_q; ++i)
         time_arrival[i] = time_arrival[i + 1];
   }
}

void report(void)
{
   //  Compute and write estimates of desired measures of performance
   fprintf(outfile, "\n\nAverage delay in queue % 11.3f minutes\n\n", total_of_delays / num_custs_delayed);
   fprintf(outfile, "Average number in queue %10.3f\n\n", area_num_in_q / time);
   fprintf(outfile, "Server utilization % 15.3f\n\n", area_server_status / time);
   fprintf(outfile, "Time simulation ended %12.3f", time);
}

void update_time_avg_stats(void)
{
   float time_since_last_event;

   // Compute time since last event, and update last-event-time marker
   time_since_last_event = time - time_last_event;
   time_last_event = time;

   // Update area under number-in-queue function
   area_num_in_q += num_in_q * time_since_last_event;
   // Update are aunder server-busy indicator function
   area_server_status += server_status * time_since_last_event;
}