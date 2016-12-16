/*	IBK Math Kernel Library
	Copyright (c) 2001-2016, Institut fuer Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, A. Paepcke, H. Fechner, St. Vogelsang
	All rights reserved.

	This file is part of the IBKMK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation 
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	This library contains derivative work based on other open-source libraries,
	see LICENSE and OTHER_LICENSES files.

*/

#ifndef IBKMK_DenseMatrixH
#define IBKMK_DenseMatrixH

#include <vector>
#include <stdexcept>
#include <cmath>
#include <cstring>

#include <IBK_configuration.h>
#include <IBK_assert.h>
#include <IBK_FormatString.h>

namespace IBKMK {

/*! A simple matrix implementation.

	This is an implementation for a square dense matrix with corresponding math algorithms.
	Resizing is possible, see resize() for details.

	Data storage is column-based, the element (2,5) has the index [2*n + 5] in the linear
	memory array.
*/
class DenseMatrix {
public:
	/*! Default construction.
		Creates an empty matrix.
	*/
	DenseMatrix() : m_n(0) {}
	/*! Initialisation constructor.
		Creates a matrix with the dimensions: n x n
	*/
	DenseMatrix(unsigned int n) : m_data(n*n), m_n(n) {}

	/*! Initialization constructor with initial value.
		Creates a matrix with the dimensions: n x n and sets all elements to 'value'.
	*/
	DenseMatrix(unsigned int n, double value) : m_data(n*n, value), m_n(n) {}


	/*! Returns a reference to the matrix element at [row][col].
	 * Column major format.
	*/
	double & operator() (unsigned int row, unsigned int col) {
		IBK_ASSERT(row < m_n);
		IBK_ASSERT(col < m_n);
		return m_data[col + row*m_n];
	}

	/*! Returns a constant reference to the matrix element at [row][col].
	 * Column major format.
	*/
	double operator() (unsigned int row, unsigned int col) const {
		IBK_ASSERT(row < m_n);
		IBK_ASSERT(col < m_n);
		return m_data[col + row*m_n];
	}

	/*! Returns the number of rows/columns in the matrix. */
	unsigned int n() const { return m_n; }

	/*! Resizes the matrix (all data is lost if size differs from previous size). */
	void resize(unsigned int n);

	/*! Swaps the content of the matrix with the content of another matrix.
		This is efficient because only internal pointers are exchanged.
	*/
	void swap(DenseMatrix & mat);

	/*! Empties the matrix and sets its dimensions to 0. */
	void clear() { m_data.clear(); m_n = 0; }

	/*! Fills the matrix with the given value. */
	void fill(double value) { IBK_ASSERT(m_n != 0); std::fill(m_data.begin(), m_data.end(), value); }

	/*! Sets the matrix values to zero. */
	void setZero() { IBK_ASSERT(m_n != 0); std::memset(&m_data[0], 0, sizeof(double)*m_n*m_n); }

	/*! Returns true is matrix is empty. */
	bool isEmpty() const { return m_n == 0; }

	/*! Performs an LU factorization of the matrix which is the first step in solving an equation system Ax = b.
		The lower and upper triangular matrices are stored in place of the original matrix.
		Uses built-in pivot vector.
		\sa backsolve()
		\return Returns error code != 0 if LU decomposition fails. Error code is row of matrix + 1 that
				became singular during decomposition.
	*/
	int lu();

	/*! Solves the linear system of equations Ax = b using back-solution when
		A is already stored as LU factorized matrix.
		Uses built-in pivot vector.
		\sa lu().
	*/
	void backsolve(double * b) const;

	/*! Multiplication operation for this simple matrix with a vector.
		\param b The vector (size n).
		\param rhs The resulting vector (size n).
	*/
	void multiply(const double * b, double * res) const;

	/*! Writes the matrix and optionally a right-hand-side vector to output stream or file. */
	void write(std::ostream & out, double * b = NULL, bool matlabFormat = false,
			   unsigned int width = 4) const;

	/*! writes matrix to string in euler format. */
	void writeEuler( std::ostream &output) const;

	/*! read matrix from stream in euler format. */
	void readEuler( std::string &input );


