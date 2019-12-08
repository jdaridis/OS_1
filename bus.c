#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <unistd.h>
#include "shm_data.h"
#include "bus.h"


int main(int argc, char const *argv[]){

    int shm_id;
    shm_data* data;
    int id;
    bus temp_bus;
    int error;
    int i;
    int final_bay;
    while(--argc){
        if(strcmp(*argv, "-t") == 0){
			if(strcmp(*(argv + 1), "VOR") == 0){
                temp_bus.type = 0;
            } else if(strcmp(*(argv + 1), "ASK") == 0){
                temp_bus.type = 1;
            } else {
                temp_bus.type = 2;
            }
            printf("TYPE %d \n", temp_bus.type);
		} else if(strcmp(*argv, "-n") == 0){
			temp_bus.people = atoi(*(argv + 1));
		} else if(strcmp(*argv, "-c") == 0){
			temp_bus.capacity = atoi(*(argv + 1));
		} else if(strcmp(*argv, "-p") == 0){
			temp_bus.parkperiod = atoi(*(argv + 1));
		} else if(strcmp(*argv, "-m") == 0){
			temp_bus.mantime = atoi(*(argv + 1));
		} else if(strcmp(*argv, "-s") == 0){
			shm_id = atoi(*(argv + 1));
		} 
		
		argv++;
    }

    data = shmat(shm_id, (void*)0, 0);
    intitiaze_bays(data);

    sem_wait(&(data->mutex));
    data->total_buses++;
    id = data->total_buses;
    sem_post(&(data->mutex));

    printf("Requesting in. Bus id: %d\n", id);
    sem_post(&(data->st_mngr));
    // printf("Notified station mngr. Bus id: %d\n", id);

    sem_wait(&(data->mutex));
    data->in_queue_count++;
    sem_post(&(data->mutex));

    sem_wait(&(data->in_queue));

    printf("Getting in station. Bus id: %d\n", id);
    sem_wait(&(data->man));

    printf("Maneuvering in station. Bus id: %d\n", id);
    sleep((temp_bus.mantime));
    sem_post(&(data->man));

    for(i = 0;i<3;i++){
        final_bay = (temp_bus.type + i)%3;
        if(*(bays[final_bay].current_num) + 1 <= *(bays[final_bay].max_num)){
            memcpy(&(bays[final_bay].buses[*(bays[final_bay].current_num)]), &temp_bus, sizeof(bus));
            (*(bays[final_bay].current_num))++;
            
            break;
        }
    }


    printf("Parked. Bus id: %d\n", id);
    sem_post(&(data->in_station));
    // Inside...
    sleep((temp_bus.parkperiod));

    printf("Requesting out. Bus id: %d\n", id);
    sem_wait(&(data->mutex));
    printf("Final bay %d current num %d\n", final_bay, *(bays[final_bay].current_num));
    (*(bays[final_bay].current_num))--;
    printf("Final bay %d current num %d\n", final_bay, *(bays[final_bay].current_num));
    data->out_queue_count++;
    sem_post(&(data->mutex));

    sem_post(&(data->st_mngr));
    sem_post(&(data->full));
    
    sem_wait(&(data->out_queue));
    sem_post(&(data->in_station));
    printf("Got out safely. Bus id: %d\n", id);




    /* code */
    return 0;
}
