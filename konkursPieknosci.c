#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#define ROOT 0
#define ACK_LEK 100
#define REQ_LEK 101
#define ACK_SAL 102
#define REQ_SAL 103

struct msg_lek{
int rec_id;
int clock_rec;
};

struct msg_sal{
int rec_id;
int clock_rec;
int m_rec;
};

// TODO
void recv_all(int *clock_vec, int size, int my_tid, int* shift);

//TODO
void send_req_lek(struct msg_lek* kollek, struct msg_lek msg, int* clock_lek);

int get_nr(struct msg_lek* kollek, int size);

int main(int argc, char **argv)
{
	int tid,size;
	MPI_Status status;
	time_t tt;
	srand(time(&tt));
	MPI_Init(&argc, &argv);
	int s = atoi(argv[0]); // pojemnosc salonu
	int m = rand() % 1000; //liczba modelek
	int l = atoi(argv[1]); //liczba lekarzy
	int count_req_sal = 0;
	struct msg_sal kolsal[size]; //kolejka do salonu
	int count_req_lek = 0;
	struct msg_lek kollek[size]; // kolejka do lekarzy
	int clock_lek, clock_sal, shift = 0;
	int count_ack_lek = 0;
	bool acklek[size];
	int count_ack_sal = 0;
	bool acksal[size];
	int clock_vec[size];
	for(int i = 0; i < size; i++)
		clock_vec[i] = 0;
	int count_sal  = s;

	//TODO recv_all();

	struct msg_lek msg;
	msg.rec_id = tid;
	msg.clock_rec = clock_vec[tid];
	send_req_lek(kollek, msg, &clock_lek);

	//TODO recv_all();
	int nr; //TODO
	if(count_req_lek + count_ack_lek == size)
		nr = get_nr(&kollek, size, tid);
	else
		nr = INT_MAX;
	while(count_req_lek + count_ack_lek != size && l > nr && shift == 0){
		recv_all();
	}
	
		//MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		//printf(" Otrzymalem %d %d od %d\n", msg[0], msg[1], status.MPI_SOURCE);
		//msg[0] = tid;
		//msg[1] = size;
		//MPI_Send( msg, 2, MPI_INT, ROOT, MSG_TAG, MPI_COMM_WORLD );
		//printf(" Wyslalem %d %d do %d\n", msg[0], msg[1], ROOT );
		

	MPI_Finalize(); // Musi być w każdym programie na końcu
}

void recv_all(int *clock_vec, int size, int my_tid){

	clock_vec[my_tid]++;
}

void send_req_lek(struct msg_lek* kollek, struct msg_lek msg, int* clock_lek){
	kollek[msg.rec_id] = msg;
	*clock_lek = msg.clock_rec;
}

int get_nr(struct msg_lek* kollek, int size, int tid){
	int nr = 0;
	for (int i = 0; i < size; i++){
		if(kolek[i].clock_rec < kollek[tid] || 
				(kollek[i].clock_rec >= kollek[tid] && kollek[i].rec_id < kollek[tid]))
			nr++;
	}
	return nr;
}
