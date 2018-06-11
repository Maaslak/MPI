#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "structures.h"
#define ROOT 0
#define ACK_LEK 100
#define REQ_LEK 101
#define ACK_SAL 102
#define REQ_SAL 103

// TODO
int recv_all(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_lek_type, lekstruct* lek, salstruct* sal, comm_type* com, bool first);

void send_req_lek(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_lek_type, lekstruct* lek);
void send_ack_lek(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_lek_type, lekstruct* lek);

void send_req_sal(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_sal_type, salstruct* sal);

MPI_Datatype initLekDatatype();
MPI_Datatype initSalDatatype();

int main(int argc, char **argv)
{
	int tid,size;
	MPI_Status status;
	time_t tt;
	srand(time(&tt));
	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &tid);

	int s = 200; // pojemnosc salonu
	int m = rand() % 100; //liczba modelek
	int l = 2; //liczba lekarzy
	int clock_vec[size];
	lekstruct lek = initLekStruct(size, l);
	salstruct sal = initSalStruct(size, m, s);
	for(int i = 0; i < size; i++)
		clock_vec[i] = 0;

	//Tworzymy nowe typy na potrzeby wysylania struktur danych
	MPI_Datatype mpi_lek_type = initLekDatatype();
	MPI_Datatype mpi_sal_type = initSalDatatype();
//	clock_vec[tid];
//	struct msg_lek msg;


	comm_type comm;

	recv_all(clock_vec, size, tid, mpi_lek_type, &lek, &sal, &comm, true);


	send_req_lek(clock_vec, size, tid, mpi_lek_type, &lek);

	/*
	if(lek.count_req_lek + lek.count_ack_lek == size)
		nr = get_nr(lek.kollek, size, tid);
	else
		nr = 1000000;
	*/

	while(lek.count_ack_lek + lek.count_req_lek != size-1 || lek.count_req_lek >= l || lek.shift != 0){
        recv_all(clock_vec, size, tid, mpi_lek_type, &lek, &sal, &comm, false);
	}
	//printf("sum: %d count_req_lek: %d l: %d Zajęty lekarz nr %d przez proces %d\n",lek.count_ack_lek + lek.count_req_lek, lek.count_req_lek, lek.l, lek.count_req_lek, tid);
	printf("Zajęty lekarz nr %d przez proces %d\n",lek.count_req_lek, tid);
	sleep(1);
    send_ack_lek(clock_vec, size, tid, mpi_lek_type, &lek);
	printf("Zwolniony lekarz nr %d\n", lek.count_req_lek);
	sal.start = true;


	freeLekStruct(&lek);
	freeSalStruct(&sal);
	MPI_Type_free(&mpi_lek_type);
	MPI_Type_free(&mpi_sal_type);

    //printf("TID: %d, clock: %d\n",tid, clock_vec[tid]);

	MPI_Finalize(); // Musi być w każdym programie na końcu
}



int recv_all(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_lek_type, lekstruct* lek, salstruct* sal, comm_type* com, bool first){
	int flag = 1;
	int res = -1;
	if(first)
        MPI_Irecv(&(com->buffer), 1, mpi_lek_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &(com->req));

	do{
        MPI_Test(&(com->req), &flag, &(com->status));
        if (!flag)break;

		if (com->buffer.clock_rec > clock_vec[my_tid])
			clock_vec[my_tid] = com->buffer.clock_rec + 1;
		else
			(clock_vec[my_tid])++;

        switch (com->status.MPI_TAG){
            case REQ_LEK:   //printf("Odebral %d od %d REQ_LEK clock: %d\n", my_tid, com->buffer.rec_id, clock_vec[my_tid]);
                            lek->kollek[com->buffer.rec_id] = com->buffer;
                            if (com->buffer.clock_rec > lek->clock_lek || (com->buffer.clock_rec == lek->clock_lek && com->buffer.rec_id > my_tid)) {
                            	//printf("moje: %d %d, jego %d %d\n", lek->clock_lek, my_tid, com->buffer.clock_rec, com->buffer.rec_id);
								(lek->count_ack_lek)++;
                                lek->acklek[com->buffer.rec_id] = true;
                            } else
                                (lek->count_req_lek)++;
                            break;
            case ACK_LEK:   //printf("Odebral %d od %d ACK_LEK clock: %d\n", my_tid, buffer.rec_id, clock_vec[my_tid]);
                            if(!lek->acklek[com->buffer.rec_id]) {
								(lek->count_req_lek)--;
								(lek->count_ack_lek)++;
								lek->shift = (lek->shift + 1) % lek->l;
							}
                            break;
        	/*case REQ_SAL:	if(sal->start){	TODO
        						sal->kolsal[com->buffer.rec_id] = com->buffer;
        						if(com->buffer.clock_rec > sal->clock_sal || (com->buffer.clock_rec == sal->clock_sal && com->buffer.rec_id > my_tid)){
        							sal->count_ack_sal++;
        							sal->acksal[com->buffer.rec_id] = true;
        						}
        						else{
                            		sal->count_req_sal++;
                            		sal->count_s -= com->buffer.
                            	}
        					}

							break;*/
        }
		MPI_Irecv(&(com->buffer), 1, mpi_lek_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &(com->req));
	}while (true);

	return res;
}

void send_req_lek(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_lek_type, lekstruct* lek){
    //(clock_vec[my_tid])++;
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
    //(clock_vec[my_tid])++;
    msg_lek_type msg;
    msg.clock_rec = clock_vec[my_tid];
    msg.rec_id = my_tid;

    for (int i = 0; i < size ; i++){
        if(my_tid != i){
            //printf("%d Wysłano od %d do %d wiadomosc ACK_LEK\n", size, msg.rec_id, i);
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

void send_req_sal(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_sal_type, salstruct* sal){
	//(clock_vec[my_tid])++;
	msg_sal_type msg;
	msg.clock_rec = clock_vec[my_tid];
	msg.rec_id = my_tid;
	msg.m_rec = sal->m;
	sal->kolsal[my_tid] = msg;
	sal->clock_sal = msg.clock_rec;

	for (int i = 0; i < size ; i++){
		if(my_tid != i){
			//printf("Wysłano od %d do %d wiadomosc REQ_SAL\n", msg.rec_id, i);
			MPI_Send(&msg, 1, mpi_sal_type, i, REQ_SAL, MPI_COMM_WORLD);
		}
	}
}

MPI_Datatype initLekDatatype(){
	int blocklengths[2] = {1, 1};
	MPI_Datatype lek_types[2] = {MPI_INT, MPI_INT};
	MPI_Datatype mpi_lek_type;
	MPI_Aint offsets[2];
	offsets[0] = offsetof(msg_lek_type, rec_id);
	offsets[1] = offsetof(msg_lek_type, clock_rec);
	MPI_Type_create_struct(2, blocklengths, offsets, lek_types, &mpi_lek_type);
	MPI_Type_commit(&mpi_lek_type);
	return mpi_lek_type;
}

MPI_Datatype initSalDatatype(){
	int blocklengths[3] = {1, 1, 1};
	MPI_Datatype sal_types[3] = {MPI_INT, MPI_INT, MPI_INT};
	MPI_Datatype mpi_sal_type;
	MPI_Aint offsets[3];
	offsets[0] = offsetof(msg_sal_type, rec_id);
	offsets[1] = offsetof(msg_sal_type, clock_rec);
	offsets[2] = offsetof(msg_sal_type, m_rec);
	MPI_Type_create_struct(3, blocklengths, offsets, sal_types, &mpi_sal_type);
	MPI_Type_commit(&mpi_sal_type);
	return mpi_sal_type;
}