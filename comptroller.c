#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "shm_data.h"

void comptroller_snapshot(FILE* fp,shm_data* data);
void comptroller_stat(FILE* fp, shm_data* data);

int main(int argc, char const *argv[]){
    int time;
    int stat_time;
    int shm_id;
    int total_time;
    shm_data* data;
    FILE* fp;
    while(--argc){ 
        if(strcmp(*argv, "-s") == 0){
			shm_id = atoi(*(argv + 1));
		} else if(strcmp(*argv, "-d") == 0) {
            time = atoi(*(argv + 1));
        } else if(strcmp(*argv, "-t") == 0) {
            stat_time = atoi(*(argv + 1));
        }
		
		argv++;
    }

    total_time = 0;

    fp = fopen("log.txt", "w");

    data = (shm_data*)shmat(shm_id, 0, 0);

    // Fix the offsets for the auxiliary array bays.
    intitiaze_bays(data);

    fprintf(fp, "Bus status: M(Maneuver) P(Parked) F(Free)\nTime in mins\n");
    
    while(data->buses_served < data->max_buses){
        sleep(time);
        fprintf(fp, "-------Station Snapshot-------\n");
        sem_wait(&(data->mutex));
        comptroller_snapshot(fp, data);
        sem_post(&(data->mutex));

        sleep(stat_time);
        fprintf(fp, "-------Station Statistics-------\n");
        sem_wait(&(data->mutex));
        comptroller_stat(fp, data);
        sem_post(&(data->mutex));
        
        fprintf(fp, "\n");
    }

    fclose(fp);
    shmdt(data);
    
    return 0;
}

void comptroller_snapshot(FILE* fp,shm_data* data){
    int parked_buses = 0;
    for(int i=0;i<3;i++){
        fprintf(fp, "Bay ");

        switch (i) {
        case 0:
            fprintf(fp, "VOR\n");
            break;
        case 1:
            fprintf(fp, "ASK\n");
            break;
        case 2:
            fprintf(fp, "PEL\n");
            break;
        
        default:
            break;
        }

        for(int j = 0;j<data->bays[i].max_num;j++){
            fprintf(fp, "\tBus %d Status: %c ", j, bays[i].buses[j].status);

            if(bays[i].buses[j].status == 'P'){
                fprintf(fp, "Passengers: %d\n", bays[i].buses[j].passengers);
                parked_buses++;
            } else {
                fprintf(fp, "\n");
            }
        }
        fprintf(fp, "Free spots: ");
        for(int j = 0;j<data->bays[i].max_num;j++){
            if(bays[i].buses[j].status == 'F'){
                fprintf(fp, "%d ", j);
            } 
        }

        fprintf(fp, "\n");
        

    }
    fprintf(fp, "Parked buses: %d\n", parked_buses);
}


void comptroller_stat(FILE* fp, shm_data* data){
    int total_wait_time = 0;
    int wait_time;
    int parked_buses = 0;
    fprintf(fp, "Wait time for.\n");
    for(int i=0;i<3;i++){
        fprintf(fp, "Bay ");

        switch (i) {
        case 0:
            fprintf(fp, "VOR\n");
            break;
        case 1:
            fprintf(fp, "ASK\n");
            break;
        case 2:
            fprintf(fp, "PEL\n");
            break;
        
        default:
            break;
        }

        for(int j = 0;j<data->bays[i].max_num;j++){
            if(bays[i].buses[j].status == 'P'){
                parked_buses++;
                wait_time = bays[i].buses[j].park_time - bays[i].buses[j].arrival_time;
                total_wait_time += wait_time;
                fprintf(fp, "\tBus %d %dm\n", j, wait_time);
            }
        }
        
    }
    if(parked_buses != 0){
        total_wait_time = total_wait_time/parked_buses;
    } else {
        total_wait_time = 0;
    }
    
    fprintf(fp, "Mean wait time: %dm\n", total_wait_time);
    fprintf(fp, "Total buses served until now: %d\n", data->buses_served);
    fprintf(fp, "Total passengers arrived: %d\n", data->total_people_in);
    fprintf(fp, "Total passengers left: %d\n", data->total_people_out);

    return;
}
