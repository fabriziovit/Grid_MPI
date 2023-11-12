#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>

MPI_Comm create_grid(int row, int nproc, int menum, int *coordinate);

void strategy1(int nloc, int rest, int menum, int *elements_loc, int *elements, int tag, MPI_Comm comm_grid, int nproc, MPI_Status status);
void strategy2(int *elements, int nloc, int *elements_loc);

int main(int argc, char **argv)
{
    int nproc, menum, row, col;
    int dim, *ndim, reorder, *period, *coordinate;
    MPI_Comm comm_grid;
    int N, *elements, nloc, rest, *elements_loc;
    int tag;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &menum);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    if (menum == 0)
    {
        if (argc != 3)
        {
            printf("Devi fornire il numero di righe della griglia come argomento!\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    row = atoi(argv[1]);

    if (nproc % row != 0)
    {
        MPI_Abort(MPI_COMM_WORLD, 1);
        MPI_Finalize();
        return 0;
    }

    coordinate = (int *)calloc(2, sizeof(int));

    comm_grid = create_grid(row, nproc, menum, coordinate);

    if (menum == 0)
    {
        // convert to integer the number to sum and strategy to apply
        N = atoi(argv[2]);

        elements = (int *)malloc(sizeof(int) * N);
        if (elements == NULL)
        {
            fprintf(stderr, "Errore nell'allocazione della memoria per l'array 'elements'!\n");
            return EXIT_FAILURE;
        }

        srand(time(NULL));

        printf("Allocazione numeri all'interno del vettore\n");

        for (int i = 0; i < N; i++)
        {
            elements[i] = 1;
        }
    }

    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    nloc = N / nproc;
    rest = N % nproc;

    if (menum < rest)
    {
        nloc = nloc + 1;
    }

    // allocation of local array for each processor
    elements_loc = (int *)malloc(sizeof(int) * nloc);
    if (elements_loc == NULL)
    {
        fprintf(stderr, "Errore nell'allocazione della memoria per l'array 'elements'!\n");
        return EXIT_FAILURE;
    }

    if (N % nproc != 0)
    {
        strategy1(nloc, rest, menum, elements_loc, elements, tag, comm_grid, nproc, status);
    }
    else
    {
        strategy2(elements, nloc, elements_loc);
    }

    printf("Sono il processore %d è e con coordinate (%d, %d) ed ho i numeri ed ho %d numeri:\n", menum, coordinate[0], coordinate[1], nloc);
    for (int i = 0; i < nloc; i++)
    {
        printf("%d\n", elements_loc[i]);
    }
    printf("\n\n");

    MPI_Finalize();
    return 0;
}

MPI_Comm create_grid(int row, int nproc, int menum, int *coordinate)
{
    int col;
    int dim, *ndim, reorder, *period;
    MPI_Comm comm_grid;

    MPI_Bcast(&row, 1, MPI_INT, 0, MPI_COMM_WORLD);

    col = nproc / row;

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

void strategy1(int nloc, int rest, int menum, int *elements_loc, int *elements, int tag, MPI_Comm comm_grid, int nproc, MPI_Status status)
{
    if (menum == 0)
    {
        for (int i = 0; i < nloc; i++)
        {
            elements_loc[i] = elements[i];
        }

        int tmp = nloc;
        int start = 0;
        for (int i = 1; i < nproc; i++)
        {
            start += tmp;
            tag = 22 + i;
            if (i == rest)
                tmp -= 1;

            MPI_Send(&elements[start], tmp, MPI_INT, i, tag, comm_grid);
        }
    }
    else
    {
        tag = 22 + menum;
        MPI_Recv(elements_loc, nloc, MPI_INT, 0, tag, comm_grid, &status);
    }
}

void strategy2(int *elements, int nloc, int *elements_loc)
{
    MPI_Scatter(elements, nloc, MPI_INT, elements_loc, nloc, MPI_INT, 0, MPI_COMM_WORLD);
}
