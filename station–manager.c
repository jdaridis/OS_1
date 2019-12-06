#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shm_data.h"

int main(int argc, char const *argv[]){
    
    int shm_id;
    shm_data* data;


     while(--argc){ 
		if(strcmp(*argv, "-s") == 0){
			shm_id = atoi(*argv + 1);
		} 
		
		argv++;
	}

    data = shmat(shm_id, 0, 0);

    while(1){
        printf("Waiting for someone to come in\n");
        sem_wait(&(data->st_mngr_in));
        printf("Permission to get in requested\n");
        while(data->current_num < data->capacity){
            printf("Permission granted\n");
            sem_post(&(data->in_queue));
            sem_post(&(data->man));
            // Inside...
        }
        printf("Waiting for someone to get out\n");
        sem_wait(&(data->st_mngr_out));
        printf("Permission to get out requested\n");
        printf("Permission granted\n");
        sem_post(&(data->out_queue));
        // Outside...

    }

    
    
    return 0;
}
