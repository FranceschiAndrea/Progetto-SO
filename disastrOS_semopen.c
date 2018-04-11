#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semOpen(){

    //prendo i valori per l'apertura del semaforo
    int n_semaforo = running->syscall_args[0];

    int sem_val = running->syscall_args[1];

    //creo il semaforo allocandolo con sem_num = n_semaforo
    Semaphore* sem = Semaphore_alloc(semnum, value);
    printf( "Alloco il semaforo con sem_num = %d\n", n_semaforo);
    if(!sem){
        running->syscall_retvalue = DSOS_ECREATESEM;
        return;
    }

    //controllo se il semaforo che voglio creare non è gia aperto
    ListHead semafori_aperti = running->sem_descriptor;
    SemDescriptor* sem_aperto = check_id(&semafori_aperti, n_semaforo);
    if( sem_aperto ){
        running->syscall_retvalue = sem_aperto->fd;
    }



}
