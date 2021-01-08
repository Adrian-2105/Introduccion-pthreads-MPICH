// IMPORTANTE: (número de procesos trabajadores - 1) % TAM_ARRAY == 0 para que funcione correctamente

#define TAM_ARRAY 800000 // no me permite un tamaño mayor en mi torre
// Para 250000000 el resultado debe ser 31250000125000000

#include "mpi.h"
#include <stdio.h>
#include <sys/time.h>
#define ll long long
#define MASTER 0
#define TO_WORKER 1
#define TO_MASTER 2

int main(int argc, char* argv[]) {
    // Variables relacionadas con el uso de MPI
    int numtasks, numworkers, taskid;

    // Inicialización de MPI
    MPI_Status status;
    MPI_Init(&argc, &argv);
    // Obtenemos el número de procesos que se están ejecutando simultáneamente
    // para la resolución del problema
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    numworkers = numtasks - 1;
    // Obtenemos el identificador de este proceso
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);

    if (TAM_ARRAY % numworkers != 0) {
        printf("El numero de trabajadores debe ser divisor de TAM_ARRAY\n");
        MPI_Finalize();
        return 0;
    }

    /****************** Código del proceso controlador ******************/
    if (taskid == MASTER) {
        printf("Ejecución iniciada con %d procesos\n", numtasks);
        // Inicialización del array
        ll array[TAM_ARRAY];
        double suma = 0;
        for (ll i = 0 ; i < TAM_ARRAY ; i++)
            array[i] = i + 1;

        // Inicio cronómetro
        struct timeval t_ini, t_fin;
        double tiempo_sg;
        gettimeofday(&t_ini, NULL);

        // Mandar a cada proceso el array
        ll SIZE = TAM_ARRAY / numworkers;
        for (int i = 0; i < numworkers; i++) {
            int dest = i + 1;
            MPI_Send(&SIZE, 1, MPI_LONG_LONG_INT, dest, TO_WORKER, MPI_COMM_WORLD);
            MPI_Send(&array[i * SIZE], SIZE, MPI_LONG_LONG_INT, dest, TO_WORKER, MPI_COMM_WORLD);
        }

        double aux;
        for (int i = 0; i < numworkers; i++) {
            int source = i + 1;
            MPI_Recv(&aux, 1, MPI_DOUBLE, source, TO_MASTER, MPI_COMM_WORLD, &status);
            suma += aux;
        }

        // Fin cronómetro
        gettimeofday(&t_fin, NULL);

        // Impresión de resultados
        printf("Suma: %.0f\n", suma);
        double num = ((double) TAM_ARRAY * ((double) TAM_ARRAY + 1)) / 2;
        printf("Suma Euler: %.0f\n", num);

        // Impresión del tiempo que ha tardado en la ejecución de la función
        tiempo_sg = ((t_fin.tv_usec - t_ini.tv_usec)/1000.0 + ((double)(t_fin.tv_sec - t_ini.tv_sec)*1000)) / 1000.0;
        printf("Tiempo: %f s\n", tiempo_sg);
    }


    /****************** Código de los trabajadores ******************/
    else if (taskid > MASTER) {
        // Recibimos el array
        ll SIZE;
        MPI_Recv(&SIZE, 1, MPI_LONG_LONG_INT, MASTER, TO_WORKER, MPI_COMM_WORLD, &status);
        ll array[SIZE];
        MPI_Recv(array, SIZE, MPI_LONG_LONG_INT, MASTER, TO_WORKER, MPI_COMM_WORLD, &status);
        // Calculamos su parte correspondiente
        double suma = 0;
        for (ll i = 0; i < SIZE; i++)
            suma += array[i];
        // Enviamos el resultado
        printf("Mi suma: %.0f\n", suma);
        MPI_Send(&suma, 1, MPI_DOUBLE, MASTER, TO_MASTER, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}
