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
int recv_all(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_type, lekstruct* lek, salstruct* sal, comm_type* com, bool first);

void send_req_lek(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_type, lekstruct* lek);
void send_ack_lek(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_type, lekstruct* lek, salstruct* sal);

void send_req_sal(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_type, salstruct* sal);
void send_ack_sal(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_type, salstruct* sal);

//MPI_Datatype initLekDatatype();
MPI_Datatype initDatatype();

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
	int l = 3; //liczba lekarzy
	int m = rand() % 100; //liczba modelek

	int clock_vec[size];
	lekstruct lek = initLekStruct(size, l);
	salstruct sal = initSalStruct(size, m, s);
	for(int i = 0; i < size; i++)
		clock_vec[i] = 0;

	//Tworzymy nowy typ na potrzeby wysylania struktur danych
	//MPI_Datatype mpi_lek_type = initLekDatatype();
	MPI_Datatype mpi_type = initDatatype();

	//Instancja struktury na potrzeby asynchronicznej komunikacji
	comm_type comm;

	recv_all(clock_vec, size, tid, mpi_type, &lek, &sal, &comm, true);


	send_req_lek(clock_vec, size, tid, mpi_type, &lek);


	while(lek.count_ack_lek + lek.count_req_lek != size-1 || lek.count_req_lek >= l || lek.shift != 0){
        recv_all(clock_vec, size, tid, mpi_type, &lek, &sal, &comm, false);
	}
	//printf("sum: %d count_req_lek: %d clock_lek: %d count_acklek: %d Zajęty lekarz nr %d przez proces %d\n",lek.count_ack_lek + lek.count_req_lek, lek.count_req_lek, lek.clock_lek, lek.count_ack_lek, lek.count_req_lek, tid);
	printf("Zajęty lekarz nr %d przez menadżera %d\n",lek.count_req_lek, tid);
	sleep(1);
    printf("Zwolniony lekarz nr %d\n", lek.count_req_lek);
	fflush(stdout);
    sleep(0.005);

    send_ack_lek(clock_vec, size, tid, mpi_type, &lek, &sal);
	sal.start = true;

	//send_req_sal(clock_vec,size, tid, mpi_type, &sal);
	while(sal.count_ack_sal + sal.count_req_sal != size - 1 || sal.count_s < sal.m){
		recv_all(clock_vec, size, tid, mpi_type, &lek, &sal, &comm, false);
	}
	printf("Zajęto %d miejsc w salonie przez menadżera %d ilosc wolnych miejsc: %d\n", sal.m, tid, sal.count_s - sal.m);
	sleep(1);
	printf("Zwolniono %d miejsc w salonie przez menadżera %d\n", sal.m, tid);
	fflush(stdout);
	sleep(0.005);
	send_ack_sal(clock_vec, size, tid, mpi_type, &sal);

	while(sal.count_ack_sal != size-1){
		recv_all(clock_vec, size, tid, mpi_type, &lek, &sal, &comm, false);
	}
	printf("\"Rozpoczynamy konkurs\" - krzyczy menadżer %d\n", tid);

	freeLekStruct(&lek);
	freeSalStruct(&sal);
	//MPI_Type_free(&mpi_lek_type);
	MPI_Type_free(&mpi_type);

    //printf("TID: %d, clock: %d\n",tid, clock_vec[tid]);

	MPI_Finalize(); // Musi być w każdym programie na końcu
}



