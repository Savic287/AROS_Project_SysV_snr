#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>

#define SIZE 4096
#define IPC_RESULT_ERROR (-1)
#define NAME "sender.c"

#define SEM_SENDER "/mysender"
#define SEM_RECEIVER "/myreceiver"

static int get_shared_block(char *filename, int size){

        key_t key;

        key = ftok(filename, 0);

        if(key == IPC_RESULT_ERROR){
                return IPC_RESULT_ERROR;
        }
        return shmget(key, size, 0644 | IPC_CREAT);
}

char *attach_memory_block(char *filename, int size){
        int shared_block_id = get_shared_block(filename, size);
        char *result;
        result = shmat(shared_block_id, NULL, 0);
        if(result == (char *)IPC_RESULT_ERROR){
                return NULL;
        }
        return result;
}

bool detach_memory_block(char *block){
        return (shmdt(block) != IPC_RESULT_ERROR);
}

bool destroy_memory_block(char *filename){

        int shared_block_id = get_shared_block(filename, 0);
        if(shared_block_id == IPC_RESULT_ERROR){
                return NULL;
        }
	return (shmctl(shared_block_id, IPC_RMID, NULL) != IPC_RESULT_ERROR);
}

int main(void){

        char *blok = attach_memory_block(NAME, SIZE);

        if(blok == NULL){
                printf("Error! Unable to get shm block!");
                return -1;
        }

	sem_unlink(SEM_SENDER);
        sem_unlink(SEM_RECEIVER);

        sem_t *sem_send = sem_open(SEM_SENDER, O_CREAT, 0660, 0);
        sem_t *sem_rece = sem_open(SEM_RECEIVER, O_CREAT, 0660, 1);

        while(1){
                sem_wait(sem_send);
                if(strlen(blok)>0){
                        printf("Receiving: \"%s\"\n", blok);
                        bool done = (strcmp(blok,"quit") == 0);
                        blok[0] = 0;
                        if(done){
                                break;
                        }
                }
                sem_post(sem_rece);
        }
	sem_close(sem_rece);
        sem_close(sem_send);
        detach_memory_block(blok);

        return 0;
}
