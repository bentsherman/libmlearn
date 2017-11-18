/**
 * @file clustering/parameterset.cpp
 *
 * Implementation of the parameter set.
 */
#include <cmath>
#include "mlearn/clustering/parameterset.h"
#include "mlearn/math/matrix_utils.h"

namespace ML {

/**
 * Construct a parameter set.
 *
 * @param k
 */
ParameterSet::ParameterSet(int k)
{
	this->_k = k;
}

/**
 * Move-construct a parameter set.
 *
 * @param theta
 */
ParameterSet::ParameterSet(ParameterSet&& theta)
	: ParameterSet()
{
	swap(*this, theta);
}

/**
 * Print a parameter set.
 */
void ParameterSet::print() const
{
	std::cout << "k = " << this->_k << "\n";

	for ( int i = 0; i < this->_k; i++ ) {
		std::cout << "p_" << i + 1 << " = " << this->_p[i] << "\n";

		std::cout << "mu_" << i + 1 << " =\n";
		this->_mu[i].print(std::cout);

		std::cout << "S_" << i + 1 << " =\n";
		this->_S[i].print(std::cout);
	}

	std::cout << "\n";
}

/**
 * Initialize a parameter set by selecting k means randomly.
 *
 * @param X
 */
void ParameterSet::initialize(const std::vector<Matrix>& X)
{
	int n = X.size();
	int d = X[0].rows();

	// choose k means randomly from data
	this->_mu = m_random_sample(X, this->_k);

	// initialize mixture proportions, covariances
	this->_p.reserve(this->_k);
	this->_S.reserve(this->_k);

	for ( int j = 0; j < this->_k; j++ ) {
		// initialize p_j = 1 / k
		this->_p.push_back(1.0f / this->_k);

		// initialize S_j = I_d
		this->_S.push_back(Matrix::identity(d));
	}

	// initialize n_j for each j
	this->_n.resize(this->_k, n / this->_k);

	// initialize mean-subtracted data array
	this->_Xsubs.reserve(this->_k);

	for ( int j = 0; j < this->_k; j++ ) {
		this->_Xsubs.push_back(m_subtract_mean(X, this->_mu[j]));
	}

	// initialize pdf matrix
	this->_h = Matrix(n, this->_k);
	this->pdf_all();
}

/**
 * Compute the mean-subtracted data of X for each mu.
 *
 * @param X
 */
void ParameterSet::subtract_means(const std::vector<Matrix>& X)
{
	for ( int j = 0; j < this->_k; j++ ) {
		for ( int i = 0; i < X.size(); i++ ) {
			Matrix& x_i = this->_Xsubs[j][i];
			x_i.assign_column(0, X[i], 0);
			x_i -= this->_mu[j];
		}
	}
}

/**
 * Compute h_ij for all i, j:
 *
 *   h_ij = h(x_i | mu_j, S_j)
 */
void ParameterSet::pdf_all()
{
	int n = _Xsubs[0].size();
	int d = _Xsubs[0][0].rows();

	float temp1 = powf(2 * M_PI, d / 2.0f);

	for ( int j = 0; j < this->_k; j++ ) {
		float temp2 = powf(this->_S[j].determinant(), -0.5f);
		Matrix S_inv = this->_S[j].inverse();

		for ( int i = 0; i < n; i++ ) {
			this->_h.elem(i, j) = temp1 * temp2 * expf(-0.5f * (_Xsubs[j][i].T() * S_inv).dot(_Xsubs[j][i]));
		}
	}
}

/**
 * Compute the log-likelihood of a parameter set:
 *
 *   L = sum(log(sum(p_j * h_ij, j=1:k)), i=1:n)
 */
float ParameterSet::log_likelihood() const
{
	int n = this->_h.rows();
	float L = 0;

	for ( int i = 0; i < n; i++ ) {
		float sum = 0;

		for ( int j = 0; j < this->_k; j++ ) {
			sum += this->_p[j] * this->_h.elem(i, j);
		}

		L += logf(sum);
	}

	return L;
}

/**
 * Swap function for ParameterSet.
 *
 * @param lhs
 * @param rhs
 */
void swap(ParameterSet& lhs, ParameterSet& rhs)
{
	std::swap(lhs._k, rhs._k);
	std::swap(lhs._n, rhs._n);
	std::swap(lhs._p, rhs._p);
	std::swap(lhs._mu, rhs._mu);
	std::swap(lhs._S, rhs._S);
	std::swap(lhs._Xsubs, rhs._Xsubs);
	std::swap(lhs._h, rhs._h);
}

}