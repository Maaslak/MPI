//
// Created by maaslak on 10/06/18.
//
#include "structures.h"


lekstruct initLekStruct(int size){
    lekstruct lek;
    lek.acklek = malloc(size * sizeof(bool));
    lek.kollek = malloc(size * sizeof(msg_lek_type));
    lek.count_ack_lek = 0;
    lek.clock_lek = 0;
    lek.shift = 0;
    lek.count_req_lek = 0;
    return lek;
}

void freeSalStruct(salstruct* sal){
    free(sal->acksal);
    free(sal->kolsal);
}

salstruct initSalStruct(int size){
    salstruct sal;
    sal.acksal = malloc(size * sizeof(bool));
    sal.kolsal = malloc(size * sizeof(msg_sal_type));
    sal.count_req_sal = 0;
    sal.count_ack_sal = 0;
    return sal;
}

void freeLekStruct(lekstruct* lek){
    free(lek->acklek);
    free(lek->kollek);
}