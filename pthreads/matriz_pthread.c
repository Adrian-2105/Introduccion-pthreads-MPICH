#define TAM_MATRIZ 500
#define NUM_HILOS 4
#define OPS TAM_MATRIZ/NUM_HILOS

#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>

double A[TAM_MATRIZ][TAM_MATRIZ];
double B[TAM_MATRIZ][TAM_MATRIZ];
double C[TAM_MATRIZ][TAM_MATRIZ];

double tiempo_hilos[NUM_HILOS];

int indice = 0;

pthread_mutex_t mutexsum;

void* mult(void* arg) {
    pthread_mutex_lock(&mutexsum);
    int hilo = indice++;
    pthread_mutex_unlock(&mutexsum);

    struct timeval ti, tf;
    gettimeofday(&ti, NULL);

    double aux;
    for (int i=hilo*OPS; i<(hilo+1)*OPS; i++) {
        for (int j=0; j<TAM_MATRIZ; j++) {
            aux = 0;
            for (int k=0; k<TAM_MATRIZ; k++)
                aux += B[i][k] * C[k][j];
            A[i][j] = aux; 
        }
    }

    gettimeofday(&tf, NULL);
    tiempo_hilos[hilo] = ((tf.tv_usec - ti.tv_usec)/1000.0 + ((double)(tf.tv_sec - ti.tv_sec)*1000)) / 1000.0;

    pthread_exit(NULL);
}

int main() {
    // Creación de hilos y argumentos
    pthread_t hilos[NUM_HILOS];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&mutexsum, NULL);
    // Inicialización de matrices
    for (int i=0; i<TAM_MATRIZ; i++) {
        for (int j=0; j<TAM_MATRIZ; j++) {
            B[i][j] = i+j;
            C[i][j] = i+j;
        }
    }
    // Inicio cronómetro
    struct timeval t_ini, t_fin;
    double tiempo_sg;
    gettimeofday(&t_ini, NULL);

    // Multiplicación: A = B x C
    for (int i=0; i<NUM_HILOS; i++)
        pthread_create(&hilos[i], &attr, mult, NULL);
    // Esperamos a la finalización de todos los hilos
    for (int i=0; i<NUM_HILOS; i++)
        pthread_join(hilos[i], NULL);

    // Fin cronómetro
    gettimeofday(&t_fin, NULL);

    goto SALTAR_IMPRESION;

    // Impresión de la matriz
    for (int i=0; i<TAM_MATRIZ; i++) {
        for (int j = 0; j < TAM_MATRIZ; j++) {
            printf("%i ", (int) A[i][j]);
        }
        printf("\n");
    }

    SALTAR_IMPRESION:
    // Impresión del tiempo que ha tardado en la ejecución de la función
    tiempo_sg = ((t_fin.tv_usec - t_ini.tv_usec)/1000.0 + ((double)(t_fin.tv_sec - t_ini.tv_sec)*1000)) / 1000.0;
    printf("Tiempo: %f s\n", tiempo_sg);
    
    for (int i=0; i<NUM_HILOS; i++)
        printf("Tiempo hilo %i: %f s\n", i+1, tiempo_hilos[i]);
        /* NOTA: no se está contabilizando el tiempo del último hilo, pero los resultados son correctos*/

    pthread_mutex_destroy(&mutexsum);
    pthread_exit(NULL);
}
