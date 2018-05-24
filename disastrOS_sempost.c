#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semPost(){
  int sem_d= running->syscall_args[0];

  //creo una lista di tutti i semafori attivi
  ListHead list_semafori=running->sem_descriptors;

  SemDescriptor* sem_dsc=(SemDescriptor*)SemDescriptorList_byFd(&list_semafori, sem_d);

  //controllo l' esistenza del semaforo
  if(!sem_dsc)
  {
      running->syscall_retvalue=DSOS_ESEMPOSTNOPENED;
      return;
  }

    Semaphore* sem=sem_dsc->semaphore;

    //controllo esistenza sem
    if (!sem){
        running->syscall_retvalue=DSOS_ENOTOPENED;
        return;
    }

    // iterazione sul contatore del semaforo per il controllo di processi in waiting e controllo dell' esistenza del descrittore preso
    if(sem->count<0 && sem->waiting_descriptors.first != NULL){

        //devo prendere il processo in testa alla coda di attesa
        SemDescriptorPtr* head_wait=(SemDescriptorPtr*) List_detach(&(sem->waiting_descriptors),(ListItem*)(sem->waiting_descriptors).first);

        //pcb del processo
        PCB* pcb=head_wait->descriptor->pcb;

        //lrimozione dalla lista di attesa
        List_detach(&waiting_list, (ListItem*) pcb);

        //inserimento in  ready
        List_insert(&ready_list, (ListItem*) ready_list.last, (ListItem*) pcb);
        pcb->status=Ready;

    }

       //aumento del semaforo
        (sem->count)++;

        running->syscall_retvalue=0;
        return;


}
