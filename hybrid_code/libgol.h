#ifndef LIBGOL_H
#define LIBGOL_H


//_________________________________ CLASSES ___________________________________

class Generate
{
    private:
        int xdim, ydim, zdim;
    public:
        void** malloc2D(int xdim, int ydim, int type_size, int* arr_size)
        /*2D Memory allocation */
        {
            void **table;
            //size of stripe in y direction
            long ysize = ydim*type_size;
            //size of whole array
            long xsize = xdim*sizeof(void*) + (xdim*ysize);
            *arr_size = xsize;
            table = (void**)malloc(xsize);
            long i, j;
            long offset;
            
            //#pragma omp parallel for
            for (i=0;i<xdim;i++)
            {
                //int id = omp_get_thread_num();
                //location x[i] points to relative to first address of x
                offset = xdim*sizeof(void*) + i*ysize;
                // tell x[i] to point to offset
                //(char*) needed to make pointer have stride of 1
                table[i] =  (void*) ((char*)table+(offset));
            }
            return table;
        }
        //__________________________________________________________________________

        void*** malloc3D(int xdim, int ydim, int zdim, int type_size, int* arr_size)
        /*3D Memory allocation */
        {
            void ***table;
            //size of stripe in z direction
            long zsize = zdim*type_size;
            //size of y-z plane plus the pointers to the stripes
            long ysize = ydim*sizeof(void*) + (ydim*zsize);
            //size of whole array
            long xsize = xdim*sizeof(void**) + (xdim*ysize);
            *arr_size = xsize;
            table = (void***)malloc(xsize);
            long i, j;
            long offset;

            //#pragma omp parallel for
            for (i=0;i<xdim;i++)
            {
                //int id = omp_get_thread_num();
                //location x[i] points to relative to first address of x
                offset = xdim*sizeof(void**) + i*ysize;
                // tell x[i] to point to offset
                //(char*) needed to make pointer have stride of 1
                table[i] =  (void**) ((char*)table+(offset));
                for (j=0;j<ydim;j++)
                {
                    //printf("%i %i\n",i,j);
                    table[i][j]=(void*)((char*)table[i] +
                                            ydim*sizeof(void*) + j*zsize);
                }
            }
            return table;
        }
        //__________________________________________________________________________

        void randGen(int xdim, int ydim, int zdim, int ***board)
		/*Generates random 'ones' and 'zeros'*/
        {
            #pragma omp parallel for
			for(unsigned int i=0; i<xdim; ++i)
			{
        		for(unsigned int j=0; j<ydim; ++j)
				{
					for(unsigned int k=0; k<zdim; ++k)
					{
            			board[i][j][k] = rand() % 2;
        			}
        		}
    		}
        }
        //__________________________________________________________________________

        void stencilMat(int ***stencil)
		/*creates the stencil matrix*/
        {
            #pragma omp parallel for
			for (unsigned int i=0; i<3; ++i)
			{
				for (unsigned int j=0; j<3; ++j)
				{
					for (unsigned int k=0; k<3; ++k)
					{
						stencil[i][j][k] = 1;
					}
				}
			}
			stencil[1][1][1] = 0;
        }
        //__________________________________________________________________________

		void convOper(int xdim,int ydim,int zdim,int ***convMat,
                                            int ***step_n0,int ***stencil)
		/*performs convolution operation*/
		{
            int temp;
            int sum;
            int a;
            #pragma omp parallel for reduction(+: sum) private(temp) 
            for (int i=0; i<xdim; ++i)
			{
				for (int j=0; j<ydim; ++j)
				{
					for (int k=0; k<zdim; ++k)
					{    
                        for (int l=-1 ;l<2; ++l)
                        {
                            for (int m=-1 ;m<2; ++m)
                            {
                                for (int n=-1 ;n<2; ++n)
                                {
                                    a = stencil[l+1][m+1][n+1];
                                    if((i+l)<=-1 || (i+l)>=xdim) 
                                    {
                                        temp=0;
                                    }
                                    else if((j+m)<=-1 || (j+m)>=ydim)
                                    {
                                        temp=0;
                                    }
                                    else if((k+n) <= -1 || (k+n)>=zdim)
                                    {
                                        temp=0;
                                    }
                                    else
                                    {
                                        temp = (step_n0[i+l][j+m][k+n]);
                                    }
                                }
                            }
                        }
                        sum += (a * temp);
                        //sum += threadsum;
                        convMat[i][j][k] = sum;
					}
				}
			}
        }
        //__________________________________________________________________________

        int rules(int state, int myneighbCount)
        /*establishes rules for the board*/
		{
			int chance;
            if(state == 0)
            {
                if (myneighbCount == 6)     {chance = 1;}
    			else {chance = 0;}
            }
            if(state == 1)
            {
                if (myneighbCount < 7)      {chance = 1;}
    			else if (myneighbCount > 5) {chance = 1;}
    			else {chance = 0;}
            }
			return chance;
		}
        //__________________________________________________________________________

		void newMatx(int xdim,int ydim,int zdim,int ***step_n1,
                                        int ***step_n0,int ***convMat)
        /*Generates the next iteration based on assigned rules*/
		{
            #pragma omp parallel for
			for(unsigned int i=0; i<xdim; i++)
			{
				for(unsigned int j=0; j<ydim; j++)
				{
		    		for(unsigned int k=0; k<zdim; k++)
					{
						step_n1[i][j][k]=rules(step_n0[i][j][k],convMat[i][j][k]);
					}
				}
			}
		}
        //__________________________________________________________________________

