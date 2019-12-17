#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <unistd.h>
#include <time.h>
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
    int spot;
    while(--argc){
        if(strcmp(*argv, "-t") == 0){
			if(strcmp(*(argv + 1), "VOR") == 0){
                temp_bus.type = 0;
            } else if(strcmp(*(argv + 1), "ASK") == 0){
                temp_bus.type = 1;
            } else {
                temp_bus.type = 2;
            }
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
    temp_bus.id = id;
    data->in_queue_count++;
    temp_bus.arrival_time = time(NULL);
    printf("Requesting in. Bus id: %d type: %d\n", id, temp_bus.type);
    sem_post(&(data->mutex));
    
    sem_post(&(data->st_mngr));
    // printf("Notified station mngr. Bus id: %d\n", id);

    sem_wait(&(data->in_queue));
    printf("Getting in station. Bus id: %d\n", id);

    sem_wait(&(data->mutex));
    printf("Here is my id. Bus id :%d\n", id);
    data->in_bus.id = temp_bus.id;
    data->in_bus.type = temp_bus.type;
    data->in_bus.arrival_time = temp_bus.arrival_time;
    
    sem_post(&(data->inform_receive));
    sem_post(&(data->mutex));
    
    sem_wait(&(data->bay_full));

    sem_wait(&(data->mutex));
    final_bay = data->in_bus.final_bay;
    spot = data->in_bus.spot;
    sem_post(&(data->mutex));
    
    printf("Can proceed to bay %d\n", final_bay);
    data->in_bus.park_time = time(NULL);
    data->in_bus.status = 'M';
    (*(bays[final_bay].current_num))++;
    memcpy(&(bays[final_bay].buses[spot]), &(data->in_bus), sizeof(shm_bus));
    sem_post(&(data->bay_full_queue));

    sem_wait(&(data->man));
    printf("Maneuvering for parking. Bus id: %d\n", id);
    sleep((temp_bus.mantime));
    sem_post(&(data->man));

    printf("Parked in %d. Bus id: %d\n",final_bay, id);
    sem_post(&(data->in_station));
    
    bays[final_bay].buses[spot].status = 'P';
    sleep((temp_bus.parkperiod));

    sem_wait(&(data->man));

    printf("Maneuvering to get out. Bus id: %d\n", id);
    sleep((temp_bus.mantime));
    sem_post(&(data->man));

    printf("Requesting out. Bus id: %d\n", id);
    sem_wait(&(data->mutex));
    (*(bays[final_bay].current_num))--;
    data->out_queue_count++;
    sem_post(&(data->mutex));

    sem_post(&(data->st_mngr));
    sem_post(&(data->full));

    sem_post(&(data->bay_full_queue));
    
    
    sem_wait(&(data->out_queue));
    //Different memcpy
    sem_wait(&(data->mutex));
    printf("My out id is: %d\n", bays[final_bay].buses[spot].id);
    memcpy(&(data->out_bus), &(bays[final_bay].buses[spot]), sizeof(shm_bus));
    sem_post(&(data->inform_receive));
    sem_post(&(data->mutex));
    
   
    printf("Got out safely. Bus id: %d\n", id);
    sem_post(&(data->in_station));

    return 0;
}
