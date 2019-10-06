/*********************************************************************
 DMP created in 2019.09
 *********************************************************************/

#include "dmp/fourier_approx.h"
#include<stdio.h>
using namespace Eigen;
using namespace std;

namespace dmp{

FourierApprox::FourierApprox(int order)
{
	n_bases = order + 1;	 //Univariate Fourier has order-many bases plus a constant basis
	features = new double[n_bases];
	weights.resize(n_bases);
	for(int i=0; i<n_bases; i++){
		features[i] = 0;
	}
}


FourierApprox::FourierApprox(const vector<double> &w)
{
	weights = w;
	n_bases = w.size();
	features = new double[n_bases];
	for(int i=0; i<n_bases; i++){
		features[i] = 0;
	}
}


FourierApprox::~FourierApprox()
{
	delete[] features;
}


double FourierApprox::evalAt(double x)
{
	calcFeatures(x);

	double wsum = 0;
	for(int i=0; i<n_bases; i++){
		wsum += features[i] * weights[i];
	}
	return wsum;
}


void FourierApprox::leastSquaresWeights(double *X, double *Y, int n_pts)
{
	MatrixXd D_mat = MatrixXd(n_pts,n_bases);
	MatrixXd Y_mat = MatrixXd(n_pts,1);

	//Calculate the design matrix
	for(int i=0; i<n_pts; i++){
		Y_mat(i,0) = Y[i];
		calcFeatures(X[i]);
		for(int j=0; j<n_bases; j++){
			D_mat(i,j) = features[j];
		}
	}

	//Calculate the least squares weights via projection onto the basis functions
	MatrixXd w = pseudoinverse(D_mat.transpose() * D_mat) * D_mat.transpose() * Y_mat;
	for(int i=0; i<n_bases; i++){
		weights[i] = w(i,0);
	}
}


void FourierApprox::calcFeatures(double x)
{
	for(int i=0; i<n_bases; i++){
		features[i] = cos(PI*i*x);
	}
}


MatrixXd FourierApprox::pseudoinverse(MatrixXd mat){
	//Numpy uses 1e-15 by default.  I use 1e-10 just to be safe.
	double precisionCutoff = 1e-10;

	//Compute the SVD of the matrix
	JacobiSVD<MatrixXd> svd(mat, ComputeThinU | ComputeThinV);
	MatrixXd U = svd.matrixU();
	MatrixXd V = svd.matrixV();
	MatrixXd S = svd.singularValues();

	//Psuedoinvert the diagonal matrix of singular values
	MatrixXd S_plus = MatrixXd::Zero(n_bases, n_bases);
	for(int i=0; i<n_bases; i++){
		if(S(i) > precisionCutoff){  //Cutoff to avoid huge inverted values for numerical stability
			S_plus(i,i) = 1.0/S(i);
		}
	}

	//Compute psuedoinverse of orginal matrix
	return V * S_plus * U.transpose();
}

}


