#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>

MPI_Comm create_grid(int N, int nproc, int menum, int *coordinate);

int main(int argc, char **argv)
{
    int nproc, menum, N = 6, dimtot;
    int dim, *ndim, reorder, *period, *coordinate;
    MPI_Comm comm_grid;
    int *elements, nloc, rest, *elements_loc;
    int tag;
    MPI_Status status;
    int *x, *y, *x_loc;
    int row = 1, row_rest;
    int i, j;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &menum);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    /*if (menum == 0)
    {
        if (argc != 2)
        {
            printf("Devi fornire il numero di righe della griglia come argomento!\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        N = atoi(argv[2]);
    }*/

    // N = atoi(argv[2]);

    /*if (nproc % N != 0)
    {
        MPI_Abort(MPI_COMM_WORLD, 1);
        MPI_Finalize();
        return 0;
    }*/

    coordinate = (int *)calloc(2, sizeof(int));

    // MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    comm_grid = create_grid(2, nproc, menum, coordinate);

    if (menum == 0)
    {
        // convert to integer the number to sum and strategy to apply
        // N = atoi(argv[2]);
        dimtot = N * N;

        elements = (int *)malloc(sizeof(int) * dimtot);
        x = (int *)malloc(sizeof(int) * N);

        if (elements == NULL && x == NULL)
        {
            fprintf(stderr, "Errore nell'allocazione della memoria per l'array 'elements'!\n");
            return EXIT_FAILURE;
        }

        srand(time(NULL));

        printf("Inserimento numeri nella matrice e nel vettore...\n");

        for (i = 0; i < dimtot; i++)
        {
            elements[i] = 1 + i;
        }
        for (i = 0; i < N; i++)
        {
            x[i] = i + 2;
        }

        printf("Matrice\n");
        for (i = 0; i < N; i++)
        {
            for (j = 0; j < N; j++)
            {
                printf("%d  ", elements[i * N + j]);
            }
            printf("\n");
        }
        printf("\n");

        printf("Vettore\n");
        for (i = 0; i < N; i++)
        {
            printf("%d  ", x[i]);
        }
        printf("\n\n");
    }

    MPI_Bcast(&dimtot, 1, MPI_INT, 0, comm_grid);
    MPI_Bcast(&N, 1, MPI_INT, 0, comm_grid);

    nloc = N;
    rest = N % nproc;

    // row_rest = dimtot % nproc;

    /*if (menum < row_rest)
    {
        row = row + 1;
    }*/

    y = (int *)malloc(sizeof(int) * row);

    // y[0] = 0;

    if (menum < rest)
    {
        // nloc = nloc + 1;
        row = row + 1;
        nloc = N * row;
    }

    for (i = 0; i < row; i++)
    {
        y[i] = 0;
    }

    // allocation of local array for each processor
    elements_loc = (int *)malloc(sizeof(int) * nloc);
    x_loc = (int *)malloc(sizeof(int) * N);

    if (elements_loc == NULL)
    {
        fprintf(stderr, "Errore nell'allocazione della memoria per l'array 'elements'!\n");
        return EXIT_FAILURE;
    }

    if (menum == 0)
    {

        for (i = 0; i < nloc; i++)
        {
            elements_loc[i] = elements[i];
        }
        x_loc = x;

        int tmp = nloc;
        int start = 0;
        for (i = 1; i < nproc; i++)
        {
            start += tmp;
            tag = 22 + i;
            if (i == rest)
                tmp = tmp / row;

            MPI_Send(&elements[start], tmp, MPI_INT, i, tag, comm_grid);
            MPI_Send(x, N, MPI_INT, i, tag + 5, comm_grid);
        }
    }
    else
    {
        tag = 22 + menum;
        MPI_Recv(elements_loc, nloc, MPI_INT, 0, tag, comm_grid, &status);
        MPI_Recv(x_loc, N, MPI_INT, 0, tag + 5, comm_grid, &status);
    }

    for (i = 0; i < row; i++)
    {
        for (j = 0; j < nloc / row; j++)
        {
            y[i] = y[i] + elements_loc[j + (i * (nloc / row))] * x_loc[j];
        }
    }

    printf("Risultato del processore %d:\n", menum);
    for (i = 0; i < row; i++)
    {
        printf("%d ", y[i]);
    }
    printf("\n");

    MPI_Finalize();
    return 0;
}

MPI_Comm create_grid(int row, int nproc, int menum, int *coordinate)
{
    int col;
    int dim, *ndim, reorder, *period;
    MPI_Comm comm_grid;

    MPI_Bcast(&row, 1, MPI_INT, 0, MPI_COMM_WORLD);

    col = nproc / 2;

    dim = 2;

    ndim = (int *)calloc(dim, sizeof(int));
    ndim[0] = row;
    ndim[1] = col;

    period = (int *)calloc(dim, sizeof(int));
    period[0] = period[1] = 0;
    reorder = 0;

    MPI_Cart_create(MPI_COMM_WORLD, dim, ndim, period, reorder, &comm_grid); // creazione griglia

    MPI_Comm_rank(comm_grid, &menum);

    MPI_Cart_coords(comm_grid, menum, 2, coordinate); // assegnazione coordinate dei nodi

    return comm_grid;
}
