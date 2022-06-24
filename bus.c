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
    int log = 0;

    FILE* fp;
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
		} else if(strcmp(*argv, "-log") == 0){
			log = 1;
		} 
		
		argv++;
    }

    if(log){
        fp = fopen("dump.txt", "a");
    } else {
        fp = stdout;
    }

    // Attach to the shared memory segment.
    data = shmat(shm_id, (void*)0, 0);

    // Fix the offsets for the auxiliary array bays.
    intitiaze_bays(data);

    arrive_at_station(data, &temp_bus, fp);

    // Wait in line until the station manager lets you in.
    sem_wait(&(data->in_queue));

    get_in_station(data, temp_bus, fp);
    
    // Wait for the station manager to tell you the bay to go.
    sem_wait(&(data->bay_full));

    proceed_to_bay(data, &final_bay, &spot, fp);

    maneuver_in(data, temp_bus, fp);

    park(data, temp_bus, final_bay, spot, fp);

    maneuver_out(data, temp_bus, fp);

    request_out(data, final_bay, spot, temp_bus, fp);
    
    // Wait in line until the station manager lets you out.
    sem_wait(&(data->out_queue));

    get_out_of_station(data, final_bay, spot, temp_bus, fp);

    return EXIT_SUCCESS;
}

// Arrive at station and notify the station manager for the arrival.
void arrive_at_station(shm_data* data, bus* temp_bus, FILE* fp){
    int id;
    sem_wait(&(data->mutex));
    data->total_buses++;
    id = data->total_buses;
    temp_bus->id = id;
    data->in_queue_count++;
    temp_bus->arrival_time = time(NULL);
    fprintf(fp,"Requesting in. Bus id: %d type: %d\n", id, temp_bus->type);
    sem_post(&(data->mutex));
    
    sem_post(&(data->st_mngr));
}

// After the manager lets you in show your id and destination.
void get_in_station(shm_data* data, bus temp_bus, FILE* fp){
    int id = temp_bus.id;
    fprintf(fp,"Getting in station. Bus id: %d type: %d\n", id, temp_bus.type);
    srand(time(NULL));
    sem_wait(&(data->mutex));
    fprintf(fp,"Here is my id. Bus id :%d\n", id);
    data->in_bus.id = temp_bus.id;
    data->in_bus.type = temp_bus.type;
    data->in_bus.arrival_time = temp_bus.arrival_time;
    data->in_bus.passengers = temp_bus.people;
    
    sem_post(&(data->inform_receive));
    sem_post(&(data->mutex));
}

// Proceed to the bay specified by the manager.
void proceed_to_bay(shm_data* data, int* final_bay, int* spot, FILE* fp){
    sem_wait(&(data->mutex));
    *final_bay = data->in_bus.final_bay;
    *spot = data->in_bus.spot;
    data->in_bus.park_time = time(NULL);
    data->in_bus.status = 'M';
    memcpy(&(bays[*final_bay].buses[*spot]), &(data->in_bus), sizeof(shm_bus));
    sem_post(&(data->mutex));

    fprintf(fp,"Can proceed to bay %d\n", *final_bay);
    sem_post(&(data->bay_full_queue));
}

// Perform the maneuver to get in.
void maneuver_in(shm_data* data, bus temp_bus, FILE* fp){
    sem_wait(&(data->man));
    fprintf(fp,"Maneuvering for parking. Bus id: %d\n", temp_bus.id);
    sleep((temp_bus.mantime));
    sem_post(&(data->man));

}

// Park the bus and disembark the passengers.
void park(shm_data* data, bus temp_bus, int final_bay,int spot,FILE* fp){
    fprintf(fp,"Parked in %d. Bus id: %d\n",final_bay, temp_bus.id);
    sem_post(&(data->in_station));
    
    sem_wait(&(data->mutex));
    data->total_people += temp_bus.people;
    data->total_people_in += temp_bus.people;
    bays[final_bay].buses[spot].status = 'P';
    sem_post(&(data->mutex));
    sleep((temp_bus.parkperiod));
}

// Perform the maneuver to get out.
void maneuver_out(shm_data* data, bus temp_bus, FILE* fp){
    sem_wait(&(data->man));

    fprintf(fp,"Maneuvering to get out. Bus id: %d\n", temp_bus.id);
    sleep((temp_bus.mantime));
    sem_post(&(data->man));

}

// Request from the station manager to get out.
void request_out(shm_data* data, int final_bay, int spot, bus temp_bus, FILE* fp){
    fprintf(fp,"Requesting out. Bus id: %d\n", temp_bus.id);
    sem_wait(&(data->mutex));
    if(data->total_people != 0){
        bays[final_bay].buses[spot].passengers = (rand()%temp_bus.capacity)%data->total_people + 1;
    } else {
        bays[final_bay].buses[spot].passengers = 0;
    }
    
    data->total_people -= bays[final_bay].buses[spot].passengers;
    data->total_people_out += bays[final_bay].buses[spot].passengers;
    (*(bays[final_bay].current_num))--;
    data->out_queue_count++;
    sem_post(&(data->mutex));

    sem_post(&(data->st_mngr));
    sem_post(&(data->full));

    sem_post(&(data->bay_full_queue));
}

// Leave your id at the front door and exit the station.
void get_out_of_station(shm_data* data, int final_bay,int  spot, bus temp_bus, FILE* fp){
    sem_wait(&(data->mutex));
    bays[final_bay].buses[spot].status = 'F';
    memcpy(&(data->out_bus), &(bays[final_bay].buses[spot]), sizeof(shm_bus));
    sem_post(&(data->inform_receive));
    sem_post(&(data->mutex));
    
   
    fprintf(fp,"Got out safely. Bus id: %d\n", temp_bus.id);
    sem_post(&(data->in_station));
}






    


 
