// Author: hariprasadr1hp
//Game of Life 2d simulator

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "libgol_serial.hpp"

using namespace std;

int main()
{
	//parameters
	int iterations = 3;		//iterations
	int xdim=5; //cartesian coordinates
	int ydim=5;
	int zdim=5;

	//initial conditions
	Generate gen;
	int*** board;	// initialize board
	int*** step_n0;	// (n) generation
	int*** step_n1;	// (n+1) generation
	int*** stencil; // kernel
	int*** convMat;	// convolution matrix
		

	int bb,cc,dd,ee;	//array initialization
	step_n0 = (int***)gen.malloc3D(xdim,ydim,zdim,sizeof(int),&bb);
	stencil = (int***)gen.malloc3D(3,3,3,sizeof(int),&cc);
	convMat = (int***)gen.malloc3D(xdim,ydim,zdim,sizeof(int),&dd);
	step_n1 = (int***)gen.malloc3D(xdim,ydim,zdim,sizeof(int),&ee);

	gen.randGen(xdim,ydim,zdim,step_n0);	//board with random 0s and 1s 
	gen.stencilMat(stencil);
	for(int iter = 0; iter<iterations ;iter++)
	{
    	gen.simulate(xdim,ydim,zdim,step_n1,step_n0,stencil,convMat);
		step_n0 = step_n1;
	}
	return 0;
}




	// cout<<step_n0[1][1][1]<<endl;
	// ///*
	// int count=0;
	// for(int i=0; i<xdim; i++)
	// {
	// 	for(int j=0; j<ydim; j++)
	// 	{
	// 		table[i][j] = 1;
	// 		for(int k=0; k<zdim; k++)
	// 		{
	// 			cout<<board[i][j][k];
	// 			cout<<convMat[i][j][k];
	// 			count++;
	// 			cout<<step_n0[i][j][k];
	// 			cout<<step_n1[i][j][k];
	// 			table[i][j][k] = rand() % 1;
	// 			table[i][j][k] = 1;
	// 		}
	// 	}
	// }//*/
	// cout<<'\n';
	// cout<<count<<endl;
	// cout<<board[i][j][k]<<endl;
	// cout<<stencil[1][1][1]<<endl;
	// cout<<convMat[i][j][k]<<endl;
	// cout<<step_n0[i][j][k]<<endl;
	// cout<<step_n1[i][j][k]<<endl;
