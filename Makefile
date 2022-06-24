all: mystation station-manager bus comptroller

mystation: mystation.c shm_data.o
	gcc -g -o mystation mystation.c shm_data.o -lpthread

station-manager: station-manager.c shm_data.o
	gcc -g -o station-manager station-manager.c shm_data.o -lpthread

bus: bus.c bus.h shm_data.o
	gcc -g -o bus bus.c shm_data.o -lpthread

shm_data.o: shm_data.c shm_data.h
	gcc -g -c shm_data.c

comptroller: comptroller.c shm_data.o
	gcc -g -o comptroller comptroller.c shm_data.o -lpthread

clean: 
	rm -f ./mystation ./station-manager ./bus ./comptroller shm_data.o 
