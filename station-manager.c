#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shm_data.h"

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

    while(1){
        // printf("Waiting for requests\n");
        sem_wait(&(data->st_mngr));
        sem_wait(&(data->full));
        if(data->out_queue_count > 0){
            printf("Permission to get out requested\n");

            sem_wait(&(data->in_station));
            printf("Permission granted\n");
            sem_wait(&(data->mutex));
            data->out_queue_count--;
            sem_post(&(data->mutex));
            sem_post(&(data->out_queue));
            sem_post(&(data->full));
            data->current_num--;
            continue;
            // Outside...
        };

        if(data->in_queue_count > 0){
            printf("Permission to get in requested\n");
            sem_wait(&(data->in_station));
            printf("Permission granted\n");
            sem_wait(&(data->mutex));
            data->in_queue_count--;
            sem_post(&(data->mutex));
            // sem_wait(&(data->man));
            // for(int i=0;i<3;i++){
            //     if(*(bays[temp_bus.type + i].current_num) + 1 <= *(bays[temp_bus.type + i].max_num)){
            //         memcpy(&(bays[temp_bus.type + i].buses[*(bays[temp_bus.type + i].current_num)]), &temp_bus, sizeof(bus));
            //         (*(bays[temp_bus.type + i].current_num))++;
            //         final_bay = temp_bus.type + i;
            //         break;
            //     }
            // }
            // sem_wait(&(data->park));
            sem_post(&(data->in_queue));
            
            data->current_num++;
            continue;
             // Inside...
        };

    }

    
    
    return 0;
}
