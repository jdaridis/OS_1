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
