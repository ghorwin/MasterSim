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

#ifndef IBKMK_2DCalculationsH
#define IBKMK_2DCalculationsH

#include "IBKMK_Vector2D.h"

namespace IBKMK {

/*! Check for intersection of each edge of the polygon with the line(p1, p2). Returns true if intersection was found and in this
	case stores the computed intersection point.
*/
bool intersectsLine2D(const std::vector<Vector2D> & polygon,
		const IBK::point2D<double> &p1, const IBK::point2D<double> &p2, IBK::point2D<double> & intersectionPoint);

/*! Point in Polygon function. Result:
	-1 point not in polyline
	0 point on polyline
	1 point in polyline

	\param	point test point
	Source https://de.wikipedia.org/wiki/Punkt-in-Polygon-Test_nach_Jordan

*/
int pointInPolygon(const std::vector<Vector2D> & poly, const IBK::point2D<double> &p, unsigned int *idx = nullptr);

/*! Polygon in Polygon function:
 *  if poly1 in poly2, returns 1
 *  if poly2 in poly1, returns 2
 *  if intersect or disjuct, returns 0 */
int polygonInPolygon(const std::vector<Vector2D> & poly1, const std::vector<Vector2D> & poly2);

/*! TODO Stephan, document. */
bool polygonInOrOnPolygon(const std::vector<Vector2D> & poly1, const std::vector<Vector2D> & poly2, double epsilon = 0.1);

/*! checks if 3 points are in counterclockwise order */
bool counterClockwise(const Vector2D & a, const Vector2D & b, const Vector2D & c);

bool polygonClockwise(const std::vector<Vector2D>& polyline);

/*! checks if 2 linesegments intersect. https://bryceboe.com/2006/10/23/line-segment-intersection-algorithm/ */
bool lineSegmentIntersect(const Vector2D &a, const Vector2D &b, const Vector2D &c, const Vector2D &d);

/*! checks if 2 polylines intersect */
bool lineIntersect(const std::vector<IBKMK::Vector2D> line1, const std::vector<IBKMK::Vector2D> line2);

/*! checks if a point is near the segment defined by points a and b. epsilon defines the distance the point can have from the segment
 * to be considered near */
bool isPointNearSegment(const Vector2D& p, const Vector2D& a, const Vector2D& b, double epsilon = 0.1);

int crossProdTestWithEps(const Vector2D& p, const Vector2D& a, const Vector2D& b, double epsilon = 0.1);

/*! Point in Polygon function. Result:
	-1 point not in polyline
	0 point on polyline
	1 point in polyline
*/
int pointInPolygonFuzzy(const std::vector<Vector2D>& polygon, const Vector2D& p, double epsilon = 0.1);

/*! Eliminates collinear points in a polygon.
	All points that are closer together than the provided epsilon will be merged.
*/
void eliminateCollinearPoints(std::vector<IBKMK::Vector2D> & polygon, double epsilon = 1e-4);

/*! Takes the vector v and enlarges the current bounding box defined through 'minVec' and 'maxVec'. */
void enlargeBoundingBox(const IBKMK::Vector2D & v, IBKMK::Vector2D & minVec, IBKMK::Vector2D & maxVec);

/*! Takes two 2D polygons and checks for intersection */
bool polyIntersect2D(const std::vector<IBKMK::Vector2D> & vertsA, const std::vector<IBKMK::Vector2D> & vertsB);

} // namespace IBKMK

#endif // IBKMK_2DCalculationsH
