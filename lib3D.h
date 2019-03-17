#ifndef LIB3D_H
#define LIB3D_H
#include <fstream>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#define MAX_SIZE 256
using namespace std;

//################################# CLASSES ###################################
class Generate
{
    private:
        int xdim, ydim, zdim;
    public:
        void*** malloc3D(int xdim,int ydim,int zdim,int type_size,int* arr_size)
        {
            void ***table;
            //size of stripe in z direction
            long zsize = zdim*type_size;
            //size of y-z plane plus the pointers to the stripes
            long ysize = ydim*sizeof(void*) + ydim*zsize;
            //size of whole array
            long xsize = xdim*sizeof(void**) + xdim*ysize;
            *arr_size = xsize;
            table = (void***)malloc(xsize);
            long i, j;
            long offset;
            for (i=0;i<xdim;i++)
            {
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
        //#####################################################################
        void randGen(int xdim, int ydim, int zdim, int ***board)
		/*Generates random 'ones' and 'zeros'*/
        {
			for(int i=0; i<xdim; i++)
			{
        		for(int j=0; j<ydim; j++)
				{
					for(int k=0; k<zdim; k++)
					{
            			board[i][j][k] = rand() % 2;
        			}
        		}
    		}
        }
        //#####################################################################
        void stencilMat(int ***stencil)
		/*creates the stencil matrix*/
        {
			for (int i=0; i<3; i++)
			{
				for (int j=0; j<3; j++)
				{
					for (int k=0; k<3; k++)
					{
						stencil[i][j][k] = 1;
					}
				}
			}
			stencil[1][1][1] = 0;
        }
        //#####################################################################
		void convOper(int xdim,int ydim,int zdim,int ***convMat,
                                            int ***step_n0,int ***stencil)
		/*performs convolution operation*/
		{
            for (int i=0; i<xdim; i++)
			{
				for (int j=0; j<ydim; j++)
				{
					for (int k=0; k<zdim; k++)
					{
						int temp=0;
                        int sum=0;
						for (int l=-1 ;l<2; l++)
						{
                            for (int m=-1 ;m<2; m++)
                            {
						 		for (int n=-1 ;n<2; n++)
						 		{
                                    if((i+l) <= -1)         {temp=0;}
                                    else if((i+l) >= xdim)  {temp=0;}
                                    else if((j+m) <= -1)    {temp=0;}
                                    else if((j+m) >= ydim)  {temp=0;}
                                    else if((k+n) <= -1)    {temp=0;}
                                    else if((k+n) >= zdim)  {temp=0;}
                                    else{temp = (step_n0[i+l][j+m][k+n]);}
                                    sum += (stencil[l+1][m+1][n+1]) * temp;
                                }
                            }
                        }
                        convMat[i][j][k] = sum;
					}
				}
			}
        }
        //#####################################################################
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
                if (myneighbCount > 7)      {chance = 1;}
    			else if (myneighbCount > 5) {chance = 1;}
    			else {chance = 0;}
            }
			return chance;
		}
        //#####################################################################
		void newmatx(int xdim,int ydim,int zdim,int ***step_n1,
                                        int ***step_n0,int ***convMat)
        /*Generates the next iteration based on assigned rules*/
		{
			//int count, state;
			for(int i=0; i<xdim; i++)
			{
				for(int j=0; j<ydim; j++)
				{
		    		for(int k=0; k<zdim; k++)
					{
						step_n1[i][j][k]=rules(step_n0[i][j][k],convMat[i][j][k]);
					}
				}
			}
		}
        //#####################################################################
		void simulate(int xdim,int ydim,int zdim,int ***step_n1,
                                int ***step_n0, int ***stencil,int ***convMat)
        /*Simulates the board*/
		{
            convOper(xdim,ydim,zdim,convMat,step_n0,stencil);   //convMat
            newmatx(xdim,ydim,zdim,step_n1,step_n0,convMat);    //step_n1
		}
};
//#############################################################################
#endif // LIB3D_H
