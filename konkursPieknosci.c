#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include "structures.h"
#define ROOT 0
#define ACK_LEK 100
#define REQ_LEK 101
#define ACK_SAL 102
#define REQ_SAL 103

// TODO
int recv_all(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_lek_type, lekstruct* lek, salstruct* sal);

void send_req_lek(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_lek_type, lekstruct* lek);

void send_ack_lek(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_lek_type, lekstruct* lek);

int get_nr(msg_lek_type* kollek, int size, int tid);

int main(int argc, char **argv)
{
	int tid,size;
	MPI_Status status;
	time_t tt;
	srand(time(&tt));
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &tid);
	int s = atoi(argv[0]); // pojemnosc salonu
	int m = rand() % 1000; //liczba modelek
	int l = atoi(argv[1]); //liczba lekarzy
	int clock_vec[size];
	lekstruct lek = initLekStruct(size);
	salstruct sal = initSalStruct(size);
	for(int i = 0; i < size; i++)
		clock_vec[i] = 0;
	int count_sal  = s;
	
//	clock_vec[tid];
//	struct msg_lek msg;


	//Tworzymy nowy typ na potrzeby wysylania struktur danych
	int blocklengths[2] = {1, 1};
	MPI_Datatype lek_types[2] = {MPI_INT, MPI_INT};
	MPI_Datatype mpi_lek_type;
	MPI_Aint offsets[2];
	offsets[0] = offsetof(msg_lek_type, rec_id);
	offsets[1] = offsetof(msg_lek_type, clock_rec);
	MPI_Type_create_struct(2, blocklengths, offsets, lek_types, &mpi_lek_type);
	MPI_Type_commit(&mpi_lek_type);

	recv_all(clock_vec, size, tid, mpi_lek_type, &lek, &sal);


	send_req_lek(clock_vec, size, tid, mpi_lek_type, &lek);
	

	//while(recv_all(clock_vec, size, tid, mpi_lek_type, &lek, &sal));
	int nr; //TODO
	/*
	if(lek.count_req_lek + lek.count_ack_lek == size)
		nr = get_nr(lek.kollek, size, tid);
	else
		nr = 1000000;
	*/

	/*while(lek.count_ack_lek != size-1){
        recv_all(clock_vec, size, tid, mpi_lek_type, &lek, &sal);
	}*/

    send_ack_lek(clock_vec, size, tid, mpi_lek_type, &lek);

    recv_all(clock_vec, size, tid, mpi_lek_type, &lek, &sal);

	
		//MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		//printf(" Otrzymalem %d %d od %d\n", msg[0], msg[1], status.MPI_SOURCE);
		//msg[0] = tid;
		//msg[1] = size;
		//MPI_Send( msg, 2, MPI_INT, ROOT, MSG_TAG, MPI_COMM_WORLD );
		//printf(" Wyslalem %d %d do %d\n", msg[0], msg[1], ROOT );

	freeLekStruct(&lek);
	freeSalStruct(&sal);
	MPI_Type_free(&mpi_lek_type);

    printf("TID: %d, clock: %d\n",tid, clock_vec[tid]);

	MPI_Finalize(); // Musi być w każdym programie na końcu
}



int recv_all(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_lek_type, lekstruct* lek, salstruct* sal){
	msg_lek_type buffer;
	MPI_Request req;
	int flag = REQ_LEK;
	MPI_Status status;
	int res = -1;
	//Lek Re

	for(int i=0; i<size; i++){
		MPI_Irecv(&buffer, 1, mpi_lek_type, i, REQ_LEK, MPI_COMM_WORLD, &req);
		MPI_Test(&req, &flag, &status);
		if(flag){
			printf("Odebral %d od %d REQ_LEK clock: %d\n", my_tid, buffer.rec_id, clock_vec[my_tid]);
			if(buffer.clock_rec > clock_vec[my_tid])
				clock_vec[my_tid] = buffer.clock_rec + 1;
			else
                (clock_vec[my_tid]) ++;
			lek->kollek[buffer.rec_id] = buffer;
			if(buffer.clock_rec > lek->clock_lek || (buffer.clock_rec == lek->clock_lek && buffer.rec_id > my_tid)) {
                (lek->count_ack_lek)++;
                lek->acklek[buffer.rec_id] = true;
            }
            else
                (lek->count_req_lek)++;
			res = 0;
		}
	}


    for(int i=0; i<size; i++){
        MPI_Irecv(&buffer, 1, mpi_lek_type, i, ACK_LEK, MPI_COMM_WORLD, &req);
        MPI_Test(&req, &flag, &status);
        if(flag){
            printf("Odebral %d od %d ACK_LEK clock: %d\n", my_tid, buffer.rec_id, clock_vec[my_tid]);
            if(buffer.clock_rec > clock_vec[my_tid])
                clock_vec[my_tid] = buffer.clock_rec + 1;
            else
                (clock_vec[my_tid]) ++;
            (lek->count_ack_lek)++;
            lek->acklek[buffer.rec_id] = true;
            res = 0;
        }
    }

	
	return res;
}

void send_req_lek(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_lek_type, lekstruct* lek){
    clock_vec[my_tid]++;
    msg_lek_type msg;
    msg.clock_rec = clock_vec[my_tid];
    msg.rec_id = my_tid;
	lek->kollek[my_tid] = msg;
	lek->clock_lek = msg.clock_rec;
	for (int i = 0; i < size ; i++){
		if(my_tid != i){
			//printf("Wysłano od %d do %d wiadomosc REQ_LEK\n", msg.rec_id, i);
			MPI_Send(&msg, 1, mpi_lek_type, i, REQ_LEK, MPI_COMM_WORLD);
		}
	}
}

void send_ack_lek(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_lek_type, lekstruct* lek){
    clock_vec[my_tid]++;
    msg_lek_type msg;
    msg.clock_rec = clock_vec[my_tid];
    msg.rec_id = my_tid;

    for (int i = 0; i < size ; i++){
        if(lek->acklek[i]){
            //printf("Wysłano od %d do %d wiadomosc REQ_LEK\n", msg.rec_id, i);
            MPI_Send(&msg, 1, mpi_lek_type, i, ACK_LEK, MPI_COMM_WORLD);
        }
    }
}

int get_nr(msg_lek_type* kollek, int size, int tid){
	int nr = 0;
	for (int i = 0; i < size; i++){
		if(kollek[i].clock_rec < kollek[tid].clock_rec || 
				(kollek[i].clock_rec >= kollek[tid].clock_rec && kollek[i].rec_id < kollek[tid].clock_rec))
			nr++;
	}
	return nr;
}
