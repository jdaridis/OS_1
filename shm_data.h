#include <sys/shm.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/types.h>
typedef struct bus bus;

typedef struct bay{
    int* current_num;
    int* max_num;
    bus* buses;
} bay;

bay bays[3];

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
    sem_t mutex;
    shm_bay bays[3];
    int final_bay;
    int in_queue_count;
    int out_queue_count;
    int capacity;
    int current_num;
    int total_buses;
} shm_data;


void initialize_shm(shm_data* data, int station_capacity/*In the future bay capacities*/);
void intitiaze_bays(shm_data* data);