int recv_all(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_type, lekstruct* lek, salstruct* sal, comm_type* com, bool first){
	int flag = 1;
	int res = -1;
	if(first)
        MPI_Irecv(&(com->buffer), 1, mpi_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &(com->req));

	do{
        MPI_Test(&(com->req), &flag, &(com->status));
        if (!flag)break;

		if (com->buffer.clock_rec > clock_vec[my_tid])
			clock_vec[my_tid] = com->buffer.clock_rec + 1;
		else
			(clock_vec[my_tid])++;

        switch (com->status.MPI_TAG){
            case REQ_LEK:   //printf("Odebral %d od %d REQ_LEK clock: %d\n", my_tid, com->buffer.rec_id, clock_vec[my_tid]);
                            lek->kollek[com->status.MPI_SOURCE] = com->buffer;
                            if (com->buffer.clock_rec > lek->clock_lek || (com->buffer.clock_rec == lek->clock_lek && com->status.MPI_SOURCE > my_tid)) {
                            	//printf("moje: %d %d, jego %d %d\n", lek->clock_lek, my_tid, com->buffer.clock_rec, com->buffer.rec_id);
								(lek->count_ack_lek)++;
                                lek->acklek[com->status.MPI_SOURCE] = true;
                            } else
                                (lek->count_req_lek)++;
                            break;
            case ACK_LEK:   //printf("Odebral %d od %d ACK_LEK clock: %d\n", my_tid, buffer.rec_id, clock_vec[my_tid]);
                            if(!lek->acklek[com->status.MPI_SOURCE]) {
                                //printf("ID: %d get ACK from %d\n", my_tid, (com->buffer.rec_id));
								(lek->count_req_lek)--;
								(lek->count_ack_lek)++;
								lek->shift = (lek->shift + 1) % lek->l;
                            }
							sal->kolsal[com->status.MPI_SOURCE] = com->buffer;
							if(com->buffer.clock_rec > sal->clock_sal || (com->buffer.clock_rec == sal->clock_sal && com->status.MPI_SOURCE > my_tid)){
								//if(!sal->acksal[com->status.MPI_SOURCE])
								sal->count_ack_sal++;
								sal->acksal[com->status.MPI_SOURCE] = true;
							}
							else{
								sal->count_req_sal++;
								sal->count_s -= com->buffer.m_rec;
							}
                            break;
        	/*case REQ_SAL:	//if(sal->start){
        						sal->kolsal[com->status.MPI_SOURCE] = com->buffer;
        						if(com->buffer.clock_rec > sal->clock_sal || (com->buffer.clock_rec == sal->clock_sal && com->status.MPI_SOURCE > my_tid)){
									//if(!sal->acksal[com->status.MPI_SOURCE])
        							sal->count_ack_sal++;
        							sal->acksal[com->status.MPI_SOURCE] = true;
        						}
        						else{
                            		sal->count_req_sal++;
                            		sal->count_s -= com->buffer.m_rec;
                            	}
        					//}
        					//else {
								//send_ack_sal(clock_vec, size, my_tid, mpi_type, sal);
							//	sal->count_req_sal++;
							//}
        					break;*/
			case ACK_SAL:	//if(com->buffer.clock_rec > sal->clock_sal || (com->buffer.clock_rec == sal->clock_sal && com->status.MPI_SOURCE > my_tid))
							sal->count_s += com->buffer.m_rec;
							//printf("%d\n",sal->count_s);
							if(!sal->acksal[com->status.MPI_SOURCE]) {
								sal->count_ack_sal++;
								sal->count_req_sal--;
							}
							sal->acksal[com->status.MPI_SOURCE] = true;
							break;
        }
		MPI_Irecv(&(com->buffer), 1, mpi_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &(com->req));
	}while (true);

	return res;
}

void send_req_lek(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_type, lekstruct* lek){
    //(clock_vec[my_tid])++;
    msg_type msg;
    msg.clock_rec = clock_vec[my_tid];
	lek->kollek[my_tid] = msg;
	lek->clock_lek = msg.clock_rec;

	for (int i = 0; i < size ; i++){
		if(my_tid != i){
			//printf("Wysłano od %d do %d wiadomosc REQ_LEK\n", msg.rec_id, i);
			MPI_Send(&msg, 1, mpi_type, i, REQ_LEK, MPI_COMM_WORLD);
		}
	}
}

void send_ack_lek(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_type, lekstruct* lek, salstruct* sal){
    //(clock_vec[my_tid])++;
    msg_type msg;
    msg.clock_rec = clock_vec[my_tid];
	msg.m_rec = sal->m;
	sal->kolsal[my_tid] = msg;
	sal->clock_sal = msg.clock_rec;

	for (int i = 0; i < size ; i++){
        if(my_tid != i){
            //printf("%d Wysłano od %d do %d wiadomosc ACK_LEK\n", size, msg.rec_id, i);
            MPI_Send(&msg, 1, mpi_type, i, ACK_LEK, MPI_COMM_WORLD);
        }
    }
}
/*
void send_req_sal(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_type, salstruct* sal){
	//(clock_vec[my_tid])++;
	msg_type msg;
	msg.clock_rec = clock_vec[my_tid];
	msg.m_rec = sal->m;
	sal->kolsal[my_tid] = msg;
	sal->clock_sal = msg.clock_rec;

	for (int i = 0; i < size ; i++){
		if(my_tid != i){
			//printf("Wysłano od %d do %d wiadomosc REQ_SAL\n", msg.rec_id, i);
			MPI_Send(&msg, 1, mpi_type, i, REQ_SAL, MPI_COMM_WORLD);
		}
	}
}
*/
void send_ack_sal(int *clock_vec, int size, int my_tid, MPI_Datatype mpi_type, salstruct* sal){
	//(clock_vec[my_tid])++;
	msg_type msg;
	msg.clock_rec = clock_vec[my_tid];
	msg.m_rec = sal->m;

	for (int i = 0; i < size ; i++){
		if(my_tid != i){
			//printf("%d Wysłano od %d do %d wiadomosc ACK_LEK\n", size, msg.rec_id, i);
			MPI_Send(&msg, 1, mpi_type, i, ACK_SAL, MPI_COMM_WORLD);
		}
	}
}
/*
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
}*/

MPI_Datatype initDatatype(){
	int blocklengths[2] = {1, 1};
	MPI_Datatype types[2] = {MPI_INT, MPI_INT};
	MPI_Datatype mpi_type;
	MPI_Aint offsets[2];
	offsets[0] = offsetof(msg_type, clock_rec);
	offsets[1] = offsetof(msg_type, m_rec);
	MPI_Type_create_struct(2, blocklengths, offsets, types, &mpi_type);
	MPI_Type_commit(&mpi_type);
	return mpi_type;
}