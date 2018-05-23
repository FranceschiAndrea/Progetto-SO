#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS.h"

#include "disastrOS_semaphore.h"
#include "disastrOS_globals.h"

#define SEM_FILL 0
#define SEM_EMPTY 1
#define SEM_MUTEX1 2
#define SEM_MUTEX2 3
#define BUFFER_LENGTH_SEM 5
#define ROUNDS 10


#define ERROR_HELPER(cond, msg) do {    \
        if (cond && (running->pid!=0)) {              \
            printf("%s: %d \n", msg,running->pid);      \
              disastrOS_exit(disastrOS_getpid()+1); \
        }   \
        else if (cond && (running->pid==0)){     \
            printf("%s:",msg);  \
            disastrOS_exit(disastrOS_getpid()+1); \
        }   \
    } while(0)  \

int read_index,write_index, deposit; // indici per la lettura e la scrittura e per token prodotti dai produttori
int transactions[BUFFER_LENGTH_SEM];

void initFunction_semaphores(void* args);
void PrintBuffer(int * buffer);
void Prod(void* args);
void Cons(void* args);

// processo sempre in "circolo"
void sleeperFunction(void* args){
    printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
    while(1) {
        getc(stdin);
        disastrOS_printStatus();
    }
}


void Prod(void* args){
    int i,ret;

    printf("Starting producer with pid : %d\n",running->pid);
    // apro i semafori per slot buffer pieni o vuoti e per l'accesso alla sezione critica
    int sem_fill= disastrOS_semOpen(SEM_FILL, 0);
    ERROR_HELPER(sem_fill < 0,"Error semOpen sem_fill process ");       // inizializzo il semaforo a 0, poichè sono stati prodotti 0 "token"

    int sem_empty = disastrOS_semOpen(SEM_EMPTY, BUFFER_LENGTH_SEM);    // inizializzo il semaforo a BUFFER_LENGTH_SEM, poichè ho ancora tutto lo spazio a disposizione
    ERROR_HELPER(sem_empty < 0,"Error semOpen sem_empty process ");

    int sem_mutex1 = disastrOS_semOpen(SEM_MUTEX1, 1);                  // semaforo che gestisce le operazioni tra i processi produttori, se un produttore è in cs gli altri devono aspettare per produrre
    ERROR_HELPER(sem_mutex1 < 0,"Error semOpen sem_mutex1 process ");

    ListHead semaphores_used = sem_list;
    Semaphore* sem_e = SemaphoreList_byId(&semaphores_used,sem_empty);
    Semaphore* sem_f = SemaphoreList_byId(&semaphores_used,sem_fill);



    for(i = 0;i < ROUNDS;i++){
        printf("\n\n+++++\n+++++\n+++++\nPid: %d\nsem_empty: %d\nsem_fill: %d\n+++++\n+++++\n+++++\n\n",running->pid, sem_e->count, sem_f->count);
        ret = disastrOS_semWait(sem_empty);                             // se il buffer è pieno(sem_empty=0) devo aspettare che abbia almeno uno spazio per poter inserire il "token"
        ERROR_HELPER(ret != 0, "Error semWait sem_empty process ");
        ret = disastrOS_semWait(sem_mutex1);                            // devo aspettare che sia il mio turno tra tutti i produttori
        ERROR_HELPER(ret != 0, "Error semWait sem_mutex1 process ");

        printf("Hello, i am prod and i am in CS! Pid : %d\n",running->pid);
        transactions[write_index] = running->pid;                       //  produco il "token"
        write_index = (write_index + 1) % BUFFER_LENGTH_SEM;

        ret = disastrOS_semPost(sem_mutex1);                            // comunico agli altri produttori che ho finito, uscendo dalla cs e facendo la post sul mutex
        ERROR_HELPER(ret != 0, "Error semPost sem_mutex1 process ");


        ret = disastrOS_semPost(sem_fill);                              // incremento sem_fill poichè ho inserito un "token"
        ERROR_HELPER(ret != 0, "Error semPost sem_fill process ");
    }


    ret = disastrOS_semClose(sem_fill);
    ERROR_HELPER(ret != 0, "Error semClose sem_fill process");

    ret = disastrOS_semClose(sem_empty);
    ERROR_HELPER(ret != 0, "Error semClose sem_empty process");

    ret = disastrOS_semClose(sem_mutex1);
    ERROR_HELPER(ret != 0, "Error semClose sem_mutex1 process");


    disastrOS_exit(disastrOS_getpid()+1);
}

