/*	IBK Math Kernel Library
	Copyright (c) 2001-today, Institut fuer Bauklimatik, TU Dresden, Germany

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

#ifndef IBKMK_SparseMatrixH
#define IBKMK_SparseMatrixH

#include <string>
#include <iosfwd>
#include <vector>

#include <IBK_InputOutput.h>


namespace IBKMK {

/*! Abstract class for sparse matrix with different storage format. This class is used as general sparse
	matrix interface by the JacobianSparse implementation. All inherited classes own an individual
	sparse matrix data format and implement direct access to data vector, access via (row,column)- format
	as well as the matrix operations multiply, ilu, backsolve and writing/storage.
*/
class SparseMatrix {
public:
	/*! Constructor, creates an empty matrix.
		Before using the matrix, call resize().
		\sa resize
	*/
	SparseMatrix() :  m_n(0) { }

	/*! Standard destructor.
	*/
	virtual ~SparseMatrix() {}

	/*! Returns size of matrix. */
	virtual unsigned int n() const { return m_n; }

	/*! Returns pointer to continuous memory array holding data values. */
	virtual double * data() { return &m_data[0]; }

	/*! Returns pointer to continuous memory array holding data values. */
	virtual const double * data() const { return &m_data[0]; }

	/*! Returns memory size of data container. */
	virtual unsigned int dataSize() const { return (unsigned int)m_data.size(); }

	/*! Adds an identity matrix to the currently stored matrix (adds 1 to main diagonal). */
	virtual void addIdentityMatrix() = 0;

	/*! Returns a reference to the element at the coordinates [i, j] (i = row).
		\param i Matrix row, 0 <= i < m_n
		\param j Matrix column, 0 <= j < m_n
		\code
		A(i,j) = 6;
		\endcode
	*/
	virtual double & operator()(unsigned int i, unsigned int j) = 0;

	/*! Returns a constant reference to the element at the coordinates [i, j].
		\param i Matrix row, 0 <= i < n
		\param j Matrix column, 0 <= j < n
		\code
		val = A.value(i,j);
		\endcode
		When an element is requested, that is not part of the
		matrix pattern the function returns a zero instead.
	*/
	virtual double value(unsigned int i, unsigned int j) const = 0;

	/*! Returns the storage index in the m_data vector for a cell in row i, and column j.
		\param i Matrix row, 0 <= i < m_n
		\param j Matrix column, 0 <= j < m_n
		\return Returns a storage index 0 <= index <= maxIndex if the
				cell is used in the sparse matrix,
				otherwise maxIndex + 1, which indicates, that this
				cell is not part of the sparse matrix (=0 cell).

		This function is currently used by the ilu() algorithm and by the write() function.
	*/
	virtual unsigned int storageIndex(unsigned int i, unsigned int j) const = 0;

	/*! Multiplies matrix with vector b and stores result in vector res.
		This version of the function works with dublicated index vector
		entries in the index vector for zero elements in data vector.
	*/
	virtual void multiply(const double * b, double * res) const = 0;

	/*! Performs an in-place ILU.
		Requires the matrix to have the values of the original matrix.
		Uses LU factorization where <math>u_{i,i} = 1</math>.
	*/
	virtual void ilu() = 0;

	/*! Solves <math>LUx = b as x = U^{-1} L^{-1} b</math> using backward elimination.
		The matrix is expected to hold incomplete LU data as generated by ilu() with
		<math>u_{i,i} = 1</math>.
	*/
	virtual void backsolveILU(double * b) const = 0;

	/*! Dumps the matrix to an output stream in human-readibly.
		\param out Output stream (ASCII).
		\param b Pointer to vector to print alongside matrix, size = n, nullptr if no vector to print.
		\param eulerFormat If true, prints in Euler-Mathtoolbox format
		\param width Column width for matrix output (precision is expected to be set as stream property)
		\param matrixLabel The label to be used for defining the matrix in Euler format, defaults to 'A'
		\param vectorLabel The label to be used for defining the vector b in Euler format, defaults to 'b'
	*/
	void write(std::ostream & out, double * b = nullptr, bool eulerFormat = false, unsigned int width=4,
					   const char * const matrixLabel = "A", const char * const vectorLabel = "b") const
	{
		IBK::write_matrix(out, *this, b, eulerFormat, width, matrixLabel, vectorLabel);
	}

	/*! Computes and returns serialization size in bytes.
		Includes memory needed for identification of matrix type.
	*/
	virtual std::size_t serializationSize() const = 0;

	/*! Stores content at memory location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Starts with matrix identification number, followed by matrix-format-specific data.
	*/
	virtual void serialize(void* & dataPtr) const = 0;

	/*! Restores content from memory at location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Copies only actual data and pivots, requires matrix to be properly resized beforehand.
		Tests sizes stored in binary memory with sizes set in matrix (must match, otherwise exception is thrown).
	*/
	virtual void deserialize(void* & dataPtr) = 0;

	/*! Restores content from memory at location pointed to by dataPtr and increases
		pointer afterwards to point just behind the memory occupied by the copied data.
		Resizes matrix to requested dimensions and populates matrix with stored content.
	*/
	virtual void recreate(void* & dataPtr) = 0;

protected:
	unsigned int						m_n;
	std::vector<double>					m_data;
}; // SparseMatrix

/*! \file IBKMK_SparseMatrix.h
	\brief Contains the abstract class interface SparseMatrix for sparse matrix implementations.
*/

} // namespace IBKMK


#endif // IBKMK_SparseMatrixH
