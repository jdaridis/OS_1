#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include "shm_data.h"
// #include "bus.h"

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

    while(--argc){ 
		if(strcmp(*argv, "-l") == 0){
			fp = fopen(*(argv + 1), "r");
			if(fp == NULL)
				fprintf(stdout, "Could not find file %s\n", *(argv + 1));
		} 
		
		argv++;
	}

    // TODO: Change config file to correct attributes;
    while(!feof(fp)){
        fscanf(fp, "Capacity: %d ", &station_capacity);
        fscanf(fp,"Serve: %d ", &max_buses);
        for(int i = 0;i<3;i++){
            fscanf(fp,"Bay %*d: %d ", &bay_cap[i]);
        }
        fscanf(fp,"Bus Capacity: %d ", &bus_capacity);
        fscanf(fp, "Max park time: %d ", &max_park);
    }
    printf("Bus capacity %d Station capacity %d\n", bus_capacity, station_capacity);

    shm_id = shmget(IPC_PRIVATE, sizeof(shm_data) + station_capacity*sizeof(shm_bus), 0666);

    printf("Shm id is %d\n", shm_id);

    data = shmat(shm_id, 0, 0);
    
    initialize_shm(data, station_capacity, max_buses, bay_cap);

    sprintf(shm_id_text, "%d", shm_id);

    pid = fork();

    if(pid == 0){
        execlp("./station-manager", "station-manager", "-s", shm_id_text, (char*)NULL);
    }

    printf("Generate random buses or enter manually?\n");
    ans = getchar();
    if(ans == 'y'){
        for(int i =0;i<3;i++){
            sprintf(bus_people, "%d", rand()%bus_capacity);
            sprintf(bus_capacity_text, "%d", bus_capacity);
            sprintf(bus_parktime, "%d", rand()%10 + 1);
            sprintf(bus_mantime, "%d", rand()%5 + 1);
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

    while(wait(NULL) > 0);

    shmdt(data);

    shmctl(shm_id, IPC_RMID, 0);

    

    return 0;
}
