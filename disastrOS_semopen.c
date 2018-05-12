#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "linked_list.h"
#include "disastrOS_globals.h"

void internal_semOpen(){

    //prendo i valori per l'apertura del semaforo
    int n_semaforo = running->syscall_args[0];

    int sem_val = running->syscall_args[1];

    int ultimo_sem_usato = running->last_sem_fd;

    //controllo che il semaforo non sia gia esistente
    ListHead sem_esistenti = sem_list;
    Semaphore* sem = SemaphoreList_byId(&sem_esistenti,n_semaforo);

    if(!sem){
        //creo il semaforo allocandolo con sem_num = n_semaforo
        sem = Semaphore_alloc(n_semaforo, sem_val);
        printf( "Alloco il semaforo con sem_num = %d\n", n_semaforo);
        if(!sem){
            running->syscall_retvalue = DSOS_ECREATESEM;
            return;
        }
        //inserisco il semaforo creato nella variabile globale contenete la lista dei semafori creati
        List_insert(&sem_list, sem_list.last, (ListItem*) sem);
    }

    //controllo se il semaforo che voglio creare non è gia aperto
    ListHead semafori_aperti = running->sem_descriptors;
    SemDescriptor* sem_aperto = check_id(&semafori_aperti, n_semaforo);
    if( sem_aperto ){
        running->syscall_retvalue = sem_aperto->fd;
        return;
    }

    //incremento il last_sem_fd in modo tale che il prossimo semaforo che il processo aprirà avrà l'fd diverso dal precedente
    (running->last_sem_fd)+=1;

    //creo un descrittore che aggiungero al PCB del processo chiamante
    SemDescriptor* desc_pcb = SemDescriptor_alloc(ultimo_sem_usato, sem, running);
    if(!desc_pcb){
        running->syscall_retvalue = DSOS_ECREATESEM;
        return;
    }

    //alloco il puntatore al semaforo
    SemDescriptorPtr* puntatore_sem = SemDescriptorPtr_alloc(desc_pcb);
    if(!puntatore_sem){
        running->syscall_retvalue = DSOS_ECREATEPTR;
        return;
    }
    desc_pcb->ptr = puntatore_sem;

    //alloco e metto nella struttura del descrittore dei semafori un puntatore da usare nelle code di waiting per non allocarlo nelle sem_wait e deallocarlo nelle sem_post

    SemDescriptorPtr * puntatore_wait = SemDescriptorPtr_alloc(desc_pcb);
    if( !puntatore_wait ) {
        running->syscall_retvalue = DSOS_ECREATEPTR;
        return;
    }
    desc_pcb->ptr_wait = puntatore_wait;


    //aggiungo il descrittore del semaforo appena aperto alla lista dei semafori nel PCB del processo che ha chiamato la funzione
    List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem*) desc_pcb);

    //aggiungo il semaforo appena aperto alla lista deisemafori
    List_insert(&sem -> descriptors, sem-> descriptors.last, (ListItem*) puntatore_sem);

    running->syscall_retvalue = desc_pcb->fd;
    return;
}
