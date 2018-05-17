#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semClose(){
  int fd= running->syscall_args[0];

        SemDescriptor* semD= SemDescriptorList_byFd(&running->sem_descriptors, fd);
        if(!semD)
        {
        running->syscall_retvalue=-1;
        }

        //prendo il semaforo del semD
        Semaphore* sem= semD->semaphore;

        //prendo i descrittori associati e li andro ad eliminare
        SemDescriptorPtr* semD_ptr = semD->ptr;


        semD_ptr=(SemDescriptorPtr*) List_detach(&(sem->descriptors),(ListItem*) semD_ptr);
        int r=SemDescriptor_free(semD_ptr);
        if(r)
        {
            running->syscall_retvalue=r;
            return;
        }

        SemDescriptorPtr* semD_wait = semD->ptr_wait;
        r=SemDescriptorPtr_free(semD_wait)
        if(r){
            running->syscall_retvalue=r;
            return;
        }

        //free del puntatore a semD della lista dei descrittori del processo
        semD=(SemDescriptor*) List_detach(&(running->sem_descriptors), (ListItem*) semD);
        r=SemDescriptor_free(semD);
        if(r){
            running->syscall_retvalue=r;
            return;
        }
        //al semaforo non sono associati piu descrittori, posso fare la free
        if((sem->descriptors).size==0)
        {
          sem = (Semaphore*) List_detach(&semaphores_list, (ListItem*) sem);
          r=semaphore_free(sem)
          if(r)
          {
            running->syscall_retvalue =r;
            return;
          }
        }
        running->syscall_retvalue = 0;
}
