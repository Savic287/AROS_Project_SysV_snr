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
#define FILENAME "sender.c"
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

int main(int argc, char *argv[]){

	char *blok = attach_memory_block(FILENAME, SIZE);

        if(blok == NULL){
                printf("Error! Unable to get shm block!");
                return -1;
        }

        sem_t *sem_send = sem_open(SEM_SENDER, 0);
        sem_t *sem_rece = sem_open(SEM_RECEIVER, 1);

	for(int i = 0; i< 5; i++){
		sem_wait(sem_rece);
		printf("Sending: \"%s\"\n", argv[1]);
		sprintf(blok, "%s", argv[1]);
		sem_post(sem_send);
	}

	sem_close(sem_send);
        sem_close(sem_rece);
        detach_memory_block(blok);

	return 0;
}
