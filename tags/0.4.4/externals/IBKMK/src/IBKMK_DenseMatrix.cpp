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

#include "IBKMK_DenseMatrix.h"

#include <IBK_StringUtils.h>
#include <IBK_Exception.h>

#include "IBKMKC_dense_matrix.h"

namespace IBKMK {

void DenseMatrix::resize(unsigned int n) {
	m_n = n;
	m_data.resize(m_n*m_n);
	if (m_pivots.size() != n)
		m_pivots.clear(); // safeguard against potential misuse
}

void DenseMatrix::swap(DenseMatrix & mat) {
	std::swap(m_n, mat.m_n);
	m_data.swap(mat.m_data);    // this is faster then normal swapping!
	m_pivots.swap(mat.m_pivots);
}

int DenseMatrix::lu() {
	if (m_pivots.size() != m_n) {
		m_pivots.resize(m_n);
	}
	return ibkmk_dense_LU_pivot(m_n, &m_data[0], &m_pivots[0]);
}


void DenseMatrix::backsolve(double * b) const {
	if (m_pivots.empty())
		throw IBK::Exception("Call lu() first before calling backsolve!", "[DenseMatrix::backsolve]");
	ibkmk_dense_backsolve_pivot(m_n, &m_data[0], &m_pivots[0], b);
}

void DenseMatrix::multiply(const double * b, double * res) const {
	ibkmk_dense_vec_mult(m_n, &m_data[0], b, res);
}

void DenseMatrix::write(std::ostream & out, double * b, bool matlabFormat,
						unsigned int width) const
{
	if (width < 1)	width = 10;
	else 			--width; // subtract one for automatically padded " "
	if (matlabFormat) {
	}
	else {
		// screen format
		for (unsigned int i=0; i<m_n; ++i) {
			out << "[ ";
			for (unsigned int j=0; j<m_n; ++j) { // column based
				out << std::setw(width) << std::right << operator()(i,j);
				out << " ";
			}
			out << " ]";
			if (b != NULL)
				out << "  [ " << std::setw(width) << std::right << b[i] << " ]";
			out << std::endl;
		}
	}
}


void DenseMatrix::writeEuler( std::ostream &out) const {

	bool isNotFirst;

	out << "[";
	for (unsigned int i=0; i<m_n; ++i) {

		isNotFirst = false;

		for (unsigned int j=0; j<m_n; ++j) { // column based

			double value = operator()(i,j);

			if ( isNotFirst && j < (m_n) ){
				out << ",";
			}

			if (value == 0){

				// now search for following value != 0 in this line
				for (unsigned int k = j+1; k < m_n; ++k){
					if ( operator()(i,k) != 0 ){
						out << std::right << value;
						isNotFirst = true;
						break;
					}
				}

			} else {
				out << std::right << value;
				isNotFirst = true;
			}
		}

		if ( i < m_n-1 )
			out << ";";

	}
	out << "]";

}

void DenseMatrix::readEuler( std::string &input  ){

	// split by ;
	std::vector< std::string > vectorOfStrings;
	IBK::explode( input, vectorOfStrings, ';' );

	// get size of matrix and resize data structures
	unsigned int sizeMatrix = vectorOfStrings.size();
	m_data.resize( (sizeMatrix*sizeMatrix), 0 );

	// remove leading '['
	size_t pos = vectorOfStrings[0].find('[');
	if (pos != std::string::npos){
		vectorOfStrings[0] = vectorOfStrings[0].substr( pos+1 );
	}

	// last ]
	pos = vectorOfStrings[(sizeMatrix-1)].rfind(']');
	if (pos != std::string::npos){
		vectorOfStrings[(sizeMatrix-1)] = vectorOfStrings[(sizeMatrix-1)].substr( 0, pos );
	}


	unsigned int counterCols = 0, counterRowValue = 0;
	for (
			std::vector< std::string >::const_iterator it = vectorOfStrings.begin(),
			end = vectorOfStrings.end();
			it != end;
			++it,
			++counterCols
		){

		// now split each string and fill values
		std::vector< std::string > vectorRowValueStrings;
		IBK::explode( *it, vectorRowValueStrings, ',' );
		counterRowValue = 0;

		for (
				std::vector< std::string >::const_iterator itRow = vectorRowValueStrings.begin(),
				end = vectorRowValueStrings.end();
				itRow != end;
				++itRow,
				++counterRowValue
			)
		{

			m_data[ counterRowValue * sizeMatrix + counterCols ] = IBK::string2val< double >(*itRow);

		} // for all ,

	} // for all ;

}


/*

double DenseMatrix::max_row_norm() {
	double res = 0.0;
	for (unsigned int i=0; i<m_n; ++i) {
		double tmp = 0.0;
		for (unsigned int j=0; j<m_n; ++j)
			tmp += std::fabs(m_data[i][j]);
		res = std::max(res,tmp);
	}
	return res;
}


double DenseMatrix::max_column_norm() {
	double res = 0.0;
	for (unsigned int i=0; i<m_n; ++i) {
		double tmp = 0.0;
		for (unsigned int j=0; j<m_n; ++j)
			tmp += std::fabs(m_data[j][i]);
		res = std::max(res,tmp);
	}
	return res;
}


double DenseMatrix::condi() {
	DenseMatrix inv(*this);
	if( inv.invert())
	{
		double t1 = max_row_norm();
		double t2 = inv.max_row_norm();
		return t1 * t2;
	}
	return 0.0;
}


double DenseMatrix::cond1() {
	DenseMatrix inv(*this);
	if( inv.invert())
	{
		double t1 = max_column_norm();
		double t2 = inv.max_column_norm();
		return t1 * t2;
	}
	return 0.0;
}


void DenseMatrix::getResiduals(const std::vector& rhs, const std::vector& res, std::vector& residuals)
{
	size_t size(rhs.size());
	residuals.resize(size);
	std::vector<long double> residuals_int(size);
	for (unsigned int i=0; i<size; ++i) {
		residuals_int[i] = static_cast<long double>(rhs[i]);
		long double result(0.0);
		for (unsigned int j=0; j<rows(); ++j)
		{
			result += m_data[i][j] * res[j];
		}
		residuals_int[i] = result - residuals_int[i];
	}
	std::copy(residuals_int.begin(), residuals_int.end(), residuals.begin());
}
// ---------------------------------------------------------------------------


bool DenseMatrix::nachiteration(const std::vector& rhs, std::vector& res, const double eps)
{
	std::vector residuals, residuals_res;
	size_t it(0);
	std::vector res_neu(res), zk(res);
	size_t size(res.size());
	double eps_loc(0);
	const size_t MAXITER(10000);
	getResiduals( rhs, res_neu, residuals);
	std::vector eps_1(size);
	for( size_t i=0; i<size; ++i)
	{
		eps_loc = std::max(eps_loc,std::fabs(residuals[i]));
		eps_1[i] = residuals[i];
	}
	DenseMatrix coeff(*this);
	DenseMatrix workspace(*this);
	std::vector<bool> add(size, true);
	while(eps_loc > eps && it < MAXITER)
	{
		++it;
		unsigned int result = coeff.solve_invert(residuals, residuals_res, eps, workspace);
		coeff = *this;
		if( result != 0 )
			return false;
		for( size_t i=0; i<size; ++i)
		{
			if( add[i])
				res_neu[i] += residuals_res[i];
			else
				res_neu[i] -= residuals_res[i];
			if( std::fabs(residuals[i]) > 1e20 )
				return false;
		}
		getResiduals( rhs, res_neu, residuals);
		eps_loc = 0;
		for( size_t i=0; i<size; ++i)
		{
			if( eps_1[i] > residuals[i] ) add[i] = !add[i];
			eps_1[i] = residuals[i];
			eps_loc = std::max(eps_loc,std::fabs(residuals[i]));
		}
	}
	if( it != MAXITER)
	{
		res = res_neu;
		return true;
	}
	return false;
}
// ---------------------------------------------------------------------------
*/

//unsigned int DenseMatrix::solve_invert(const std::vector& d, std::vector& x)
//{
///*	if (workspace_matrix.rows() != m_n || workspace_matrix.cols() != m_n)
//		workspace_matrix.resize(m_n, m_n);
//	workspace_matrix = *this;
//	if( !workspace_matrix.invert() )
//		return 1;
//	// calculate residuals x = A^-1 * d
//*/
//	unsigned int res = invert();
//	if (res != 0)
//		return res;
//	x = operator*(d);
///*	if( !checkResiduals(d, x, eps))
//	{
////		if( nachiteration(rhs, res, eps))
////			return 0;
////		DenseMatrix coeff_rscale(*this);
////		std::vector rhs_rscale(rhs);
////		coeff_rscale.row_scaling(rhs_rscale);
////		if( !coeff_rscale.invert())
////			return 1;
////		res = coeff_rscale * rhs_rscale;
////		if( !checkResiduals(rhs, res, eps))
////			return 2;
//		return 2;
//	}
//*/
//	return 0;
//}

///*
//unsigned int DenseMatrix::solve_gauss_pivot2(const std::vector<double>& rhs, std::vector<double>& res, const double eps) {
//	std::vector workspace(2*(rhs.size()+1));
//	DenseMatrix coeff_copy(*this);
////	coeff_copy.fill(*this);
////	std::vector<long double> res_neu(rhs.size());
////	std::copy(rhs.begin(), rhs.end(), res_neu.begin());
//	res = rhs;
//	unsigned int result = coeff_copy.solve_gauss_pivot(res, workspace);
//	if( result == 0 )
//	{
////        std::copy(res_neu.begin(), res_neu.end(), res.begin());
////		if( check_residuals(rhs, res, eps))
//			return 0;
//		if( nachiteration(rhs, res, eps))
//			return 0;
//		return 2;
//	}
//	return 1;
//}
//*/



//unsigned int DenseMatrix::solve_gauss_pivot(double * d, double * workspace) {
//	double * current_row;
//	double pivot, tmp, sum, mult, pivot_1;
//	if (m_n != m_n) return 1;  // invalid dimensions
//	// 1. elimination into upper triagonal matrix (n = m_n = m_n)
//	for (unsigned int i=0; i<m_n-1; ++i) {
//		// find Pivot element
//		unsigned int pivot_index = i;
//		pivot = std::fabs(m_data[i][i]); // pivot holds absolute value of pivot
//		for (unsigned int p=i+1; p<m_n; ++p) {
//			tmp = std::fabs(m_data[p][i]);
//			if (pivot < tmp) {
//				pivot = tmp;
//				pivot_index = p;
//			}
//		}
//		// if Pivot element is zero, bail out
//		if (pivot==0)   {
//			return 2;  // matrix singular
//		}
//		// if Pivot index is different from current row, swap rows
//		if (pivot_index != i) {
//			unsigned int bytes = static_cast<unsigned int>((m_n-i)*sizeof(T));
//			double * src = &m_data[pivot_index][i];
//			pivot = *src;
//			memcpy(workspace, src, bytes);
//			memcpy(src, &m_data[i][i], bytes);
//			memcpy(&m_data[i][i], workspace, bytes);
//			std::swap(d[pivot_index], d[i]);
//		}
//		// now modify all rows below the i-th row
//		pivot_1 = 1.0/pivot;
//		for (unsigned int j=i+1; j<m_n; ++j) {
//			current_row = &m_data[j][0];
//			mult = current_row[i]*pivot_1;  // must be smaller then 1 now
//			current_row[i]=0;
//			if (mult==0) continue;        // row remains unchanged
//			for (unsigned int k=i+1; k<m_n; ++k)
//				current_row[k] -= m_data[i][k]*mult;
//			d[j] -= d[i]*mult;
//		}
//	}
//	// 2. backward substitution
//	for (int i=static_cast<int>(m_n)-1; i>=0; --i) {
//		current_row = &m_data[i][0];
//		if (!current_row[i])  return 2;  // matrix singular
//		sum = 0;
//		for (unsigned int j=i+1; j<m_n; ++j)
//			sum += d[j]*current_row[j];
//		d[i] = (d[i] - sum)/current_row[i];
//	}
//	return 0;
//}
//// ---------------------------------------------------------------------------


//int DenseMatrix::solve_sor(const double * d, double * x, double * workspace,
//	double w, double eps, double limit, int & max_iters,
//	const int * constraints, const double * lower_lim, const T* upper_lim) const
//{
//	unsigned int n = m_n;
//	// ensure that there are no zeros in the diagonal
//	for (unsigned int i=0; i<n; ++i)
//		if (m_data[i][i] == 0) return 2;
//	while (--max_iters) {
///*
//		cout << max_iters << "   ";
//		for (unsigned int i=0; i<n; ++i)
//			cout << x[i] << " ";
//		cout << endl;
//*/
//		// calculate maximum residual of all equations
//		double maxres = max_residual(d, x);
//		if (maxres > eps) {
//			if (maxres > limit)
//				return 3;   //limit exceeded
//		}
//		else {
//			break; // converged
//		}
//		// store old guesses for convergence criterion
//		std::memcpy(workspace, x, n*sizeof(double));
//		// solve equations for the diagonal element using last calculated guesses
//		for (unsigned int i=0; i<n; ++i) {
//			double sum = 0;
//			unsigned int j;
//			for (j=0; j<i; ++j)
//				sum += m_data[i][j]*x[j];
//			++j; // skip element i
//			for (; j<n; ++j)
//				sum += m_data[i][j]*x[j];
//			x[i]=w*( (d[i] - sum) / m_data[i][i] ) + (1-w)*x[i];
//			if (constraints != NULL) {
//				switch (constraints[i]) {
//					case 0 : ; // no constraints
//					default: ;
//						break;

//					case 1 : // must not go below lower limit
//						IBK_ASSERT(lower_lim != NULL);
//						if (x[i] < lower_lim[i]) {
//							x[i] = workspace[i] - 0.1*(workspace[i] - lower_lim[i]);
//						}
//						break;

//					// TODO : other constraints
//				}
//			}
//		}
//	}
//	if (!max_iters)		return 4;
//	else				return 0;
//}
//// ---------------------------------------------------------------------------


//void DenseMatrix::scale_rows() {
//	for (unsigned int i=0; i<m_n; ++i) {
//		// calculated maximum coefficient in this row
//		double tmp = 0.0;
//		for (unsigned int j=0; j<m_n; ++j)
//			tmp += std::fabs(m_data[i][j]);
//		// calculate inverse
//		tmp = 1.0 / tmp;
//		// scale all coefficients in this row with this value
//		for (unsigned int j=0; j<m_n; ++j)
//			m_data[i][j] *= tmp;
//	}
//}
//// ---------------------------------------------------------------------------


//void DenseMatrix::col_scaling(std::vector& d)
//{
//	d.resize(m_n);
//	for( size_t i=0; i<m_n; ++i)
//	{
//		double tmp(0.0);
//		for( size_t j=0; j<m_n; ++j)
//			tmp += std::fabs(m_data[j][i]);
//		tmp = 1.0 / tmp;
//		d[i] = tmp;
//		for( size_t j=0; j<m_n; ++j)
//			m_data[j][i] *= tmp;
//	}
//}
//// ---------------------------------------------------------------------------


//bool DenseMatrix::row_scaling(std::vector& d)
//{
//	if( d.size() != m_n) return false;
//	for( size_t i=0; i<m_n; ++i)
//	{
//		double tmp(0.0);
//		for( size_t j=0; j<m_n; ++j)
//			tmp += std::fabs(m_data[i][j]);
//		tmp = 1.0 / tmp;
//		for( size_t j=0; j<m_n; ++j)
//			m_data[i][j] *= tmp;
//		d[i] *= tmp;
//	}
//	return true;
//}
//// ---------------------------------------------------------------------------


//unsigned int DenseMatrix::invert() {
//	IBK_ASSERT(m_n == m_n );
//	unsigned int N = m_n; // readibility improvement
//	if (N <= 1) return 0;	// no need to calculate inverse matrix
//	if (m_data[0][0] == 0)
//		return 1; // matrix singular in first row (row 0)
//	for (unsigned int i=1; i<N; ++i) // normalize row 0
//		m_data[0][i] /= m_data[0][0];
//	for (unsigned int i=1; i<N; ++i) {
//		for (unsigned int j=i; j<N; ++j) {   // do a column of L
//			double sum = 0.0;
//			for (unsigned int k=0; k<i; ++k)
//				sum += m_data[j][k] * m_data[k][i];
//			m_data[j][i] -= sum;
//		}
//		if (i == N-1) continue;
//		for (unsigned int j=i+1; j<N; ++j) { // do a row of U
//			double sum = 0.0;
//			for (unsigned int k = 0; k<i; ++k)
//				sum += m_data[i][k]*m_data[k][j];
//			if (m_data[i][i] == 0)
//				return i+1; // matrix singular in row i
//			m_data[i][j] = (m_data[i][j]-sum) / m_data[i][i];
//		}
//	}
//	for (unsigned int i=0; i<N; ++i) {  // invert L
//		for (unsigned int j=i; j<N; ++j) {
//			double x = 1.0;
//			if (i != j) {
//				x = 0.0;
//				for (unsigned int k=i; k<j; ++k)
//					x -= m_data[j][k]*m_data[k][i];
//			}
//			m_data[j][i] = x / m_data[j][j];
//		}
//	}
//	for (unsigned int i=0; i<N; ++i) {  // invert U
//		for (unsigned int j=i; j<N; ++j) {
//			if (i == j) continue;
//			double sum = 0.0;
//			for (unsigned int k=i; k<j; ++k)
//				sum += m_data[k][j]*( (i==k) ? 1.0 : m_data[i][k] );
//			m_data[i][j] = -sum;
//		}
//	}
//	for (unsigned int i=0; i<N; ++i) {   // final inversion
//		for (unsigned int j=0; j<N; ++j) {
//			double sum = 0.0;
//			for (unsigned int k = ( (i>j) ? i : j ); k<N; ++k)
//				sum += ( (j==k) ? 1.0 : m_data[j][k] ) * m_data[k][i];
//			m_data[j][i] = sum;
//		}
//	}
//	return true;
//}


//double DenseMatrix::max_residual(const double * d, const double * x) const {
//	// test residuals
//	double max_res = 0.0;
//	for (unsigned int i=0; i<m_n; ++i)	{
//		double result = 0.0;
//		for (unsigned int j=0; j<m_n; ++j)
//			result += m_data[i][j] * x[j];
//		max_res = std::max(max_res, std::fabs(result - d[i]) );	// remember maximum of normalized residual
//	}
//	return max_res;
//}


} // namespace IBKMK

