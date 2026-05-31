/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

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


	This library contains derivative work based on other open-source libraries.
	See OTHER_LICENCES and source code headers for details.

*/

#ifndef IBK_cuboidH
#define IBK_cuboidH

#include <iostream>
#include <sstream>

#include "IBK_rectangle.h"
#include "IBK_Exception.h"

namespace IBK {

/*!
	A type that represents a cuboid.
	The class can be used whenever coordinates describing a cuboid have to
	be stored together, such as for 3d coordinates or when describing a
	geometrical shape. Note, that we use a right hand coordinate system.
*/
template <class T>
class cuboid {
public:
	/*! Default contructor (initialises the cuboid with default value of type T). */
	cuboid() = default;

	/*! Constructor (initialises the cuboid with given parameters for the coordinates).
		\param newLeft		Lower x-coordinate.
		\param newTop		Lower y-coordinate.
		\param newRight		Upper x-coordinate.
		\param newBottom	Upper y-coordinate.
		\param newBack		Lower z-coordinate.
		\param newFront		Upper z-coordinate.
	*/
	cuboid(T newLeft, T newTop, T newRight, T newBottom, T newBack, T newFront ) {
		set( newLeft, newTop, newRight, newBottom, newBack, newFront);
	}

	/*! Prints the cuboid range.
		\param out Stream buffer for data output.
	*/
	virtual std::ostream& output(std::ostream& out) const {
		return out << left << " " << top << " " << back << " " << right << " " << bottom << " " << front;
	}

	/*! Initializes a rectangle inside a 3D space using IBK::rectangle equivalence function.
		\param newLeft		Lower x-coordinate.
		\param newTop		Lower y-coordinate.
		\param newRight		Upper x-coordinate.
		\param newBottom	Upper y-coordinate.
	*/
	virtual void set(T newLeft, T newTop, T newRight, T newBottom) {
		set( newLeft, newTop, newRight, newBottom, T() , T());
	}

	/*! Initializes a rectangle or a cuboid.
		\param newLeft		Lower x-coordinate.
		\param newTop		Lower y-coordinate.
		\param newRight		Upper x-coordinate.
		\param newBottom	Upper y-coordinate.
		\param newBack		Lower z-coordinate: only for cuboid.
		\param newFront		Upper z-coordinate: only for cuboid.
	*/
	void set(T newLeft, T newTop, T newBottom, T newRight, T newBack, T newFront) {
		FUNCID(cuboid::set);

		if (newLeft > newRight || newTop > newBottom || newBack > newFront) {
			// decide which error message to use:
			// 2D
			if (newBack == newFront) {
				throw IBK::Exception(IBK::FormatString(
					"Invalid cuboid coordinates (%1,%3)x(%2,%4)!")
					.arg(newLeft).arg(newBottom).arg(newRight).arg(newTop), FUNC_ID);
			}
			else /* 3D*/ {
				throw IBK::Exception(IBK::FormatString(
					"Invalid cuboid coordinates (%1,%3)x(%2,%4)x(%5,%6)!")
					.arg(newLeft).arg(newBottom).arg(newRight).arg(newTop).arg(newBack)
					.arg(newFront), FUNC_ID);
			}
		}
		left=newLeft;
		top=newTop;
		right=newRight;
		bottom=newBottom;
		front=newFront;
		back=newBack;
	}

	/*! Depth of the cuboid.
		\return Difference between the coordinates 'back' and 'front'.
	*/
	T depth() const { return front - back; }

	/*! Cuboid projection to xy plane.
		\return Rectangle with coordinates on xy plane.
	*/
	rectangle<T> XY() const { return rectangle< T >( left, top, right, bottom ); }

	/*! Cuboid projection to xz plane.
		\return Rectangle with coordinates (xmin,xmax)x(ymin,ymax) = (left,right)x(front,back).
	*/
	rectangle<T> XZ() const { return rectangle< T >( left, back, right, front ); }

	/*! Cuboid projection to yz plane.
		\return Rectangle with coordinates (xmin,xmax)x(ymin,ymax) = (top,bottom)x(front,back).
	*/
	rectangle<T> YZ() const { return rectangle< T >( top, back, bottom, front ); }

	/*! Formatted output of the cuboid.
		\return String with format: [x_left, y_top, z_back] - [x_right, y_bottom, z_front].
	*/
	std::string formattedString() const {
		std::stringstream strm;
		strm << "[" << left << "," << top << "," << back  << "] - [" << right << "," << bottom  << "," << front << "]";
		return strm.str();
	}

	/*! String conversion.
		\return The range as string with all coordinates separated by a single space.
	*/
	std::string toString() const {
		std::stringstream strm;
		strm << *this;
		return strm.str();
	}

	T left;		///< The left coordinate.
	T top;		///< The top coordinate.
	T right;	///< The right coordinate.
	T bottom;	///< The bottom coordinate.
	T front;	///< The front coordinate.
	T back;		///< The back coordinate.
};

/*! Inline function that writes the coordinates to a stream.
	(the coordinates are written in the following order: left, top, back, right, bottom, front).
	\param out	Stream buffer for cuboid output.
	\param cube	Cuboid to be printed.
*/
template <class T>
inline std::ostream& operator<<(std::ostream& out, const cuboid<T>& cube) {
	return cube.output(out);
}

/*! Inline function that reads the coordinates from a stream.
	(the coordinates are expected in the following order: left, top, back, right, bottom, front).
	The coordinates of 'cube' are only modified if ALL coordinates could be read
	correctly.
	\param in	Input stream for cuboid coodinates.
	\param cube	Cuboid to be read.
*/
template <class T>
inline std::istream& operator>>(std::istream& in, cuboid<T>& cube) {
	cuboid<T> tmp;
	T a,b,c,d,e,f;
	// in >> tmp.left >> tmp.top >> tmp.right >> tmp.bottom
	if (in >> a >> b >> c >> d ) {
		if ( in >> e >> f ) // can we read another two coordinates?
			tmp.set( a, b, c, d, e, f );
		else
			tmp.set( a, b, c, d, 0, 0 );  // just four coordinates
		// put data to the top level
		std::swap(tmp,cube);
	}

	return in;
}

/*! Compares two cuboids.
	\param	lhs First cuboid to compare
	\param	rhs Second cuboid to compare
	\return 'true' if they have the same coordinates, 'false' otherwise.
*/
template <class T>
inline bool operator==(const cuboid<T>& lhs, const cuboid<T>& rhs) {
	return (lhs.left == rhs.left && lhs.top==rhs.top &&
			lhs.right==rhs.right && lhs.bottom==rhs.bottom &&
			lhs.front==rhs.front && lhs.back==rhs.back);
}


} // namespace IBK

/*! \file IBK_Cuboid.h
	\brief Contains the declaration of template class cuboid.
*/

#endif // IBK_cuboidH