void Cons(void* args){
    int i,ret;

    printf("Starting consumer with pid : %d\n",running->pid);

    // apro i semafori per la gestione del buffer e per l'accesso alla cs dei consumatori
    int sem_fill= disastrOS_semOpen(SEM_FILL, 0);
    ERROR_HELPER(sem_fill < 0,"Error semOpen sem_fill process ");

    int sem_empty = disastrOS_semOpen(SEM_EMPTY, BUFFER_LENGTH_SEM);
    ERROR_HELPER(sem_empty < 0,"Error semOpen sem_empty process ");

    int sem_mutex2 = disastrOS_semOpen(SEM_MUTEX2, 1);
    ERROR_HELPER( sem_mutex2 < 0,"Error semOpen sem_mutex2 process ");

    ListHead semaphores_used = sem_list;
    Semaphore* sem_e = SemaphoreList_byId(&semaphores_used,sem_empty);
    Semaphore* sem_f = SemaphoreList_byId(&semaphores_used,sem_fill);


    for(i = 0;i < ROUNDS;i++){
        printf("\n\n+++++\n+++++\n+++++\nPid: %d\nsem_empty: %d\nsem_fill: %d\n+++++\n+++++\n+++++\n\n",running->pid, sem_e->count, sem_f->count);
        ret = disastrOS_semWait(sem_fill);                                              // aspetto finchè sem_fill non contenga almeno un "token" prodotto
        ERROR_HELPER(ret != 0, "Error semWait sem_fill process");

        ret = disastrOS_semWait(sem_mutex2);                                            // semaforo per regolare l'accesso alla cs dei consumatori (1 per volta)
        ERROR_HELPER(ret != 0, "Error semWait sem_mutex2 process ");

        printf("Hello,i am the cons and i am in CS! Pid : %d\n",running->pid);
        int lastTransaction = transactions[read_index];
        read_index = (read_index + 1) % BUFFER_LENGTH_SEM;
        deposit += lastTransaction;
        if (read_index % 10 == 0) {
            printf("After the last 10 transactions balance is now %d.\n", deposit);
        }

        ret = disastrOS_semPost(sem_mutex2);                                            // comunico agli altri consumatori che ho finito facendo la post sul sem_mutex
        ERROR_HELPER(ret != 0, "Error semPost sem_mutex2 process ");

        ret = disastrOS_semPost(sem_empty);                                             // incremento sem_empty poichè ho liberato uno spazio
        ERROR_HELPER(ret != 0, "Error semPost sem_empty process ");

    }

    ret = disastrOS_semClose(sem_fill);
    ERROR_HELPER(ret != 0, "Error semClose fd_fill process ");
    ret = disastrOS_semClose(sem_empty);
    ERROR_HELPER(ret != 0, "Error semClose fd_empty process ");
    ret = disastrOS_semClose(sem_mutex2);
    ERROR_HELPER(ret != 0, "Error semClose fd_me2 process ");

    disastrOS_exit(disastrOS_getpid()+1);
}



void initFunction_semaphores(void* args) {
    disastrOS_printStatus();
    printf("hello, I am init and I just started pid=%d\n",running->pid);
    disastrOS_spawn(sleeperFunction, 0);

    //inizializzo write index e read_index
    write_index=read_index=0;
    printf("I feel like to spawn 10 nice processes\n");
    int children=0;
    int i;
    int fd[10];
    for (i=0; i<5; ++i) {
        int type=0;
        int mode=DSOS_CREATE;
        printf("mode: %d\n", mode);
        printf("opening resource\n");
        fd[i]=disastrOS_openResource(i,type,mode);
        printf("fd=%d\n", fd[i]);
        disastrOS_spawn(Prod, 0);
        children++;
    }

    for (; i<10; ++i) {
        int type=0;
        int mode=DSOS_CREATE;
        printf("mode: %d\n", mode);
        printf("opening resource\n");
        fd[i]=disastrOS_openResource(i,type,mode);
        printf("fd=%d\n", fd[i]);
        disastrOS_spawn(Cons, 0);
        children++;
    }
    int retval;
    int pid;
    while(children>0 && (pid=disastrOS_wait(0, &retval))>=0){
        printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
         pid, retval, children);
        --children;
    }
    for (i=0; i<10; ++i) {
        printf("closing resource %d\n",fd[i]);
        disastrOS_closeResource(fd[i]);
        disastrOS_destroyResource(i);
    }

    disastrOS_printStatus();

    printf("shutdown!\n");
    disastrOS_shutdown();
}

int main(int argc, char** argv){
    char* logfilename=0;
    if (argc>1) {
    logfilename=argv[1];
    }
    // creiamo il processo init
    // il primo sara in running mentre gli altri verranno messi in ready queue
    // spawno il processo init

    printf("start\n");
    disastrOS_start(initFunction_semaphores, 0, logfilename);
    return 0;
}
