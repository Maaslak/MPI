//
// Created by maaslak on 10/06/18.
//
#include "structures.h"


lekstruct initLekStruct(int size, int l){
    lekstruct lek;
    lek.acklek = malloc(size * sizeof(bool));
    for(int i = 0; i < size; i++)
        lek.acklek[i] = false;
    lek.kollek = malloc(size * sizeof(msg_type));
    lek.count_ack_lek = 0;
    lek.clock_lek = 1000000;
    lek.shift = 0;
    lek.count_req_lek = 0;
    lek.l = l;
    return lek;
}

void freeSalStruct(salstruct* sal){
    free(sal->acksal);
    free(sal->kolsal);
}

salstruct initSalStruct(int size, int m, int s){
    salstruct sal;
    sal.acksal = malloc(size * sizeof(bool));
    sal.kolsal = malloc(size * sizeof(msg_type));
    sal.count_req_sal = 0;
    sal.count_ack_sal = 0;
    sal.clock_sal = 100000;
    sal.count_s = s;
    sal.m = m;
    sal.start = false;
    return sal;
}

void freeLekStruct(lekstruct* lek){
    free(lek->acklek);
    free(lek->kollek);
}