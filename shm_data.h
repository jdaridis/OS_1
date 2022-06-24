#include <sys/shm.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/types.h>

typedef struct shm_bus{
    int id;
    int type;
    long arrival_time;
    long leave_time;
    long park_time;
    int passengers;
    int final_bay;
    int spot;
    char status;
} shm_bus;

typedef struct Bay{
    int* current_num;
    int* max_num;
    shm_bus* buses;
} Bay;

Bay bays[3]; //Array to get info about the bays more easily.

typedef struct shm_bay{
    int current_num;
    int max_num;
} shm_bay;

typedef struct shm_data{
    sem_t st_mngr; //To notify the station manager to do some job.
    sem_t full;   //For the buses to wait outside the station if its full.
    sem_t in_queue; //The queue in which each incoming bus is blocked unitl its turn.
    sem_t out_queue; //The queue in which each outgoing bus is blocked unitl its turn.
    sem_t man; //Only one bus is allowed to perform a maneuver.
    sem_t in_station; //Only 2 buses are allowed in the station at each time, one waiting and one maneuvering.
    sem_t inform_receive; // Wait for another process to communicate some information.
    sem_t bay_full; //Block until there is a bay available.
    sem_t bay_full_queue; //If a bus is waiting for an available bay, all the other buses wait too.
    sem_t mutex; //Only one process can be at a critical section at a time.
    shm_bay bays[3]; //Info about the bays.
    shm_bus in_bus; //Info about the incoming bus.
    shm_bus out_bus; //Info about the outgoing bus.
    int capacity;
    int in_queue_count;
    int out_queue_count;
    int current_num;
    int total_buses;
    int total_people;
    int total_people_in;
    int total_people_out;
    int max_buses;
    int buses_served;
} shm_data;


void initialize_shm(shm_data* data, int station_capacity, int max_buses, int *bay_cap);
void intitiaze_bays(shm_data* data);