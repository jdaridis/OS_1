#include <sys/shm.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/types.h>
// #include "bus.h"

typedef struct shm_bus{
    int id;
    int type;
    long arrival_time;
    long leave_time;
    int final_bay;
    int spot;
    char status;
} shm_bus;

typedef struct Bay{
    int* current_num;
    int* max_num;
    shm_bus* buses;
} Bay;

Bay bays[3];

typedef struct shm_bay{
    int current_num;
    int max_num;
} shm_bay;

typedef struct shm_data{
    sem_t st_mngr;
    sem_t full;
    sem_t in_queue;
    sem_t out_queue;
    sem_t man;
    sem_t in_station;
    sem_t inform_read;
    sem_t inform_receive;
    sem_t comptroller;
    sem_t mutex;
    shm_bay bays[3];
    shm_bus curr_bus;
    int capacity;
    int in_queue_count;
    int out_queue_count;
    int current_num;
    int total_buses;
    int max_buses;
    int buses_served;
} shm_data;


void initialize_shm(shm_data* data, int station_capacity, int max_buses, int *bay_cap);
void intitiaze_bays(shm_data* data);