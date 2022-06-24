enum bus_types{VOR,ASK,PEL};

typedef struct bus{
    int id;
    int type;
    int people;
    int capacity;
    int parkperiod;
    int mantime;
    long arrival_time;
} bus;

void arrive_at_station(shm_data* data, bus* temp_bus, FILE* fp);
void get_in_station(shm_data* data, bus temp_bus, FILE* fp);
void proceed_to_bay(shm_data* data, int* final_bay, int* spot, FILE* fp);
void maneuver_in(shm_data* data, bus temp_bus, FILE* fp);
void park(shm_data* data, bus temp_bus, int final_bay,int spot,FILE* fp);
void maneuver_out(shm_data* data, bus temp_bus, FILE* fp);
void request_out(shm_data* data, int final_bay, int spot, bus temp_bus, FILE* fp);

void  get_out_of_station(shm_data* data, int final_bay,int  spot, bus temp_bus, FILE* fp);
