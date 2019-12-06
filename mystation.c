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
    FILE* fp;
    int bus_capacity;
    int station_capacity;
    


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
        fscanf(fp, "%d %d", &bus_capacity, &bus_capacity);
    }

    shm_id = shmget(IPC_PRIVATE, sizeof(shm_data), 0666);

    printf("Shm id is %d\n", shm_id);

    data = shmat(shm_id, 0, 0);
    
    sem_init(&(data->st_mngr_in), 1, 0);
    sem_init(&(data->st_mngr_out), 1, 0);
    sem_init(&(data->in_queue), 1, 0);
    sem_init(&(data->out_queue), 1, 0);
    sem_init(&(data->man), 1, 1);
    sem_init(&(data->mutex), 1, 1);
    data->capacity = station_capacity;
    data->current_num = 0;



    sprintf(shm_id_text, "%d", shm_id);

    sleep(30);

    shmdt(data);

    shmctl(shm_id, IPC_RMID, 0);

    

    return 0;
}
