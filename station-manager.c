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
    int error;
    

    while(--argc){ 
        if(strcmp(*argv, "-s") == 0){
			shm_id = atoi(*(argv + 1));
		} 
		
		argv++;
    }

    data = shmat(shm_id, 0, 0);
    intitiaze_bays(data);

    while(data->buses_served < data->max_buses){
        // printf("P st_mngr\n");
        sem_wait(&(data->st_mngr));
        // printf("P bay_full_queue\n");
        sem_wait(&(data->bay_full_queue));
        // printf("P full\n");
        sem_wait(&(data->full));
        // printf("P in_station\n");
        sem_wait(&(data->in_station));
        
        if(data->out_queue_count > 0){
            serve_out_queue(data);
            continue;
            // Outside...
        };

        if(data->in_queue_count > 0){
            serve_in_queue(data);
            continue;
             // Inside...
        };

        

    }

    return 0;
}

void serve_out_queue(shm_data* data){
    // printf("P in_station\n");
    // sem_wait(&(data->in_station));
    printf("Permission to get out granted\n");

    // printf("P mutex\n");
    sem_wait(&(data->mutex));
    data->out_queue_count--;
    // printf("V out_queue\n");
    // printf("V mutex\n");
    sem_post(&(data->mutex));
    
    sem_post(&(data->out_queue));
    // printf("P inform_receive\n");
    printf("Id to check you out\n");
    sem_wait(&(data->inform_receive));
    data->out_bus.leave_time = time(NULL);
    printf("Checked out bus %d from bay: %d\n", data->out_bus.id, data->out_bus.final_bay);
    if(data->out_bus.final_bay == 2 || data->out_bus.final_bay == data->blocked_bay){
        // printf("V bay_full after out\n");
        
        
        sem_post(&(data->bay_full));
        // sem_post(&(data->bay_full_queue));
        if(data->blocked_bay_queue == 0){
            data->blocked_bay = -1;
            
        } 
            
    }
    sem_post(&(data->inform_read));

    sem_post(&(data->full));
    data->current_num--;
    data->buses_served++;
}

void serve_in_queue(shm_data* data){
    // printf("P in_station\n");
    
    // sem_wait(&(data->in_station));
    // printf("P bay_full\n");
    
    
    printf("Permission to get in granted\n");


    
    sem_wait(&(data->mutex));
    data->in_queue_count--;
    sem_post(&(data->in_queue));
    printf("Your id please\n");
    sem_post(&(data->mutex));

    
    sem_wait(&(data->inform_receive));
    data->in_bus.arrival_time = time(NULL);
    data->in_bus.status = 'A';
    printf("Checked in bus %d\n", data->in_bus.id);
    // printf("V inform_read\n");
    sem_post(&(data->inform_read));

    // printf("P mutex\n");
    sem_wait(&(data->mutex));
    bay_policy(data);
    // printf("V mutex\n");
    sem_post(&(data->mutex));
    
    
    // printf("P mutex\n");
   
    
    // printf("V mutex\n");
    

    // printf("V in_queue\n");
    
   
    // printf("P inform_receive\n");
    

    data->current_num++;
}

void bay_policy(shm_data* data){
    int i;
    int final_bay;
    int spot;
    final_bay = (data->in_bus.type);
    printf("Current bus is: %d and bay[%d] cur %d max %d\n", data->in_bus.id, final_bay, *(bays[final_bay].current_num), *(bays[final_bay].max_num));
    for(i=0;i<2;i++){
        if(*(bays[final_bay].current_num) < *(bays[final_bay].max_num)){
            data->in_bus.final_bay = final_bay;
            spot = *(bays[final_bay].current_num);
            data->in_bus.spot = spot;

            
            
            // printf("V bay_full_queue\n");
            
            
            // printf("V bay_full\n");
            sem_post(&(data->bay_full));
            
            data->blocked_bay_queue--;
            if(data->blocked_bay_queue == 0){
                data->blocked_bay = -1;
            } else {
                // sem_post(&(data->bay_full_queue));
            }
                
            
            
            
            
            break;
        }
        final_bay = 2;
    }
    if(i == 2){
        data->blocked_bay = data->in_bus.type;
        data->in_bus.final_bay = data->in_bus.type;
    } /* else {
        sem_post(&(data->bay_full_queue));

    } */
    // printf("V inform_receive\n");
    sem_post(&(data->inform_receive));
}
