#include <sys/shm.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/types.h>
typedef struct bus bus;

typedef struct bay{
    int current_num;
    int max_num;
    bus* buses;
} bay;

typedef struct shm_data{
    sem_t st_mngr_in;
    sem_t st_mngr_out;
    sem_t in_queue;
    sem_t out_queue;
    sem_t man;
    sem_t mutex;
    bay bays[3];
    int capacity;
    int current_num;
} shm_data;