	/*! Returns the row sum norm of the matrix. */
	double rowSumNorm();
	/*! Returns the column sum norm of the matrix. */
	double columnSumNorm();
	/*! Returns the condition based of the row sum norm of the matrix. */
	double conditionRowBased();
	/*! Returns the condition based of the column sum norm of the matrix. */
	double conditionColBased();

	/*! Moved from simple matrix. \todo check if fits to Norms above. */
//
//	/*! Returns the row sum norm of the matrix. */
//	T max_row_norm();
//	/*! Returns the column sum norm of the matrix. */
//	T max_column_norm();
//	/*! Returns the condition based of the row sum norm of the matrix. */
//	T condi();
//	/*! Returns the condition based of the column sum norm of the matrix. */
//	T cond1();

//	/*! Multiplication operation for this simple matrix with a vector.
//		\param a The vector (size n).
//		\return Returns a std::vector (size n). Throws an IBK_exception if sizes of matrix and
//				vector are different.
//	*/
//	std::vector<T> operator*(const std::vector<T>& a) const;

//	/*! Multiplication operation for a simple matrix and a vector.
//		\param M The coefficient matrix (size n x n).
//		\param a The vector (size n).
//		\return Returns a std::vector (size n). Throws an IBK_exception if sizes of matrix and
//				vector are different.
//	*/
//	template <typename T>
//	inline std::vector<T> operator*(const matrix<T>& M, const std::vector<T>& a) {
//		return M.operator*(a); // reuse member function
//	}

//	template <typename T>
//	typename std::vector<T> matrix<T>::operator*(const std::vector<T>& a) const {
//		if( a.size() != cols_ )
//			return std::vector<T>();
//		std::vector<T> res(cols_, 0.0);
//		for (unsigned int j=0; j<rows_; ++j)
//			for (unsigned int i=0; i<cols_; ++i)
//				res[j] += data_[j][i]*a[i];
//		return res;
//	}



//	template <typename T>
//	T matrix<T>::max_row_norm() {
//		T res = 0.0;
//		for (unsigned int i=0; i<rows_; ++i) {
//			T tmp = 0.0;
//			for (unsigned int j=0; j<cols_; ++j)
//				tmp += std::fabs(data_[i][j]);
//			res = std::max(res,tmp);
//		}
//		return res;
//	}

//	template <typename T>
//	T matrix<T>::max_column_norm() {
//		T res = 0.0;
//		for (unsigned int i=0; i<cols_; ++i) {
//			T tmp = 0.0;
//			for (unsigned int j=0; j<rows_; ++j)
//				tmp += std::fabs(data_[j][i]);
//			res = std::max(res,tmp);
//		}
//		return res;
//	}

//	template <typename T>
//	T matrix<T>::condi() {
//		matrix<T> inv(*this);
//		if( inv.invert())
//		{
//			T t1 = max_row_norm();
//			T t2 = inv.max_row_norm();
//			return t1 * t2;
//		}
//		return 0.0;
//	}

//	template <typename T>
//	T matrix<T>::cond1() {
//		matrix<T> inv(*this);
//		if( inv.invert())
//		{
//			T t1 = max_column_norm();
//			T t2 = inv.max_column_norm();
//			return t1 * t2;
//		}
//		return 0.0;
//	}

//	/*! Invertes the matrix in-place (via LU factorization).
//		\return Returns an error code as follows.
//		\todo move to IBKMK::DenseMatrix
//	*/
//	unsigned int invert();

//	/*! Devide all rows of the matrix by the sum of the absolute value of all coefficients in the row.
//		\todo move to IBKMK::DenseMatrix
//	*/
//	void scale_rows();
//	/// Makes a simple row scaling for a linear system of equations
//	/// @param d reference to a vector with the right-hand-side of the equation system (at least rows() elements).
//	/// @return false if size of d not equal to rows()
//	///	\todo move to IBKMK::DenseMatrix
//	bool row_scaling(std::vector<T>& d);
//	/// Makes a simple column scaling
//	/// @param d returns the scaling vector.
//	///	\todo move to IBKMK::DenseMatrix
//	void col_scaling(std::vector<T>& d);

//	/*! Solves the linear system of equations A*x = d by calculating the inverse first (with the LU algorithm, without pivoting).
//		\param d Const reference to a vector with the right-hand-side of the equation system (at least rows() elements).
//		\param x Reference to a vector with the result.
//		\return The return codes are as follows: <ul>
//		<li>Return value = 0 : All ok</li>
//		<li>Return value = 1 : Error - Invalid dimensions</li>
//		<li>Return value = 2 : Error - Matrix is singular</li>
//		</ul>
//		\todo move to IBKMK::DenseMatrix
//	*/
//	unsigned int solve_invert(const std::vector<T>& d, std::vector<T>& x);

//	/*! Solves the linear system of equations A*x = d with the GAUSS algorithm (with partial pivoting) and
//		stores the resultant x vector in the place of vector d. The matrix is hereby modified.
//		The parameter workspace must point to a memory block of size n.
//		\return The return codes are as follows: <ul>
//		<li>Return value = 0 : All ok</li>
//		<li>Return value = 1 : Error - Invalid dimensions</li>
//		<li>Return value = 2 : Error - Matrix is singular</li>
//		</ul>
//		\todo move to IBKMK::DenseMatrix
//	*/
//	unsigned int solve_gauss_pivot(T * d, T * workspace);
//	/*! Convenience function, essentially behaves like the function above. Make sure the vector has
//		the right size!
//		\todo move to IBKMK::DenseMatrix
//	*/
//	unsigned int solve_gauss_pivot(std::vector<T>& d, std::vector<T>& workspace) {
//		return solve_gauss_pivot(&d[0], &workspace[0]);
//	}

//	/*! Solves a linear equation system with the successive over-relaxation.
//		Equations system given as : A*x = d, whereas A is this matrix with size n x n.
//		The results are stored in the first n elements of the workspace vector.
//		Optional output values are the max_iters parameter, which holds the number of remaining iterations.
//		During the iteration max_iters is counted down until it reaches zero, upon which the function
//		returns with error code 4.
//		\param d			Pointer to right-hand-side vector, size n.
//		\param x			Pointer to solution vector, size n.
//		\param workspace	Pointer to free space for calculations, at least of size n.
//		\param w			Relaxation parameter, 0 < w < 2.
//		\param eps			The maximum difference between x_n and x_n+1 to assume convergence.
//		\param limit		The minimum difference between x_n and x_n+1 to assume divergence.
//		\param max_iters	Maximum number of iterations, contains after return the remaining iterations.
//		\param contraints	Optional pointer to vector integers flags encoding solution constraints.
//		\param lower_lim	Optional pointer to vector with lower limits for solution variables.
//		\param upper_lim	Optional pointer to vector with upper limits for solution variables.
//		\return The return codes are as follows: <ul>
//		<li>Return value = 0 : All ok</li>
//		<li>Return value = 1 : Error - Invalid dimensions</li>
//		<li>Return value = 2 : Error - Zero element in main diagonal</li>
//		<li>Return value = 3 : Error - Diverging solution</li>
//		<li>Return value = 4 : Error - Convergence iteration limit exceeded</li>
//		</ul>
//		\todo move to IBKMK::DenseMatrix
//	*/
//	int solve_sor(const T * d, T * x, T * workspace, double w, double eps, double limit, int & max_iters,
//		const int * constraints=NULL, const T * lower_lim=NULL, const T* upper_lim=NULL) const;
//	/*! Convenience function, behaves essentially like the function above.
//		\todo move to IBKMK::DenseMatrix
//	*/
//	int solve_sor(const std::vector<T> & d, std::vector<T> & x, std::vector<T> & workspace, double w, double eps, double limit, int & max_iters,
//		const int * constraints=NULL, const T * lower_lim=NULL, const T * upper_lim=NULL) const
//	{
//		return solve_sor(&d[0], &x[0], &workspace[0], w, eps, limit, max_iters, constraints, lower_lim, upper_lim);
//	}

//	/*! Calculates the normalized maximum residual of the solution to the linear equation solution A*x = d.
//		The residuals are calculated for each line and the absolute maximum of all residuals is returned normalized by the
//		left hand side of the equation.
//		\param d Const reference to a vector with the right-hand-side of the equation system (at least rows() elements).
//		\param x Reference to a vector with the result.
//		\return Returns absolute value of maximum normalized residual.
//		\todo move to IBKMK::DenseMatrix
//	*/
//	T max_residual(const T * d, const T * x) const;
//	/*! Convenience function, works essentially like the function above.
//		\todo move to IBKMK::DenseMatrix
//	*/
//	T max_residual(const std::vector<T>& d, const std::vector<T>& x) const {
//		IBK_ASSERT(x.size() == d.size());
//		IBK_ASSERT(rows_ == d.size());
//		return max_residual(&d[0], &x[0]);
//	}

//	/*! Checks the accuracy of a linear equation solution A*x = d. The check is done line by line and
//		each line must fulfil the requirement.
//		\param d Const reference to a vector with the right-hand-side of the equation system (at least rows() elements).
//		\param x Reference to a vector with the result.
//		\param eps Epsilon value for residue testing.
//		\return Returns true, if residuals were all below eps.
//		\todo move to IBKMK::DenseMatrix
//	*/
//	bool check_residuals(const std::vector<T>& d, const std::vector<T>& x, const T & eps) const {
//		return max_residual(d, x) < eps;
//	}


//private:
//	/*! \todo move to IBKMK::DenseMatrix */
//	void getResiduals(const std::vector<T>& rhs, const std::vector<T>& res, std::vector<T>& residuals);
//	/*! \todo move to IBKMK::DenseMatrix */
//	bool nachiteration(const std::vector<T>& rhs, std::vector<T>& res, const double eps);


//	template <typename T>
//	void matrix<T>::getResiduals(const std::vector<T>& rhs, const std::vector<T>& res, std::vector<T>& residuals)
//	{
//		size_t size(rhs.size());
//		residuals.resize(size);
//		std::vector<long double> residuals_int(size);
//		for (unsigned int i=0; i<size; ++i) {
//			residuals_int[i] = static_cast<long double>(rhs[i]);
//			long double result(0.0);
//			for (unsigned int j=0; j<rows(); ++j)
//			{
//				result += data_[i][j] * res[j];
//			}
//			residuals_int[i] = result - residuals_int[i];
//		}
//		std::copy(residuals_int.begin(), residuals_int.end(), residuals.begin());
//	}
//	// ---------------------------------------------------------------------------

//	template <typename T>
//	bool matrix<T>::nachiteration(const std::vector<T>& rhs, std::vector<T>& res, const double eps)
//	{
//		std::vector<T> residuals, residuals_res;
//		size_t it(0);
//		std::vector<T> res_neu(res), zk(res);
//		size_t size(res.size());
//		double eps_loc(0);
//		const size_t MAXITER(10000);
//		getResiduals( rhs, res_neu, residuals);
//		std::vector<T> eps_1(size);
//		for( size_t i=0; i<size; ++i)
//		{
//			eps_loc = std::max(eps_loc,std::fabs(residuals[i]));
//			eps_1[i] = residuals[i];
//		}
//		matrix<T> coeff(*this);
//		matrix<T> workspace(*this);
//		std::vector<bool> add(size, true);
//		while(eps_loc > eps && it < MAXITER)
//		{
//			++it;
//			unsigned int result = coeff.solve_invert(residuals, residuals_res, eps, workspace);
//			coeff = *this;
//			if( result != 0 )
//				return false;
//			for( size_t i=0; i<size; ++i)
//			{
//				if( add[i])
//					res_neu[i] += residuals_res[i];
//				else
//					res_neu[i] -= residuals_res[i];
//				if( std::fabs(residuals[i]) > 1e20 )
//					return false;
//			}
//			getResiduals( rhs, res_neu, residuals);
//			eps_loc = 0;
//			for( size_t i=0; i<size; ++i)
//			{
//				if( eps_1[i] > residuals[i] ) add[i] = !add[i];
//				eps_1[i] = residuals[i];
//				eps_loc = std::max(eps_loc,std::fabs(residuals[i]));
//			}
//		}
//		if( it != MAXITER)
//		{
//			res = res_neu;
//			return true;
//		}
//		return false;
//	}
//	// ---------------------------------------------------------------------------

//	template <typename T>
//	unsigned int matrix<T>::solve_invert(const std::vector<T>& d, std::vector<T>& x)
//	{
//	/*	if (workspace_matrix.rows() != rows_ || workspace_matrix.cols() != cols_)
//			workspace_matrix.resize(cols_, rows_);
//		workspace_matrix = *this;
//		if( !workspace_matrix.invert() )
//			return 1;
//		// calculate residuals x = A^-1 * d
//	*/
//		unsigned int res = invert();
//		if (res != 0)
//			return res;
//		x = operator*(d);
//	/*	if( !checkResiduals(d, x, eps))
//		{
//	//		if( nachiteration(rhs, res, eps))
//	//			return 0;
//	//		matrix<T> coeff_rscale(*this);
//	//		std::vector<T> rhs_rscale(rhs);
//	//		coeff_rscale.row_scaling(rhs_rscale);
//	//		if( !coeff_rscale.invert())
//	//			return 1;
//	//		res = coeff_rscale * rhs_rscale;
//	//		if( !checkResiduals(rhs, res, eps))
//	//			return 2;
//			return 2;
//		}
//	*/
//		return 0;
//	}

//	/*template <typename T>
//	unsigned int matrix<T>::solve_gauss_pivot2(const std::vector<double>& rhs, std::vector<double>& res, const double eps) {
//		std::vector<T> workspace(2*(rhs.size()+1));
//		matrix<T> coeff_copy(*this);
//	//	coeff_copy.fill(*this);
//	//	std::vector<long double> res_neu(rhs.size());
//	//	std::copy(rhs.begin(), rhs.end(), res_neu.begin());
//		res = rhs;
//		unsigned int result = coeff_copy.solve_gauss_pivot(res, workspace);
//		if( result == 0 )
//		{
//	//        std::copy(res_neu.begin(), res_neu.end(), res.begin());
//	//		if( check_residuals(rhs, res, eps))
//				return 0;
//			if( nachiteration(rhs, res, eps))
//				return 0;
//			return 2;
//		}
//		return 1;
//	}
//	*/


//	template <typename T>
//	unsigned int matrix<T>::solve_gauss_pivot(T * d, T * workspace) {
//		T * current_row;
//		T pivot, tmp, sum, mult, pivot_1;
//		if (cols_ != rows_) return 1;  // invalid dimensions
//		// 1. elimination into upper triagonal matrix (n = cols_ = rows_)
//		for (unsigned int i=0; i<cols_-1; ++i) {
//			// find Pivot element
//			unsigned int pivot_index = i;
//			pivot = std::fabs(data_[i][i]); // pivot holds absolute value of pivot
//			for (unsigned int p=i+1; p<cols_; ++p) {
//				tmp = std::fabs(data_[p][i]);
//				if (pivot < tmp) {
//					pivot = tmp;
//					pivot_index = p;
//				}
//			}
//			// if Pivot element is zero, bail out
//			if (pivot==0)   {
//				return 2;  // matrix singular
//			}
//			// if Pivot index is different from current row, swap rows
//			if (pivot_index != i) {
//				unsigned int bytes = static_cast<unsigned int>((cols_-i)*sizeof(T));
//				T * src = &data_[pivot_index][i];
//				pivot = *src;
//				memcpy(workspace, src, bytes);
//				memcpy(src, &data_[i][i], bytes);
//				memcpy(&data_[i][i], workspace, bytes);
//				std::swap(d[pivot_index], d[i]);
//			}
//			// now modify all rows below the i-th row
//			pivot_1 = 1.0/pivot;
//			for (unsigned int j=i+1; j<rows_; ++j) {
//				current_row = &data_[j][0];
//				mult = current_row[i]*pivot_1;  // must be smaller then 1 now
//				current_row[i]=0;
//				if (mult==0) continue;        // row remains unchanged
//				for (unsigned int k=i+1; k<cols_; ++k)
//					current_row[k] -= data_[i][k]*mult;
//				d[j] -= d[i]*mult;
//			}
//		}
//		// 2. backward substitution
//		for (int i=static_cast<int>(cols_)-1; i>=0; --i) {
//			current_row = &data_[i][0];
//			if (!current_row[i])  return 2;  // matrix singular
//			sum = 0;
//			for (unsigned int j=i+1; j<cols_; ++j)
//				sum += d[j]*current_row[j];
//			d[i] = (d[i] - sum)/current_row[i];
//		}
//		return 0;
//	}
//	// ---------------------------------------------------------------------------

//	template <typename T>
//	int matrix<T>::solve_sor(const T * d, T * x, T * workspace,
//		double w, double eps, double limit, int & max_iters,
//		const int * constraints, const T * lower_lim, const T* upper_lim) const
//	{
//		unsigned int n = rows_;
//		// ensure that there are no zeros in the diagonal
//		for (unsigned int i=0; i<n; ++i)
//			if (data_[i][i] == 0) return 2;
//		while (--max_iters) {
//	/*
//			cout << max_iters << "   ";
//			for (unsigned int i=0; i<n; ++i)
//				cout << x[i] << " ";
//			cout << endl;
//	*/
//			// calculate maximum residual of all equations
//			T maxres = max_residual(d, x);
//			if (maxres > eps) {
//				if (maxres > limit)
//					return 3;   //limit exceeded
//			}
//			else {
//				break; // converged
//			}
//			// store old guesses for convergence criterion
//			std::memcpy(workspace, x, n*sizeof(double));
//			// solve equations for the diagonal element using last calculated guesses
//			for (unsigned int i=0; i<n; ++i) {
//				double sum = 0;
//				unsigned int j;
//				for (j=0; j<i; ++j)
//					sum += data_[i][j]*x[j];
//				++j; // skip element i
//				for (; j<n; ++j)
//					sum += data_[i][j]*x[j];
//				x[i]=w*( (d[i] - sum) / data_[i][i] ) + (1-w)*x[i];
//				if (constraints != NULL) {
//					switch (constraints[i]) {
//						case 0 : ; // no constraints
//						default: ;
//							break;

//						case 1 : // must not go below lower limit
//							IBK_ASSERT(lower_lim != NULL);
//							if (x[i] < lower_lim[i]) {
//								x[i] = workspace[i] - 0.1*(workspace[i] - lower_lim[i]);
//							}
//							break;

//						// TODO : other constraints
//					}
//				}
//			}
//		}
//		if (!max_iters)		return 4;
//		else				return 0;
//	}
//	// ---------------------------------------------------------------------------

//	template <typename T>
//	void matrix<T>::scale_rows() {
//		for (unsigned int i=0; i<rows_; ++i) {
//			// calculated maximum coefficient in this row
//			T tmp = 0.0;
//			for (unsigned int j=0; j<cols_; ++j)
//				tmp += std::fabs(data_[i][j]);
//			// calculate inverse
//			tmp = 1.0 / tmp;
//			// scale all coefficients in this row with this value
//			for (unsigned int j=0; j<cols_; ++j)
//				data_[i][j] *= tmp;
//		}
//	}
//	// ---------------------------------------------------------------------------

//	template <typename T>
//	void matrix<T>::col_scaling(std::vector<T>& d)
//	{
//		d.resize(cols_);
//		for( size_t i=0; i<cols_; ++i)
//		{
//			T tmp(0.0);
//			for( size_t j=0; j<rows_; ++j)
//				tmp += std::fabs(data_[j][i]);
//			tmp = 1.0 / tmp;
//			d[i] = tmp;
//			for( size_t j=0; j<rows_; ++j)
//				data_[j][i] *= tmp;
//		}
//	}
//	// ---------------------------------------------------------------------------

//	template <typename T>
//	bool matrix<T>::row_scaling(std::vector<T>& d)
//	{
//		if( d.size() != rows_) return false;
//		for( size_t i=0; i<rows_; ++i)
//		{
//			T tmp(0.0);
//			for( size_t j=0; j<cols_; ++j)
//				tmp += std::fabs(data_[i][j]);
//			tmp = 1.0 / tmp;
//			for( size_t j=0; j<cols_; ++j)
//				data_[i][j] *= tmp;
//			d[i] *= tmp;
//		}
//		return true;
//	}
//	// ---------------------------------------------------------------------------

//	template <typename T>
//	unsigned int matrix<T>::invert() {
//		IBK_ASSERT(cols_ == rows_ );
//		unsigned int N = cols_; // readibility improvement
//		if (N <= 1) return 0;	// no need to calculate inverse matrix
//		if (data_[0][0] == 0)
//			return 1; // matrix singular in first row (row 0)
//		for (unsigned int i=1; i<N; ++i) // normalize row 0
//			data_[0][i] /= data_[0][0];
//		for (unsigned int i=1; i<N; ++i) {
//			for (unsigned int j=i; j<N; ++j) {   // do a column of L
//				T sum = 0.0;
//				for (unsigned int k=0; k<i; ++k)
//					sum += data_[j][k] * data_[k][i];
//				data_[j][i] -= sum;
//			}
//			if (i == N-1) continue;
//			for (unsigned int j=i+1; j<N; ++j) { // do a row of U
//				T sum = 0.0;
//				for (unsigned int k = 0; k<i; ++k)
//					sum += data_[i][k]*data_[k][j];
//				if (data_[i][i] == 0)
//					return i+1; // matrix singular in row i
//				data_[i][j] = (data_[i][j]-sum) / data_[i][i];
//			}
//		}
//		for (unsigned int i=0; i<N; ++i) {  // invert L
//			for (unsigned int j=i; j<N; ++j) {
//				T x = 1.0;
//				if (i != j) {
//					x = 0.0;
//					for (unsigned int k=i; k<j; ++k)
//						x -= data_[j][k]*data_[k][i];
//				}
//				data_[j][i] = x / data_[j][j];
//			}
//		}
//		for (unsigned int i=0; i<N; ++i) {  // invert U
//			for (unsigned int j=i; j<N; ++j) {
//				if (i == j) continue;
//				T sum = 0.0;
//				for (unsigned int k=i; k<j; ++k)
//					sum += data_[k][j]*( (i==k) ? 1.0 : data_[i][k] );
//				data_[i][j] = -sum;
//			}
//		}
//		for (unsigned int i=0; i<N; ++i) {   // final inversion
//			for (unsigned int j=0; j<N; ++j) {
//				T sum = 0.0;
//				for (unsigned int k = ( (i>j) ? i : j ); k<N; ++k)
//					sum += ( (j==k) ? 1.0 : data_[j][k] ) * data_[k][i];
//				data_[j][i] = sum;
//			}
//		}
//		return true;
//	}

//	template <typename T>
//	T matrix<T>::max_residual(const T * d, const T * x) const {
//		// test residuals
//		T max_res = 0.0;
//		for (unsigned int i=0; i<rows_; ++i)	{
//			T result = 0.0;
//			for (unsigned int j=0; j<cols_; ++j)
//				result += data_[i][j] * x[j];
//			max_res = std::max(max_res, std::fabs(result - d[i]) );	// remember maximum of normalized residual
//		}
//		return max_res;
//	}

