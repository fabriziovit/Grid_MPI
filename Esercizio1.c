#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int main(int argc, char **argv)
{
    int nproc, menum, row, col;
    int dim, *ndim, reorder, *period, *coordinate;
    MPI_Comm comm_grid;

    row = atoi(argv[1]);

    MPI_Init(&argc, &argv);
    // MPI_Comm_rank(MPI_COMM_WORLD, &menum);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    if (nproc % row != 0)
    {
        MPI_Abort(MPI_COMM_WORLD, 1);
        MPI_Finalize();
        return 0;
    }

    MPI_Bcast(&row, 1, MPI_INT, 0, MPI_COMM_WORLD);

    col = nproc / row;

    dim = 2;
    coordinate = (int *)calloc(dim, sizeof(int));

    ndim = (int *)calloc(dim, sizeof(int));
    ndim[0] = row;
    ndim[1] = col;

    period = (int *)calloc(dim, sizeof(int));
    period[0] = period[1] = 0;
    reorder = 0;

    MPI_Cart_create(MPI_COMM_WORLD, dim, ndim, period, reorder, &comm_grid); // creazione griglia

    MPI_Comm_rank(comm_grid, &menum);

    MPI_Cart_coords(comm_grid, menum, 2, coordinate); // assegnazione coordinate dei nodi

    printf("Processo %d: coordinate (%d, %d)\n", menum, coordinate[0], coordinate[1]);
    MPI_Finalize();

    return 0;
}

/*void crea_griglia_bi (MPI_Comm *griglia, MPI_Comm *grigliar, MPI_Comm *grigliac, int menum, int nproc,  int riga, int col, int *coordinate){
    int dim=2, ndim[2], reorder, period[2], vc[2];
    //ndim = (int*) calloc (dim, sizeof(int));
    ndim[0] = riga;
    //si suppone che il controllo sulla divisione di nproc per riga sia stato fatto fuori, altrimenti invece di col si deve passare in input nproc
    ndim[1] = col;
    //period = (int*) calloc (dim, sizeof(int));
    period[0] = period [1] = 0;
    reorder = 0;
    //si suppone che coordinate sia stato allocato fuori dalla funzione, altrimenti deve essere allocato qui
    MPI_Cart_create(MPI_COMM_WORLD, dim, ndim, period, reorder, griglia);
    // creazione griglia
    MPI_Cart_coords (*griglia, menum, 2, coordinate);
    // assegnazione coordinate dei nodi
    vc[0] = 0;
    vc[1] = 1;
    MPI_Cart_sub(*griglia, vc, grigliar);
    // divisione in righe del communicator: ho cancellato i
    collegamenti sulla dimensione 0, cioè tra le righe, comunicano
    solo all’interno delle singole righe
    vc[0] = 1;
    vc[1] = 0;
    MPI_Cart_sub(*griglia, vc, grigliac);
    // divisione in colonne del comunicator: ho cancellato i collegamenti sulla dimensione 1, cioè tra le colonne, comunicano solo all’interno delle singole colonne
}*/