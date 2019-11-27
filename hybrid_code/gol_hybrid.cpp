#include "mpi.h"
#include <omp.h>
#include <time.h>
#include "libgol.h"


int main(int argc, char* argv[])
{
	//parameters
    int omp_threads= 12;
	int iterations = 100;		//iterations
	int xdim=30; //cartesian coordinates
	int ydim=30;
	int zdim=30;
	int*** board0;	// (n) generation
	int*** board1;	// (n+1) generation
	int*** stencil; // kernel
	int*** convMat;	// convolution matrix

    //object instantiation
    Generate gen;
    Utility util;
    
	//array initialization
	int aa,bb,cc,dd;	
	board0 = (int***)gen.malloc3D(xdim,ydim,zdim,sizeof(int),&aa);
	stencil = (int***)gen.malloc3D(3,3,3,sizeof(int),&bb);
	convMat = (int***)gen.malloc3D(xdim,ydim,zdim,sizeof(int),&cc);
	board1 = (int***)gen.malloc3D(xdim,ydim,zdim,sizeof(int),&dd);

    //MPI initialization
    int rank, size, ierr, root=0;
    double tstart, tend;
    MPI_Status status;
    MPI_Request request;
    MPI_Comm MPI_COMM_WORLD;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Barrier(MPI_COMM_WORLD);
    tstart = MPI_Wtime();

    //sanity check
    // Abort if more processors than the array dimension
    if(size > xdim)
    {
        if(rank == 0)
        {
            printf("Too many processes than required..\n");
        }
        MPI_Finalize();
        exit(1);
    }

    //the "partioned" section handled by each rank
    int*** splitboard0;	
    int*** splitboard1;
    int splitdim, chunkdim;
    int ee,ff;

    int toptag=1;
    int bottomtag=2;
    int** top1;
    int** top2;
    int** bottom1;
    int** bottom2;
    int gg,hh,ii,jj;
    top1 = (int**)gen.malloc2D(ydim,zdim,sizeof(int),&gg);
    top2 = (int**)gen.malloc2D(ydim,zdim,sizeof(int),&hh);
    bottom1 = (int**)gen.malloc2D(ydim,zdim,sizeof(int),&ii);
    bottom2 = (int**)gen.malloc2D(ydim,zdim,sizeof(int),&jj);


    splitdim = util.splitCells(xdim,rank,size);
    if (rank==root || rank==size-1)
    {
        chunkdim = splitdim+1;
    }
    else
    {
        chunkdim = splitdim+2;
    }   
    splitboard0 = (int***)gen.malloc3D(chunkdim,ydim,zdim,sizeof(int),&ee);
    splitboard1 = (int***)gen.malloc3D(chunkdim,ydim,zdim,sizeof(int),&ff);

	//simulation  
	gen.randGen(xdim,ydim,zdim,board0);	//board with random 0s and 1s 
	gen.stencilMat(stencil);	//generates the stencil matrix
    
    util.assignGrid(splitboard0, board0, xdim, ydim, zdim, splitdim, rank, size);
	
    for(int iter = 0; iter<iterations; ++iter)
	{
    	gen.simulate(chunkdim,ydim,zdim,splitboard1,splitboard0,stencil,convMat);
		splitboard0 = splitboard1;
        util.topExtract(top1,splitboard0,ydim,zdim,chunkdim,rank,size);
        util.bottomExtract(bottom1,splitboard0,ydim,zdim,chunkdim,rank,size);
        if(rank==root)
        {
            MPI_Send(&bottom1[0][0],(ydim*zdim),MPI_INT,1,toptag,MPI_COMM_WORLD);
            MPI_Recv(&bottom2[0][0],(ydim*zdim),MPI_INT,1,bottomtag,MPI_COMM_WORLD,&status);   
        }
        else if(rank==(size-1))
        {
            MPI_Send(&top1[0][0],(ydim*zdim),MPI_INT,(size-2),bottomtag,MPI_COMM_WORLD);
            MPI_Recv(&top2[0][0],(ydim*zdim),MPI_INT,(size-2),toptag,MPI_COMM_WORLD,&status);
        }
        else
        {
            MPI_Send(&top1[0][0],(ydim*zdim),MPI_INT,(rank-1),bottomtag,MPI_COMM_WORLD);
            MPI_Recv(&bottom2[0][0],(ydim*zdim),MPI_INT,(rank+1),bottomtag,MPI_COMM_WORLD,&status);
            MPI_Send(&bottom1[0][0],(ydim*zdim),MPI_INT,(rank+1),toptag,MPI_COMM_WORLD);
            MPI_Recv(&top2[0][0],(ydim*zdim),MPI_INT,(rank-1),toptag,MPI_COMM_WORLD,&status);
        }
        util.topAssign(splitboard0,top2,ydim,zdim,chunkdim,rank,size);         
        util.bottomAssign(splitboard0,top2,ydim,zdim,chunkdim,rank,size);
        MPI_Barrier(MPI_COMM_WORLD);    
	}

    MPI_Barrier(MPI_COMM_WORLD);
    tend = MPI_Wtime();
    if(rank==root){printf("Elapsed time for %d processes: %f seconds\n",size,(tend-tstart));}
    MPI_Finalize();
	return 0;
}