	/*! Devide all rows of the matrix by the sum of the absolute value of all coefficients in the row. */
//	void scale_rows();
	/// Makes a simple row scaling for a linear system of equations
	/// @param d reference to a vector with the right-hand-side of the equation system (at least rows() elements).
	/// @return false if size of d not equal to rows()
//	bool row_scaling(std::vector<T>& d);
	/// Makes a simple column scaling
	/// @param d returns the scaling vector.
//	void col_scaling(std::vector<T>& d);

	/*! Calculates the normalized maximum residual of the solution to the linear equation solution A*x = d.
		The residuals are calculated for each line and the absolute maximum of all residuals is returned normalized by the
		left hand side of the equation.
		\param d Const reference to a vector with the right-hand-side of the equation system (at least rows() elements).
		\param x Reference to a vector with the result.
		\return Returns absolute value of maximum normalized residual.
	*/
	double max_residual(const double * d, const double * x) const;

	/*! Gives access to raw memory holding the matrix data. */
	std::vector<double> & data() { return m_data; }

private:
//	void getResiduals(const std::vector<T>& rhs, const std::vector<T>& res, std::vector<T>& residuals);
//	bool nachiteration(const std::vector<T>& rhs, std::vector<T>& res, const double eps);

	std::vector<double>			m_data;		///< Storage member, stores data column based using index [row + col*n]
	unsigned int				m_n;		///< Dimension of the matrix, n x n

	std::vector<int>			m_pivots;		///< Pivoting vector, used for lu() factorization, resized on first call to lu().

}; // class DenseMatrix
// ---------------------------------------------------------------------------


} // namespace IBKMK

#endif // IBKMK_DenseMatrixH
