#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shm_data.h"

// Initialise the shared memory segment with the appropriate values.
void initialize_shm(shm_data* data, int station_capacity, int max_buses, int *bay_cap){
    sem_init(&(data->st_mngr), 1, 0);
    sem_init(&(data->full), 1, station_capacity);
    sem_init(&(data->in_queue), 1, 0);
    sem_init(&(data->out_queue), 1, 0);
    sem_init(&(data->bay_full), 1, 0);
    sem_init(&(data->bay_full_queue), 1, 1);
    sem_init(&(data->in_station), 1, 2);
    sem_init(&(data->inform_receive), 1, 0);
    sem_init(&(data->man), 1, 1);
    sem_init(&(data->mutex), 1, 1);
    data->capacity = station_capacity;
    data->current_num = 0;
    data->in_queue_count = 0;
    data->out_queue_count = 0;
    data->total_buses = 0;
    data->max_buses = max_buses;
    data->buses_served = 0;
    data->total_people = 0;
    data->total_people_in = 0;
    data->total_people_out = 0;
    for(int i = 0;i<3;i++){
        data->bays[i].max_num = bay_cap[i];
        data->bays[i].current_num = 0;
        
    }
    intitiaze_bays(data);
    for(int i=0;i<3;i++){
        for(int j = 0;j<data->bays[i].max_num;j++){
            bays[i].buses[j].status = 'F';
        }
    }
}


// Prepare the structure for the bays and the buses with the correct offsets each time.
void intitiaze_bays(shm_data* data){
    for(int i = 0;i<3;i++){
        bays[i].current_num = &(data->bays[i].current_num);
        bays[i].max_num = &(data->bays[i].max_num);
    }

    bays[0].buses = (shm_bus*)(data + 1);
    for(int i=1;i<3;i++){
        bays[i].buses = bays[i - 1].buses + (*(bays[i-1].max_num));
    }
    
   
}