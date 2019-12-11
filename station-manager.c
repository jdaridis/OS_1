#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "shm_data.h"

int main(int argc, char const *argv[]){
    
    int shm_id;
    shm_data* data;
    int error;
    int final_bay;
    // shm_bus temp_bus;
    int spot;

    while(--argc){ 
        if(strcmp(*argv, "-s") == 0){
			shm_id = atoi(*(argv + 1));
		} 
		
		argv++;
    }

    data = shmat(shm_id, 0, 0);
    intitiaze_bays(data);

    while(data->buses_served < data->max_buses){
        // printf("Waiting for requests\n");
        sem_wait(&(data->st_mngr));
        sem_wait(&(data->full));
        if(data->out_queue_count > 0){

            sem_wait(&(data->in_station));
            printf("Permission to get out granted\n");

            sem_wait(&(data->mutex));
            data->out_queue_count--;
            sem_post(&(data->out_queue));
            sem_post(&(data->mutex));
            
            sem_wait(&(data->inform_receive));
            data->curr_bus.leave_time = time(NULL);
            printf("Checked out bus %d\n", data->curr_bus.id);
            sem_post(&(data->inform_read));

            sem_post(&(data->full));
            data->current_num--;
            data->buses_served++;
            continue;
            // Outside...
        };

        if(data->in_queue_count > 0){

            sem_wait(&(data->in_station));
            printf("Permission to get in granted\n");
            
            sem_wait(&(data->mutex));
            data->in_queue_count--;
            sem_post(&(data->in_queue));
            printf("Your id please\n");
            sem_post(&(data->mutex));
            
            
            sem_wait(&(data->inform_receive));
            data->curr_bus.arrival_time = time(NULL);
            data->curr_bus.status = 'A';
            printf("Checked in bus %d\n", data->curr_bus.id);
            sem_post(&(data->inform_read));

            
            for(int i = 0;i<3;i++){
                final_bay = (data->curr_bus.type + i)%3;
                if(*(bays[final_bay].current_num) + 1 <= *(bays[final_bay].max_num)){
                    data->curr_bus.final_bay = final_bay;
                    spot = *(bays[final_bay].current_num);
                    
                    data->curr_bus.spot = spot;
                    memcpy(&(bays[final_bay].buses[spot]), &(data->curr_bus), sizeof(shm_bus));
                    (*(bays[final_bay].current_num))++;
                    break;
                }
            }
            sem_post(&(data->inform_receive));

            data->current_num++;
            
            continue;
             // Inside...
        };

    }

    
    
    return 0;
}
