#define TAM_MATRIZ 500
// todas las matrices serán de tamaño TAM_MATRIZ x TAM_MATRIZ

#include "mpi.h"
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
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

    /****************** Código del proceso controlador ******************/
    if (taskid == MASTER) {
        printf("Ejecución iniciada con %d procesos\n", numtasks);
        // Inicialización de las matrices a multiplicar
        int A[TAM_MATRIZ][TAM_MATRIZ];
        int B[TAM_MATRIZ][TAM_MATRIZ];
        double suma = 0;
        for (int i = 0; i < TAM_MATRIZ; i++)
            for (int j = 0; j < TAM_MATRIZ; j++) {
                A[i][j] = i + j;
                B[i][j] = abs(i - j);
            }

        // Inicio cronómetro
        struct timeval t_ini, t_fin;
        double tiempo_sg;
        gettimeofday(&t_ini, NULL);

        // Mandar a cada proceso las submatrices correspondientes
        int FILAS = TAM_MATRIZ;
        int COLUMNAS = TAM_MATRIZ;
        int div_filas = TAM_MATRIZ / numworkers;
        for (int i = 0; i < numworkers; i++) {
            int ind = div_filas * i;
            int dest = i + 1;
            // Matriz A parcial
            MPI_Send(&div_filas, 1, MPI_INT, dest, TO_WORKER, MPI_COMM_WORLD);
            MPI_Send(&COLUMNAS, 1, MPI_INT, dest, TO_WORKER, MPI_COMM_WORLD);
            MPI_Send(&A[ind], div_filas*COLUMNAS, MPI_INT, dest, TO_WORKER, MPI_COMM_WORLD);

            // Matriz B completa
            MPI_Send(&FILAS, 1, MPI_INT, dest, TO_WORKER, MPI_COMM_WORLD);
            MPI_Send(&COLUMNAS, 1, MPI_INT, dest, TO_WORKER, MPI_COMM_WORLD);
            MPI_Send(B, FILAS*COLUMNAS, MPI_INT, dest, TO_WORKER, MPI_COMM_WORLD);
        }

        // Construimos la matriz resultado y la recibimos
        int C[TAM_MATRIZ][TAM_MATRIZ];
        double aux;
        for (int i = 0; i < numworkers; i++) {
            int ind = div_filas * i;
            int source = i + 1;
            MPI_Recv(&C[ind], div_filas*COLUMNAS, MPI_INT, source, TO_MASTER, MPI_COMM_WORLD, &status);
        }

        // Fin cronómetro
        gettimeofday(&t_fin, NULL);

        // Impresión de resultados
#ifdef PRINT
        printf("\nA\n----\n");
        for (int i = 0; i < TAM_MATRIZ; i++) {
            for (int j = 0; j < TAM_MATRIZ; j++)
                printf("%d ", A[i][j]);
            printf("\n");
        }

        printf("\nB\n----\n");
        for (int i = 0; i < TAM_MATRIZ; i++) {
            for (int j = 0; j < TAM_MATRIZ; j++)
                printf("%d ", B[i][j]);
            printf("\n");
        }

        printf("\nMPI\n----\n");
        for (int i = 0; i < TAM_MATRIZ; i++) {
            for (int j = 0; j < TAM_MATRIZ; j++)
                printf("%d ", C[i][j]);
            printf("\n");
        }

        printf("\nIterativo\n--------");
        for (int i=0; i < TAM_MATRIZ; i++) {
            for (int j=0; j < TAM_MATRIZ; j++) {
                aux = 0;
                for (int k=0; k < TAM_MATRIZ; k++)
                    aux += A[i][k] * B[k][j];
                C[i][j] = aux;
            }
        }

        printf("\n");
        for (int i = 0; i < TAM_MATRIZ; i++) {
            for (int j = 0; j < TAM_MATRIZ; j++)
                printf("%d ", C[i][j]);
            printf("\n");
        }
#endif

        // Impresión del tiempo que ha tardado en la ejecución de la función
        tiempo_sg = ((t_fin.tv_usec - t_ini.tv_usec)/1000.0 + ((double)(t_fin.tv_sec - t_ini.tv_sec)*1000)) / 1000.0;
        printf("Tiempo: %f s\n", tiempo_sg);
    }


        /****************** Código de los trabajadores ******************/
    else if (taskid > MASTER) {
        int FILAS_A, COLUMNAS_A, FILAS_B, COLUMNAS_B;
        // Recibimos la matriz A parcial
        MPI_Recv(&FILAS_A, 1, MPI_INT, MASTER, TO_WORKER, MPI_COMM_WORLD, &status);
        MPI_Recv(&COLUMNAS_A, 1, MPI_INT, MASTER, TO_WORKER, MPI_COMM_WORLD, &status);
        int A[FILAS_A][COLUMNAS_A];
        MPI_Recv(A, FILAS_A*COLUMNAS_A, MPI_INT, MASTER, TO_WORKER, MPI_COMM_WORLD, &status);
        // Recibimos la matriz B completa
        MPI_Recv(&FILAS_B, 1, MPI_INT, MASTER, TO_WORKER, MPI_COMM_WORLD, &status);
        MPI_Recv(&COLUMNAS_B, 1, MPI_INT, MASTER, TO_WORKER, MPI_COMM_WORLD, &status);
        int B[FILAS_B][COLUMNAS_B];
        MPI_Recv(B, FILAS_B*COLUMNAS_B, MPI_INT, MASTER, TO_WORKER, MPI_COMM_WORLD, &status);

        // Calculamos su parte correspondiente
        int C[FILAS_A][COLUMNAS_B];
        double aux;
        for (int i=0; i < FILAS_A; i++) {
            for (int j=0; j < COLUMNAS_B; j++) {
                aux = 0;
                for (int k=0; k < FILAS_B; k++)
                    aux += A[i][k] * B[k][j];
                C[i][j] = aux;
            }
        }

        // Enviamos el resultado
        //MPI_Send(&FILAS_A, 1, MPI_INT, MASTER, TO_MASTER, MPI_COMM_WORLD);
        //MPI_Send(&COLUMNAS_B, 1, MPI_INT, MASTER, TO_MASTER, MPI_COMM_WORLD);
        MPI_Send(C, FILAS_A*COLUMNAS_B, MPI_INT, MASTER, TO_MASTER, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}
