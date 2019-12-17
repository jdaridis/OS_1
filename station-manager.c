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
        sem_wait(&(data->st_mngr));
        sem_wait(&(data->bay_full_queue));
        sem_wait(&(data->full));
        sem_wait(&(data->in_station));
        
        if(data->out_queue_count > 0){
            serve_out_queue(data);
            continue;
        };

        if(data->in_queue_count > 0){
            serve_in_queue(data);
            continue;
        };

    }

    return 0;
}

void serve_out_queue(shm_data* data){
    int out_bay, out_spot, out_id;
    printf("Permission to get out granted\n");

    sem_wait(&(data->mutex));
    data->out_queue_count--;
    sem_post(&(data->mutex));
    
    sem_post(&(data->out_queue));
    printf("Id to check you out\n");
    sem_wait(&(data->inform_receive));
    out_bay = data->out_bus.final_bay;
    out_spot = data->out_bus.spot;
    out_id = data->out_bus.id;
    bays[out_bay].buses[out_spot].leave_time = time(NULL);
    bays[out_bay].buses[out_spot].status = 'E';
    printf("Checked out bus %d from bay: %d\n", out_id, out_bay);

    if(data->blocked_bay){
        sem_wait(&(data->mutex));
        bay_policy(data);
        data->blocked_bay = 0;
        sem_post(&(data->mutex));

    }
   
    sem_post(&(data->full));
    data->current_num--;
    data->buses_served++;
}

void serve_in_queue(shm_data* data){
    
    printf("Permission to get in granted\n");
    
    sem_wait(&(data->mutex));
    data->in_queue_count--;
    sem_post(&(data->in_queue));
    printf("Your id please\n");
    sem_post(&(data->mutex));

    sem_wait(&(data->inform_receive));
    data->in_bus.status = 'A';
    printf("Checked in bus %d\n", data->in_bus.id);
    sem_wait(&(data->mutex));
    bay_policy(data);
    sem_post(&(data->mutex));
    

    data->current_num++;
}

void bay_policy(shm_data* data){
    int i;
    int final_bay;
    int spot;
    final_bay = (data->in_bus.type);
    for(i=0;i<2;i++){
        if(*(bays[final_bay].current_num) < *(bays[final_bay].max_num)){
            data->in_bus.final_bay = final_bay;
            for(int j=0;j<*(bays[final_bay].max_num);j++){
                if(bays[final_bay].buses[j].status == 'F' || bays[final_bay].buses[j].status == 'E'){
                    spot = j;
                    bays[final_bay].buses[j].status = 'W';
                    break;
                }
            }
            
            data->in_bus.spot = spot;
            
            sem_post(&(data->bay_full));

            break;
        }
        final_bay = 2;
    }
    if(i == 2){
        data->blocked_bay = 1;
        data->in_bus.final_bay = data->in_bus.type;
    }
}