		void simulate(int xdim,int ydim,int zdim,int ***step_n1,
                                int ***step_n0, int ***stencil,int ***convMat)
        /*Simulates the board*/
		{
            convOper(xdim,ydim,zdim,convMat,step_n0,stencil);   //convMat
            newMatx(xdim,ydim,zdim,step_n1,step_n0,convMat);    //step_n1
		}
};
//___________________________________________________________________________________
//___________________________________________________________________________________
        
class Utility
{
    private:
        int xdim,ydim,zdim;
    
    public:
		void assignGrid(int ***splitboard, int ***board, int xdim,int ydim, 
                            int zdim, int splitdim,int rank,int size)
        /*Assigns Grid structure based on rank*/
		{
            int a;
            int b;
            a = splitdim * rank;
            b = xdim-splitdim;
            if (rank==0)
            {
                #pragma omp parallel for
                for(unsigned int i=0; i<splitdim+1; ++i)
                {
                    for(unsigned int j=0; j<ydim; ++j)
                    {
                        for(unsigned int k=0;k<zdim; ++k)
                        {
                            splitboard[i][j][k] = board[i][j][k];
                        }
                    }
                }
            }
            else if (rank==size-1)
            {
                #pragma omp parallel for
                for(unsigned int i=0; i<splitdim+1; ++i)
                {
                    for(unsigned int j=0; j<ydim; ++j)
                    {
                        for(unsigned int k=0; k<zdim; ++k)
                        {
                            splitboard[i][j][k] = board[(b+i)-1][j][k];
                        }
                    }
                }
            }
            else
            {
                #pragma omp parallel for
                for(unsigned int i=0; i<splitdim+2; ++i)
                {
                    for(unsigned int j=0; j<ydim; ++j)
                    {
                        for(unsigned int k=0; k<zdim; ++k)
                        {
                            splitboard[i][j][k] = board[(a+i)-1][j][k];
                        }
                    }
                }
            }
		}
        //__________________________________________________________________________

        void stitchGrid(int ***board,int ***splitboard, int xdim, int ydim,
                            int zdim, int chunkdim,int rank,int size)
        /*Stitches grids together after simulation*/
        {
            int a = rank * int(xdim/size);
            int b = xdim-chunkdim;
            if (rank==0)
            {
                #pragma omp parallel for
                for(unsigned int i=0; i<chunkdim-1+1; ++i)
                {
                    for(unsigned int j=0 ; j<ydim; ++j)
                    {
                        for(unsigned int k=0; k<zdim; ++k)
                        {
                            board[i][j][k] = splitboard[i][j][k];
                        }
                    }
                }
            }
            else if (rank==size-1)
            {
                #pragma omp parallel for
                for(unsigned int i=1; i<chunkdim-1; ++i)
                {
                    for(unsigned int j=0; j<ydim; ++j)
                    {
                        for(unsigned int k=0; k<zdim; ++k)
                        {
                            board[(b+i)-1][j][k] = splitboard[i][j][k];
                        }
                    }
                }
            }
            else
            {
                #pragma omp parallel for
                for(unsigned int i=1; i<chunkdim-1; ++i)
                {
                    for(unsigned int j=0; j<ydim; ++j)
                    {
                        for(unsigned int k=0; k<zdim; ++k)
                        {
                            board[(a+i)-1][j][k] = splitboard[i][j][k];
                        }
                    }
                }
            }
        }
        //__________________________________________________________________________

        void topExtract(int **top,int ***splitboard, int ydim,int zdim, 
                        int chunkdim,int rank,int size)
        /*extracts the cell status of the top grid*/
        {
            #pragma omp parallel for
            for(unsigned int j=0; j<ydim; ++j)
            {
                for(unsigned int k=0; k<zdim; ++k)
                {
                    top[j][k] = splitboard[0][j][k];
                }
            }
        }
        //__________________________________________________________________________

        void bottomExtract(int **bottom,int ***splitboard, int ydim,int zdim, 
                        int chunkdim,int rank,int size)
        /*extracts the cell status of the bottom grid*/
        {
            #pragma omp parallel for
            for(unsigned int j=0; j<ydim; ++j)
            {
                for(unsigned int k=0; k<zdim; ++k)
                {
                    bottom[j][k] = splitboard[chunkdim-1][j][k];
                }
            }
        }
        //__________________________________________________________________________

        void topAssign(int ***splitboard, int **top, int ydim,int zdim, 
                        int chunkdim,int rank,int size)
        /*assigns the cell status of the top grid*/
        {
            #pragma omp parallel for
            for(unsigned int j=0; j<ydim; ++j)
            {
                for(unsigned int k=0; k<zdim; ++k)
                {
                    splitboard[0][j][k] = top[j][k];
                }
            }
        }
        //__________________________________________________________________________

        void bottomAssign(int ***splitboard, int **bottom, int ydim,int zdim, 
                        int chunkdim,int rank,int size)
        /*extracts the cell status of the bottom grid*/
        {
            #pragma omp parallel for
            for(unsigned int j=0; j<ydim; ++j)
            {
                for(unsigned int k=0; k<zdim; ++k)
                {
                    splitboard[chunkdim-1][j][k] = bottom[j][k];
                }
            }
        }
        //__________________________________________________________________________

        int splitCells(int xdim, int rank, int size)
        /*splits the grid structure*/
		{
            int splitdim;
            if (rank==size-1)
            {
                splitdim = (xdim/size) + (xdim%size);
            }
            else
            {
                splitdim = xdim/size;
            }
            return splitdim;
		}
        //__________________________________________________________________________
};

//__________________________________________________________________________________

#endif // LIB3D_H





