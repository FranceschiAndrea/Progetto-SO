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
  ListHead list_semafori=running->sem_descriptor;

  SemDescriptor* sem_dsc = (SemDescriptor*)SemDescriptorList_byFd(&list_semafori, sem_d);

  //controllo l' esistenza del semaforo
  if(!sem_dsc)
  {

      running->syscall_retvalue=-1;
      return;
  }

    Semaphore* sem = sem_dsc->semaphore;

    // iterazione sul contatore del semaforo per il controllo di processi in waiting
    while(sem<count){


    }


}
