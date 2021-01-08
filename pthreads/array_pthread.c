/* NOTA IMPORTANTE
No entiendo el por qué, pero al establecer un tamaño de array muy grande y muy pocos hilos
los resultados son erróneos (p.e: TAM_ARRAY=250000000; NUM_HILOS=4).
Sin embargo, al regular ambas partes se pueden obtener resultados correctos
*/

#define TAM_ARRAY 250000000
#define NUM_HILOS 4
#define OPS TAM_ARRAY/NUM_HILOS

#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>

int array[TAM_ARRAY];

double tiempo_hilos[NUM_HILOS];

double suma = 0;
int indice = 0;

pthread_mutex_t mutexsum;

void* sumatorio(void* arg) {
    pthread_mutex_lock(&mutexsum);
    int hilo = indice++;
    pthread_mutex_unlock(&mutexsum);

    double suma_parcial = 0;

    struct timeval ti, tf;
    gettimeofday(&ti, NULL);

    for (long long i=0 ; i<OPS ; i++)
        suma_parcial += array[i+hilo*OPS];

    pthread_mutex_lock(&mutexsum);
    suma += suma_parcial;
    pthread_mutex_unlock(&mutexsum);

    gettimeofday(&tf, NULL);
    tiempo_hilos[hilo] = ((tf.tv_usec - ti.tv_usec)/1000.0 + ((double)(tf.tv_sec - ti.tv_sec)*1000)) / 1000.0;

    printf("Suma hilo %i: %.0f\n", hilo+1, suma_parcial);
    pthread_exit(NULL);
}

int main() {
    // Inicialización de hilos
    pthread_t hilos[NUM_HILOS];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&mutexsum, NULL);
    // Inicialización de array
    for (long long i=1 ; i<=TAM_ARRAY ; i++) {
        array[i-1] = i;
    }

    // Inicio cronómetro
    struct timeval t_ini, t_fin;
    double tiempo_sg;
    gettimeofday(&t_ini, NULL);

    // Llamada a función
    for (int i=0; i<NUM_HILOS; i++) {
        pthread_create(&hilos[i], &attr, sumatorio, NULL);
    }
    // Espera de la finalización de hilos
    for (int i=0; i<NUM_HILOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    // Fin cronómetro
    gettimeofday(&t_fin, NULL);

    //goto SALTAR_IMPRESION;

    // Impresión de resultados
    printf("Suma: %.0f\n", suma);

    SALTAR_IMPRESION:
    // Impresión del tiempo que ha tardado en la ejecución de la función
    tiempo_sg = ((t_fin.tv_usec - t_ini.tv_usec)/1000.0 + ((double)(t_fin.tv_sec - t_ini.tv_sec)*1000)) / 1000.0;
    printf("Tiempo: %f s\n", tiempo_sg);

    pthread_mutex_destroy(&mutexsum);
    pthread_exit(NULL);
}
