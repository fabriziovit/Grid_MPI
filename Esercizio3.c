#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

typedef struct
{
    int row_start;
    int row_end;
    int col_start;
    int col_end;
} Subblock;

MPI_Comm create_grid(int px, int q, int nproc);
void get_subblock_coordinates(int px, int q, int menum, int *k, int *l);
Subblock get_subblock(int px, int q, int k, int l, int N, int M);

int main(int argc, char **argv)
{
    int nproc, menum, row, col;
    int dim, *ndim, reorder, *period, *coordinate;
    MPI_Comm comm_grid;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &menum);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    if (menum == 0)
    {
        if (argc != 5)
        {
            printf("Devi fornire il numero di righe e colonne della matrice, e il numero di colonne della griglia!\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    int N = atoi(argv[1]); // Numero di righe della matrice
    int M = atoi(argv[2]); // Numero di colonne della matrice
    int p = atoi(argv[3]); // Numero di righe della griglia
    int q = atoi(argv[4]); // Numero di colonne della griglia

    if (nproc != p * q)
    {
        printf("Il numero di processi deve essere p x q!\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Creazione della griglia p x q
    comm_grid = create_grid(p, q, nproc);

    // Calcolo delle coordinate del processo nella griglia
    coordinate = (int *)calloc(2, sizeof(int));
    MPI_Cart_coords(comm_grid, menum, 2, coordinate);

    int k, l;
    get_subblock_coordinates(p, q, menum, &k, &l);

    Subblock subblock = get_subblock(p, q, k, l, N, M);

    printf("Processo %d: Sottoblocco rettangolare con coordinate (%d, %d) - (%d, %d)\n",
           menum, subblock.row_start, subblock.col_start, subblock.row_end, subblock.col_end);

    MPI_Finalize();
    return 0;
}

MPI_Comm create_grid(int p, int q, int nproc)
{
    int dim, *ndim, reorder, *period;
    MPI_Comm comm_grid;

    int col = nproc / p;

    dim = 2;

    ndim = (int *)calloc(dim, sizeof(int));
    ndim[0] = p;
    ndim[1] = q;

    period = (int *)calloc(dim, sizeof(int));
    period[0] = period[1] = 0;
    reorder = 0;

    MPI_Cart_create(MPI_COMM_WORLD, dim, ndim, period, reorder, &comm_grid);

    return comm_grid;
}

void get_subblock_coordinates(int p, int q, int menum, int *k, int *l)
{
    *k = menum / q;
    *l = menum % q;
}

Subblock get_subblock(int p, int q, int k, int l, int N, int M)
{
    Subblock subblock;
    int rows_per_block = N / p;
    int columns_per_block = M / q;

    subblock.row_start = k * rows_per_block;
    subblock.row_end = (k + 1) * rows_per_block - 1;

    subblock.col_start = l * columns_per_block;
    subblock.col_end = (l + 1) * columns_per_block - 1;

    return subblock;
}
