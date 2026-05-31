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

#include "IBKMK_2DCalculations.h"
#include "IBK_Line.h"
#include "IBK_MessageHandler.h"
#include "IBK_messages.h"

namespace IBKMK {

bool intersectsLine2D(const std::vector<Vector2D> & polygon,
					  const IBK::point2D<double> &p1, const IBK::point2D<double> &p2, IBK::point2D<double> & intersectionPoint)
{
	IBK::Line line(p1, p2);
	unsigned int polySize = (unsigned int)polygon.size();
	for (unsigned int i=0; i<polySize; ++i){
		IBK::Line otherLine(polygon[i], polygon[(i+1)%polySize]);

		// TODO : enhance intersectsLine2D  function to return also two intersection points
		IBK::point2D<double> intersectionPoint2;
		if (line.intersects(otherLine, intersectionPoint, intersectionPoint2) > 0)
			return true;
	}
	return false;
}


static int crossProdTest(const IBKMK::Vector2D & a, IBKMK::Vector2D b, IBKMK::Vector2D c) {
	const unsigned TOLERANCE_DIGITS = 4;
	if (a.equal<TOLERANCE_DIGITS>(b))
		return 0; // fuzzy comparison

	if (a.m_y == b.m_y && a.m_y == c.m_y) {
		if (	(b.m_x<= a.m_x && a.m_x <= c.m_x) ||
				(c.m_x<= a.m_x && a.m_x <= b.m_x) )
			return 0;
		else
			return 1;
	}

	if (b.m_y > c.m_y)
		std::swap(b,c);

	if (a.m_y <= b.m_y || a.m_y > c.m_y)
		return 1;

	double delta = (b.m_x - a.m_x) * (c.m_y - a.m_y) -(b.m_y - a.m_y) * (c.m_x - a.m_x);
	if(delta > 0)			return	1;
	else if(delta < 0)		return	-1;
	else					return	0;
}


/* Point in Polygon function. Result:
	-1 point not in polyline
	0 point on polyline
	1 point in polyline

	\param	point test point
	Source https://de.wikipedia.org/wiki/Punkt-in-Polygon-Test_nach_Jordan

*/
int pointInPolygon(const std::vector<Vector2D> & polygon, const IBK::point2D<double> &p, unsigned int *idx) {
	int t=-1;
	size_t polySize = polygon.size();
	unsigned int i=0;
	for (;i<polySize; ++i) {
		t *= crossProdTest(p, polygon[i], polygon[(i+1) % polySize]);
		if (t==0)
			break;
	}

	if (idx != nullptr)
		*idx = i;

	return t;
}

bool isPointNearSegment(const Vector2D& p, const Vector2D& a, const Vector2D& b, double epsilon) {
	Vector2D ab = b - a;
	Vector2D ap = p - a;

	// Length of line segment
	double lengthOfSegment = ab.m_x * ab.m_x + ab.m_y * ab.m_y;

	// if segment is a point return
	if (lengthOfSegment == 0.0) {
		return (ap.m_x * ap.m_x + ap.m_y * ap.m_y) <= epsilon * epsilon;
	}

	// Project point onto line segment
	double t = std::max(0.0, std::min(1.0, (ap.m_x * ab.m_x + ap.m_y * ab.m_y) / lengthOfSegment));
	Vector2D projection = a + ab * t;

	// Check distance from point to projection
	Vector2D dist = p - projection;
	return (dist.m_x * dist.m_x + dist.m_y * dist.m_y) <= epsilon * epsilon;
}

int crossProdTestWithEps(const Vector2D& p, const Vector2D& a, const Vector2D& b, double epsilon) {
	// If point is near the edge, consider it on the polygon
	if (isPointNearSegment(p, a, b, epsilon)) {
		return 0;
	}

	// Traditional cross product test
	double cross = (b.m_x - a.m_x) * (p.m_y - a.m_y) - (b.m_y - a.m_y) * (p.m_x - a.m_x);

	if (std::abs(cross) < epsilon) return 0;
	return (cross > 0) ? 1 : -1;
}

int pointInPolygonFuzzy(const std::vector<Vector2D>& polygon, const Vector2D& p, double epsilon)
{
	if (polygon.size() < 3) return -1; // Invalid polygon

	int t = 0;
	size_t polySize = polygon.size();

	for (size_t i = 0; i < polySize; ++i) {
		const Vector2D& current = polygon[i];
		const Vector2D& next = polygon[(i + 1) % polySize];

		// Check if point is on or very close to the current edge
		if (isPointNearSegment(p, current, next, epsilon)) {
			return 0; // Point is on the polygon
		}

		// Ray-casting test with robustness checks
		if (current.m_y <= p.m_y) {
			if (next.m_y > p.m_y) {
				int cross = crossProdTestWithEps(p, current, next, epsilon);
				if (cross == 0) {
					return 0;
				}
				if (cross > 0) t++;
			}
		} else {
			if (next.m_y <= p.m_y) {
				int cross = crossProdTestWithEps(p, current, next, epsilon);
				if (cross == 0) {
					return 0;
				}
				if (cross < 0) t--;
			}
		}
	}

	return (t != 0) ? 1 : -1;
}

// This is wrapper function around:
//   polygon.erase(polygon.begin()+idx)
// with additional assets and debug output.
void erasePoint(std::vector<IBKMK::Vector2D> & polygon, unsigned int idx) {
	IBK_ASSERT(idx < polygon.size());
	std::vector<IBKMK::Vector2D>::iterator it = polygon.begin() + idx;
#ifdef IBK_DEBUG
	//		IBK::IBK_Message(IBK::FormatString("Point to be erased: X %1 Y %2").arg(it->m_x).arg(it->m_y), IBK::MSG_DEBUG);
#endif
	polygon.erase(it);
}


void eliminateCollinearPoints(std::vector<IBKMK::Vector2D> & polygon, double epsilon) {
	if (polygon.size()<2)
		return;

	// check for duplicate points in polyline and remove duplicates
	//
	// the algorithm works as follows:
	// - we start at current index 0
	// - we compare the vertex at current index with that of the next vertex
	//    - if both are close enough together, we elminate the current vertex and try again
	//    - otherwise both vertexes are ok, and we advance the current index
	// - algorithm is repeated until we have processed the last point of the polygon
	// Note: when checking the last point of the polygon, we compare it with the first vertex (using modulo operation).
	unsigned int i=0;
	double eps2 = epsilon*epsilon;
	while (polygon.size() > 1 && i < polygon.size()) {
		// distance between current and next point
		IBKMK::Vector2D diff = polygon[i] - polygon[(i+1) % polygon.size()]; // Note: when i = size-1, we take different between last and first element
		if (diff.magnitudeSquared() < eps2)
			erasePoint(polygon, i); // remove point and try again
		else
			++i;
	}

	// Now we have only different points. We process the polygon again and remove all vertexes between collinear edges.
	// The algorithm uses the following logic:
	//    - take 3 subsequent vertexes, compute lines from vertex 1-2 and 1-3
	//    - compute projection of line 1 onto 2
	//    - compute projected end point of line 1 in line 2
	//    - if distance between projected point and original vertex 2 is < epsison, remove vertex 2

	i=0;
	while (polygon.size() > 1 && i < polygon.size()) {
		// we check if we can remove the current vertex i
		// take the last and next vertex
		const IBKMK::Vector2D & last = polygon[(i + polygon.size() - 1) % polygon.size()];
		const IBKMK::Vector2D & next = polygon[(i+1) % polygon.size()];
		// compute vertex a and b and normalize
		IBKMK::Vector2D a = next - last; // vector to project on
		IBKMK::Vector2D b = polygon[i] - last; // vector that shall be projected
		double anorm2 = a.magnitudeSquared();
		if (anorm2 < eps2) {
			// next and last vectors are nearly identical, hence we have a "spike" geometry and need to remove
			// both the spike and one of the identical vertexes
			erasePoint(polygon, i); // remove point (might be the last)
			if (i < polygon.size())
				erasePoint(polygon, i); // not the last point, remove next as well
			else
				erasePoint(polygon, i-1); // remove previous point
			continue;
		}

		// compute projection vector
		IBKMK::Vector2D astar = a.scalarProduct(b)/anorm2 * a;
		// get vertex along vector b
		astar += last;
		// compute difference
		IBKMK::Vector2D diff = polygon[i] - astar;
		if (diff.magnitudeSquared() < eps2)
			erasePoint(polygon, i); // remove point and try again
		else
			++i;
	}
}


void enlargeBoundingBox(const Vector2D & v, Vector2D & minVec, Vector2D & maxVec) {
	minVec.m_x = std::min(minVec.m_x, v.m_x);
	minVec.m_y = std::min(minVec.m_y, v.m_y);

	maxVec.m_x = std::max(maxVec.m_x, v.m_x);
	maxVec.m_y = std::max(maxVec.m_y, v.m_y);
}

bool polyIntersect2D(const std::vector<Vector2D> & vertsA, const std::vector<Vector2D> & vertsB) {
	IBKMK::Vector2D intersectP;
	for (unsigned int i = 0, count = vertsA.size(); i<count; ++i ) {
		// check if any of the edge centerpoints are contained within the other polygon
		// this is a catch-all solution for all cases of partial overlapping intersections that
		// would go undetected by testing for edge intersections, as well as edge-intersection-free containment
		if (IBKMK::pointInPolygon(vertsB, (vertsA[(i+1)%vertsA.size()]+vertsA[i])*0.5) == 1) {
			return true;
		}
		// check for 2D polygon line intersections
		if (IBKMK::intersectsLine2D(vertsB, vertsA[(i+1)%vertsA.size()], vertsA[i], intersectP)) {
			return true;
		}
	}
	for (unsigned int i = 0, count = vertsB.size(); i<count; ++i ) {
		if (IBKMK::pointInPolygon(vertsA, (vertsB[(i+1)%vertsB.size()]+vertsB[i])*0.5) == 1) {
			return true;
		}
		if (IBKMK::intersectsLine2D(vertsA, vertsB[(i+1)%vertsB.size()], vertsB[i], intersectP)) {
			return true;
		}
	}
	return false;
}

bool lineSegmentIntersect(const Vector2D & a, const Vector2D & b, const Vector2D & c, const Vector2D & d) {
	return counterClockwise(a,c,d) != counterClockwise(b,c,d) && counterClockwise(a,b,c) != counterClockwise(a,b,d);
}

bool lineIntersect(const std::vector<Vector2D> line1, const std::vector<Vector2D> line2) {
	for (size_t i = 0; i < line1.size() - 1; ++i) {
		for (size_t j = 0; j < line2.size() - 1; ++j) {
			if (lineSegmentIntersect(line1[i], line1[i+1], line2[j], line2[j+1])) {
				return true;
			}
		}
	}
	return false;
}

int polygonInPolygon(const std::vector<Vector2D> & poly1, const std::vector<Vector2D> & poly2) {
	bool poly1InPoly2 = true;
	for (const Vector2D& coord : poly1) {
		if (pointInPolygon(poly2, coord) != 1) {
			poly1InPoly2 = false;
			break;
		}
	}
	if (poly1InPoly2) return 1;
	bool poly2InPoly1 = true;
	for (const Vector2D& coord : poly2) {
		if (pointInPolygon(poly1, coord) != 1) {
			poly2InPoly1 = false;
			break;
		}
	}

	if(poly2InPoly1) return 2;
	return 0;
}

bool polygonInOrOnPolygon(const std::vector<Vector2D> & poly1, const std::vector<Vector2D> & poly2, double epsilon) {
	bool poly1InPoly2 = true;
	for (const Vector2D& coord : poly1) {
		if (pointInPolygonFuzzy(poly2, coord, epsilon) < 0) {
			poly1InPoly2 = false;
			break;
		}
	}

	return poly1InPoly2;
}

bool counterClockwise(const Vector2D & a, const Vector2D & b, const Vector2D & c) {
	return (c.m_y-a.m_y)*(b.m_x-a.m_x) > (b.m_y - a.m_y)*(c.m_x - a.m_x);
}

bool polygonClockwise(const std::vector<Vector2D>& polyline) {
	if (polyline.size() < 3) {
		// A polyline needs at least 3 points to form an area
		return false;
	}

	double area = 0.0;
	int n = polyline.size();

	for (int i = 0; i < n; ++i) {
		const Vector2D& current = polyline[i];
		const Vector2D& next = polyline[(i + 1) % n];

		area += (next.m_x - current.m_x) * (next.m_y + current.m_y);
	}

	return area > 0;
}



} // namespace IBKMK
