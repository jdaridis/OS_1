#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include "shm_data.h"

int main(int argc, char const *argv[]){

    int shm_id;
    pid_t pid;
    shm_data* data;
    char shm_id_text[10];
    char bus_people[10];
    char bus_capacity_text[10];
    char bus_parktime[10];
    char bus_mantime[10];
    char bus_type[4];
    int bay_cap[3];
    FILE* fp;
    int bus_capacity;
    int max_park;
    int station_capacity;
    int initial_buses = 5;
    int max_buses;
    char ans;
    int t;
    int status;
    int snap_time;
    int stat_time;
    char snap_text[10];
    char stat_text[10];


    while(--argc){ 
		if(strcmp(*argv, "-l") == 0){
			fp = fopen(*(argv + 1), "r");
			if(fp == NULL){
				fprintf(stdout, "Could not find file %s\n", *(argv + 1));
                exit(1);
            }
		} 
		
		argv++;
	}

    srand(123);
    
    // Read the config file.
    while(!feof(fp)){
        fscanf(fp, "Capacity: %d ", &station_capacity);
        fscanf(fp,"Serve: %d ", &max_buses);
        for(int i = 0;i<3;i++){
            fscanf(fp,"Bay %*d: %d ", &bay_cap[i]);
        }
        fscanf(fp,"Bus Capacity: %d ", &bus_capacity);
        fscanf(fp, "Max park time: %d ", &max_park);
        fscanf(fp, "Snapshot time: %d ", &snap_time);
        fscanf(fp, "Stats time: %d ", &stat_time);
    }

    // Create the shared memory segment and keep the id.
    shm_id = shmget(IPC_PRIVATE, sizeof(shm_data) + station_capacity*sizeof(shm_bus), 0666);

    //Print the shared memory id, if needed for manual spawining.
    printf("Shm id is %d\n", shm_id);

    // Attach to the shared memory segment.
    data = shmat(shm_id, 0, 0);
    
    initialize_shm(data, station_capacity, max_buses, bay_cap);

    sprintf(shm_id_text, "%d", shm_id);

    pid = fork();

    // Spawn the station manager process.
    if(pid == 0){
        execlp("./station-manager", "station-manager", "-s", shm_id_text, (char*)NULL);
    }

    sprintf(snap_text, "%d", snap_time);
    sprintf(stat_text, "%d", stat_time);

    pid = fork();
    
    // Spawn the comptroller process.
    if(pid == 0){
        execlp("./comptroller", "comptroller","-d", snap_text,"-t", stat_text, "-s", shm_id_text, (char*)NULL);
    }

    printf("Generate random buses or enter manually?(y/n)\n");
    ans = getchar();
    if(ans == 'y'){
        for(int i =0;i<max_buses;i++){
            sprintf(bus_people, "%d", rand()%bus_capacity);
            sprintf(bus_capacity_text, "%d", bus_capacity);
            sprintf(bus_parktime, "%d", rand()%max_park + 1);
            sprintf(bus_mantime, "%d", rand()%2 + 1);
            t = rand()%3;
            switch(t){
                case 0:
                    strcpy(bus_type, "VOR");
                    break;
                case 1:
                    strcpy(bus_type, "ASK");
                    break;
                case 2:
                    strcpy(bus_type, "PEL");
                    break;   
            }
            pid = fork();
            if(pid == 0){
                execlp("./bus", "bus","-t", bus_type,"-n", bus_people, "-c", bus_capacity_text, "-p", bus_parktime, "-m", bus_mantime, "-s", shm_id_text, NULL);
            }

        }
    }

    // Wait all the children to finish and catch any occuring error.
    while(wait(&status) > 0){
        if(status != EXIT_SUCCESS){
            fprintf(stderr, "Error code: %d\n", status);
            break;
        }
    }

    // Detach from the shared memory.
    shmdt(data);

    // Destroy the shared memory segment.
    shmctl(shm_id, IPC_RMID, 0);

    return EXIT_SUCCESS;
}
