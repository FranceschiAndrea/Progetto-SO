#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semWait(){

    int fd = running->syscall_args[0];

    //prendo la lista dei semafori aperti dal processo
    ListHead sem_aperti = running->sem_descriptors;

    //cerco tra i semafori aperti quello sul quale devo fare la wait
    SemDescriptor* sem_desc = (SemDescriptor*) SemDescriptorList_byFd(&sem_aperti, fd);

    if( !sem_desc ){
        running->syscall_retvalue = DSOS_ESEMNOTOPENEDBYME;
        return;
    }

    //controllo se il semaforo è ancora aperto
    Semaphore* semaforo = sem_desc->semaphore;

    if ( !semaforo ){
        //entro qui solo se un altro processo ha chiuso il semaforo e nel running è rimasto il puntatore al descrittore
        running->syscall_retvalue = DSOS_ESEMNOTOPENED;
        return;
    }

}
