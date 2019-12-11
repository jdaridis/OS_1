#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shm_data.h"
// #include "bus.h"

void initialize_shm(shm_data* data, int station_capacity, int max_buses, int *bay_cap){
    sem_init(&(data->st_mngr), 1, 0);
    // sem_init(&(data->st_mngr_out), 1, 0);
    sem_init(&(data->full), 1, station_capacity);
    sem_init(&(data->in_queue), 1, 0);
    sem_init(&(data->out_queue), 1, 0);
    sem_init(&(data->in_station), 1, 2);
    sem_init(&(data->inform_read), 1, 0);
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
    // TODO Fix bays with real values.
    for(int i = 0;i<3;i++){
        data->bays[i].max_num = bay_cap[i];
        data->bays[i].current_num = 0;
        // data->bays[i].buses = (bus*)(data + 1) + i*(station_capacity/3);
    }
    
}

void intitiaze_bays(shm_data* data){
    for(int i = 0;i<3;i++){
        bays[i].current_num = &(data->bays[i].current_num);
        bays[i].max_num = &(data->bays[i].max_num);
    }

    bays[0].buses = (shm_bus*)(data + 1);

    for(int i=1;i<3;i++){
        bays[i].buses = (shm_bus*)(data + 1) + (*(bays[i-1].max_num)) * i;
    }
}