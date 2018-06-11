//
// Created by maaslak on 10/06/18.
//

#ifndef KONKURSPIEKNOSCI_C_STRUCTURES_H
#define KONKURSPIEKNOSCI_C_STRUCTURES_H
#include <mpi.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct msg_lek{
    int rec_id;
    int clock_rec;
}msg_lek_type;

typedef struct msg_sal{
    int rec_id;
    int clock_rec;
    int m_rec;
}msg_sal_type;

typedef struct comunication{
    MPI_Status status;
    msg_lek_type buffer;
    MPI_Request req;
}comm_type;

typedef struct lek_type{
    int l;
    int clock_lek, count_ack_lek, shift;
    bool* acklek;
    msg_lek_type* kollek; // kolejka do lekarzy
    int count_req_lek;
}lekstruct;

typedef struct sal_type{
    bool start;
    int m, s;
    int count_s;
    int count_req_sal, clock_sal, count_ack_sal;
    msg_sal_type* kolsal; //kolejka do salonu
    bool* acksal;
}salstruct;

void freeSalStruct(salstruct* );
void freeLekStruct(lekstruct* );
lekstruct initLekStruct(int, int);
salstruct initSalStruct(int, int, int);




#endif //KONKURSPIEKNOSCI_C_STRUCTURES_H
