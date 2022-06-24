#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "shm_data.h"

void bay_policy(shm_data* data);
void serve_out_queue(shm_data* data);
void serve_in_queue(shm_data* data);

int main(int argc, char const *argv[]){
    
    int shm_id;
    shm_data* data;

    while(--argc){ 
        if(strcmp(*argv, "-s") == 0){
			shm_id = atoi(*(argv + 1));
		} 
		
		argv++;
    }

    // Attach to the shared memory segment.
    data = shmat(shm_id, 0, 0);
    
    // Fix the offsets for the auxiliary array bays.
    intitiaze_bays(data);

    // Station manager operations until we serve all the buses for the day.
    while(data->buses_served < data->max_buses){
        sem_wait(&(data->st_mngr)); //Make sure you have something to do.
        sem_wait(&(data->bay_full_queue)); //Wait until there is no blocked bay.
        sem_wait(&(data->full)); //Wait until the station isn't full.
        sem_wait(&(data->in_station)); //Make sure only 2 buses are moving in the station.
        
        // Serve the outgoing buses first.
        if(data->out_queue_count > 0){
            serve_out_queue(data);
            continue;
        };

        // Serve the incoming buses.
        if(data->in_queue_count > 0){
            serve_in_queue(data);
            continue;
        };

    }

    return EXIT_SUCCESS;
}

void serve_out_queue(shm_data* data){
    int out_bay, out_spot, out_id;
    printf("Permission to get out granted\n");

    sem_wait(&(data->mutex));
    data->out_queue_count--;
    sem_post(&(data->mutex));
    
    sem_post(&(data->out_queue)); //Let the bus out.
    printf("Id to check you out\n");

    // Wait for the bus to show its identity (id, destination).
    sem_wait(&(data->inform_receive));
    out_bay = data->out_bus.final_bay;
    out_spot = data->out_bus.spot;
    out_id = data->out_bus.id;
    bays[out_bay].buses[out_spot].leave_time = time(NULL);
    bays[out_bay].buses[out_spot].status = 'F';
    printf("Checked out bus %d from bay: %d with passengers %d\n", out_id, out_bay, data->out_bus.passengers);

    // When a bus has left the station notify about the bay it freed;
    sem_wait(&(data->mutex));
    bay_policy(data);
    sem_post(&(data->mutex));
   
    sem_post(&(data->full));
    data->current_num--;
    data->buses_served++;
}

void serve_in_queue(shm_data* data){
    
    printf("Permission to get in granted\n");
    
    sem_wait(&(data->mutex));
    data->in_queue_count--;
    sem_post(&(data->in_queue)); //Let the bus in.
    printf("Your id please\n");
    sem_post(&(data->mutex));

    // Wait for the bus to show its identity (id, destination).
    sem_wait(&(data->inform_receive));
    data->in_bus.status = 'A';
    printf("Checked in bus %d\n", data->in_bus.id);

    // Find the appropriate bay.
    sem_wait(&(data->mutex));
    bay_policy(data);
    sem_post(&(data->mutex));
    

    data->current_num++;
}

// Find the appropriate bay for the incoming bus, if one has room.
void bay_policy(shm_data* data){
    int i;
    int final_bay = 2;
    int spot = -1;
    final_bay = (data->in_bus.type);

    for(i=0;i<2;i++){
        if(*(bays[final_bay].current_num) < *(bays[final_bay].max_num)){
            for(int j=0;j<*(bays[final_bay].max_num);j++){

                if(bays[final_bay].buses[j].status == 'F'){
                    spot = j;
                    break;
                }
            }

            if(spot == -1){
                final_bay = 2;
                continue;
            }
            data->in_bus.final_bay = final_bay;
            data->in_bus.spot = spot;
            
            sem_post(&(data->bay_full));
            (*(bays[final_bay].current_num))++;
            break;
        }
        final_bay = 2;
    }
    if(i == 2){
        data->in_bus.final_bay = data->in_bus.type;
    }
}
