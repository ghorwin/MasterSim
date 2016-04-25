#ifndef IBK_pointH
#define IBK_pointH

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <cmath>
#include <iomanip>

namespace IBK {

/*! The class point2D represents a 2D point consisting of an x and y coordinate.
	This class is implemented as template so that it can be used for integer and
	floating point values.
*/
template <typename T>
class point2D {
public:
	/*! Default constructor. */
	point2D() : m_x(T()), m_y(T()) {}
	/*! Constructor (initialises with coordinates 'a' and 'b'). */
	point2D(T a, T b) : m_x(a), m_y(b) {}
	/*! Virtual destructor, so that destructor of derived classes is called. */
	virtual ~point2D() {}
	/*! Sets both coordinates. */
	void set(T a, T b) { m_x=a; m_y=b; }

	T m_x;   ///< The X-coordinate.
	T m_y;   ///< The Y-coordinate.
};

/*! Returns 'true' if the points 'lhs' and 'rhs' have the same coordinates. */
template <typename T>
inline bool operator == (const point2D<T>& lhs, const point2D<T>& rhs) {
	return lhs.m_x == rhs.m_x && lhs.m_y == rhs.m_y;
}

/*! Returns 'true' if the points 'lhs' and 'rhs' do not have the same coordinates. */
template <typename T>
inline bool operator!=(const point2D<T>& lhs, const point2D<T>& rhs) {
	return lhs.m_x != rhs.m_x || lhs.m_y != rhs.m_y;
}

/*! Returns 'true' if the 'lhs' has smaller coordinates than 'rhs', based on sequential comparison of x, and y values. */
template <typename T>
inline bool operator<(const point2D<T>& lhs, const point2D<T>& rhs) {
	if (lhs.m_x == rhs.m_x) {
		return lhs.m_y < rhs.m_y;
	}
	return lhs.m_x < rhs.m_x;
}


/*! Stream output operator for point2D objects.
	The point is written into the output stream in the format: 'x y'.
*/
template <typename T>
inline std::ostream& operator<<(std::ostream& out, const point2D<T>& p) {
	return out << p.m_x << ' ' << p.m_y;
}

/*! Stream input operator for point2D objects.
	The point is read from the output stream in the format: 'x y'.
*/
template <typename T>
inline std::istream& operator>>(std::istream& in, point2D<T>& p) {
	in >> p.m_x;
	in.clear();
	char komma = ' ';
	while (komma != ',' && in.good())
		in >> komma;
	in >> p.m_y;
	return in;
}




/*! The class point3D represents a 3D point consisting of an x, y and z coordinate.
	This class is implemented as template so that it can be used for integer and
	floating point values.
*/
template <typename T>
class point3D {
public:
	/*! Default constructor. */
	point3D() : m_x(T()), m_y(T()), m_z(T()) {}

	/*! Constructor (initialises with coordinates 'a', 'b' and 'c'). */
	point3D(T a, T b, T c) : m_x(a), m_y(b), m_z(c) {}

	/*! Virtual destructor, so that destructor of derived classes is called. */
	virtual ~point3D() {}

	/*! Sets all coordinates. */
	void set(T x, T y, T z) {
		m_x = x;
		m_y = y;
		m_z = z;
	}

	T				m_x;	///< The X-coordinate.
	T				m_y;	///< The Y-coordinate.
	T				m_z;	///< The Z-coordinate.
};

/*! Returns 'true' if the points 'lhs' and 'rhs' have the same coordinates. */
template <typename T>
inline bool operator == (const point3D<T>& lhs, const point3D<T>& rhs) {
	return lhs.m_x == rhs.m_x && lhs.m_y == rhs.m_y && lhs.m_z == rhs.m_z;
}

/*! Returns 'true' if the points 'lhs' and 'rhs' do not have the same coordinates. */
template <typename T>
inline bool operator!=(const point3D<T>& lhs, const point3D<T>& rhs) {
	return lhs.m_x != rhs.m_x || lhs.m_y != rhs.m_y || lhs.m_z != rhs.m_z;
}

/*! Returns 'true' if the 'lhs' has smaller coordinates than 'rhs', based on sequential comparison of x, y, and z values. */
template <typename T>
inline bool operator<(const point3D<T>& lhs, const point3D<T>& rhs) {
	if (lhs.m_x == rhs.m_x) {
		if (lhs.m_y == rhs.m_y) {
			return lhs.m_z < rhs.m_z;
		}
		return lhs.m_y < rhs.m_y;
	}
	return lhs.m_x < rhs.m_x;
}

/*! Stream output operator for point3D objects.
	The point is written into the output stream in the format: 'x y z'.
*/
template <typename T>
inline std::ostream& operator<<(std::ostream& out, const point3D<T>& p) {
	return out << p.m_x << ' ' << p.m_y << ' ' << p.m_z;
}

/*! Stream input operator for point3D objects.
	The point is read from the output stream in the format: 'x y z'.
*/
template <typename T>
inline std::istream& operator>>(std::istream& in, point3D<T>& p) {
	in >> p.m_x;
	in.clear();
	char komma = ' ';
	while (komma != ',' && in.good())
			in >> komma;
	in >> p.m_y;
	in.clear();
	char komma2 = ' ';
	while (komma2 != ',' && in.good())
			in >> komma2;
	in >> p.m_z;
	return in;
}


} // namespace IBK

/*! \file IBK_point.h
	\brief Contains the declarations of the template classes point2D and point3D.
*/

#endif // IBK_pointH

