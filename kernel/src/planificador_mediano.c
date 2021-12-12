#include "planificador_mediano.h"

void iniciar_planificador_mediano(){
    printf("Inicio planificador MEDIANO \n");


    pthread_t hilo_planificador;
    pthread_create(&hilo_planificador, NULL, planificador_mediano_plazo, (void *)NULL);

    pthread_t hilo_salida_de_block;
    pthread_create(&hilo_salida_de_block, NULL, salida_de_block, (void *)NULL);
}


void *planificador_mediano_plazo(void *_){
    while(1){
        sem_wait(&cambio_de_listas_mediano);

        int tamanio_block = list_size(lista_blocked);
        int tamanio_ready = list_size(lista_ready);
        int tamanio_new = list_size(lista_new);

        if(tamanio_block > 0 && tamanio_ready == 0 && tamanio_new > 0){
            
            
            mover_proceso_de_lista(lista_blocked, lista_s_blocked, tamanio_block - 1, S_BLOCKED);
            sem_wait(&mutex_multiprogramacion);
            multiprogramacion_disponible = multiprogramacion_disponible + 1;
            sem_post(&mutex_multiprogramacion);
            sem_post(&cambio_de_listas_largo);
        }
    }
}

void *salida_de_block(void *_){
    
    while(1){
        //sem_wait(&pedir_salida_de_block);
        sem_wait(&salida_block);

        bool encontrado = false;
        int tamanio_lista_blocked = list_size(lista_blocked);
        int tamanio_lista_s_blocked = list_size(lista_s_blocked);
        int index = 0;

        //Busco en block   
        while(!encontrado && (index < tamanio_lista_blocked)){
            
            t_proceso *aux = list_get(lista_blocked, index);
            if(aux->salida_block){
                printf("Mediano sacó de block al %d\n", aux->id);
                mover_proceso_de_lista(lista_blocked, lista_ready, index, READY);

                encontrado = true;
            }
            index ++;
        }

        index = 0;

        //Busco en suspended block
        while(!encontrado && (index < tamanio_lista_s_blocked)){

            t_proceso *aux = list_get(lista_s_blocked, index);
            
            if(aux->salida_block){
                
                mover_proceso_de_lista(lista_s_blocked, lista_s_ready, 0, READY);
                encontrado = true;
                
            }
            index ++;
        }
        
    }
    return NULL;
}