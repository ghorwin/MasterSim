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

#include "IBKMK_RectangleDecomposition.h"

#include <algorithm>
#include <string>
#include <map>
#include <list>

namespace IBKMK {

// Chord represents a connection between two points in (x/y)
struct Chord {
	unsigned int start_x;
	unsigned int start_y;
	unsigned int end_x;
	unsigned int end_y;
	Chord(unsigned int start_x, unsigned int start_y, unsigned int end_x, unsigned int end_y)
		: start_x(start_x), start_y(start_y), end_x(end_x), end_y(end_y) {}
	bool operator<(const Chord& other) const {
		if (start_x != other.start_x) return start_x < other.start_x;
		if (start_y != other.start_y) return start_y < other.start_y;
		if (end_x != other.end_x) return end_x < other.end_x;
		return end_y < other.end_y;
	}
	bool operator==(const Chord& other) const {
		return other.start_x == start_x && other.start_y == start_y && other.end_x == end_x && other.end_y == end_y;
	}
	std::string string() const {
		return "(("+std::to_string(start_x)+", "+std::to_string(start_y)+") - ("+std::to_string(end_x)+", "+std::to_string(end_y)+"))";
	}
};


void rectDecomposition(std::set<std::pair<unsigned int,unsigned int>> & shape,
					   std::set<std::pair<std::pair<unsigned int,unsigned int>,std::pair<unsigned int, unsigned int>>> & output) {

	// Get border dimensions
	unsigned int max_x = 0;
	unsigned int max_y = 0;
	for ( const std::pair<unsigned int,unsigned int> & node : shape) {
		max_x = std::max(max_x, node.first);
		max_y = std::max(max_y, node.second);
	}
	max_x++;
	max_y++;

	// Finding concave vertices
	std::set<std::pair<unsigned int,unsigned int>> concave;
	for (unsigned int x = 0; x < max_x; x++) {
		for (unsigned int y = 0; y < max_y; y++) {

			const bool x0y0 = shape.find(std::pair<unsigned int,unsigned int>(x,y)) != shape.end();
			const bool x1y0 = shape.find(std::pair<unsigned int,unsigned int>(x+1,y)) != shape.end();
			const bool x0y1 = shape.find(std::pair<unsigned int,unsigned int>(x,y+1)) != shape.end();
			const bool x1y1 = shape.find(std::pair<unsigned int,unsigned int>(x+1,y+1)) != shape.end();
			if ((int)x0y0 + (int)x1y0 + (int)x0y1 + (int)x1y1 == 3) {
				concave.insert(std::pair<unsigned int,unsigned int>(x+1,y+1));
			}
		}
	}

	// Finding chords between concave vertices
	std::set<Chord> chords;
	for (const std::pair<unsigned int,unsigned int> & vert : concave) {
		for (const std::pair<unsigned int,unsigned int> & vert2 : concave) {
			if ((vert.first < vert2.first && vert.second == vert2.second) || (vert.first == vert2.first && vert.second < vert2.second)) {
				chords.insert(Chord(vert.first, vert.second, vert2.first, vert2.second));
			}
		}
	}

	// Eliminating partly external or along-border chords
	std::set<Chord> chords_rm;
	for (const Chord & ch : chords) {
		// horizontal
		if (ch.start_x < ch.end_x) {
			for (unsigned int i = ch.start_x; i < ch.end_x; i++) {
				const bool above = shape.find(std::pair<unsigned int,unsigned int>(i,ch.start_y)) != shape.end();
				const bool below = shape.find(std::pair<unsigned int,unsigned int>(i,ch.start_y - 1)) != shape.end();
				if (!above || !below) {
					chords_rm.insert(ch);
					break;
				}
			}

		}
		// vertical
		else if (ch.start_y < ch.end_y) {
			for (unsigned int i = ch.start_y; i < ch.end_y; i++) {
				const bool right = shape.find(std::pair<unsigned int,unsigned int>(ch.start_x,i)) != shape.end();
				const bool left = shape.find(std::pair<unsigned int,unsigned int>(ch.start_x - 1,i)) != shape.end();
				if (!left || !right) {
					chords_rm.insert(ch);
					break;
				}
			}
		}
	}
	for (const Chord & rm : chords_rm) chords.erase(rm);

	// Detect intersections of chords
	std::map<Chord, std::set<Chord>> intersections;
	for (const Chord & chordA : chords) {
		for (const Chord & chordB : chords) {
			// Prevent chord interseciton with itself
			if (chordA == chordB)
				continue;

			// Detect common vertices
			if ((chordA.start_x == chordB.start_x && chordA.start_y == chordB.start_y) ||
				(chordA.start_x == chordB.end_x && chordA.start_y == chordB.end_y) ||
				(chordA.end_x == chordB.start_x && chordA.end_y == chordB.start_y) ||
				(chordA.end_x == chordB.end_x && chordA.end_y == chordB.end_y)) {
				intersections[chordA].insert(chordB);

			}
			// Detect intersections
			else {
				// Intuitively this next part looks like it neglects the case where a chord ends on another chord without sharing a vertex ( T shape ).
				// However this scenario is impossible, as one of these chords would have been eliminated by now due to not being fully contained within the shape.
				if (chordA.start_x < chordB.start_x && chordB.start_x < chordA.end_x && chordB.start_y < chordA.start_y && chordA.start_y < chordB.end_y) {
					intersections[chordA].insert(chordB);
				} else if (chordB.start_x < chordA.start_x && chordA.start_x < chordB.end_x && chordA.start_y < chordB.start_y && chordB.start_y < chordA.end_y) {
					intersections[chordA].insert(chordB);
				}
			}
		}
	}

	// Obtain largest set of independend chords with maximum flow heuristic, always keeping the chords with the lowest number of intersections
	// Therefore we always keep the chord with the lowest number of intersections, delete chords that are intersected by it from candidates, update
	// other intersections, and rerun algorithm from 0 until no more candidates are left.
	std::set<Chord> deleted_chords;
	std::set<Chord> finalchords;
	unsigned int i = 0;
	while (deleted_chords.size() < chords.size()) {
		for (const Chord & chord : chords) {
			if (deleted_chords.find(chord) == deleted_chords.end()) {
				if (intersections[chord].size() <= i) {
					finalchords.insert(chord);
					deleted_chords.insert(chord);

					std::set<Chord> intersections_chord_copy = intersections[chord];
					for (const Chord & intersected_chord : intersections_chord_copy) {

						if (deleted_chords.find(intersected_chord) == deleted_chords.end()) {
							deleted_chords.insert(intersected_chord);
							std::set<Chord> chords_deleted_chord_intersects = intersections[intersected_chord];
							for (const Chord & chord_intersects_with_deleted_chord : chords_deleted_chord_intersects) {
								intersections[chord_intersects_with_deleted_chord].erase(intersected_chord);
								// reset i to 0 to ensure we match the next lowest intersect number proceeding a deletion
								i = 0;
							}
						}
					}
				}
			}
		}
		i++;
	}

	// Find leftover isolated concave vertices
	std::set<std::pair<unsigned int,unsigned int>> isolated_concave_verts;
	bool isolated;
	for (const std::pair<unsigned int,unsigned int> & vert : concave) {
		isolated = true;
		for (const Chord & chord : finalchords) {
			if ((chord.start_x == vert.first && chord.start_y == vert.second) || (chord.end_x == vert.first && chord.end_y == vert.second)) {
				isolated = false;
				break;
			}
		}
		if (isolated) {
			isolated_concave_verts.insert(vert);
		}
	}

	// Decompose finalchords into single step chords for later processing
	std::set<Chord> decomposed_finalchords;
	for (const Chord & chord : finalchords) {
		std::pair<unsigned int,unsigned int> start = std::pair<unsigned int,unsigned int>(chord.start_x, chord.start_y);
		const std::pair<unsigned int,unsigned int> end = std::pair<unsigned int,unsigned int>(chord.end_x, chord.end_y);
		if (chord.start_x < chord.end_x) {
			// horizontal
			while (start != end) {
				decomposed_finalchords.insert(Chord(start.first, start.second, start.first+1, start.second));
				start = std::pair<unsigned int,unsigned int>(start.first+1, start.second);
			}
		} else {
			// vertical
			while (start != end) {
				decomposed_finalchords.insert(Chord(start.first, start.second, start.first, start.second+1));
				start = std::pair<unsigned int,unsigned int>(start.first, start.second+1);
			}
		}
	}

	// Explore new shapes after applying shape division by chords
	std::list<std::list<std::pair<unsigned int,unsigned int>>> new_shapes;
	while (shape.size() != 0) {
		std::list<std::pair<unsigned int,unsigned int>> new_shape;
		std::pair<unsigned int,unsigned int> first_node = *shape.begin();
		new_shape.push_back(first_node);
		shape.erase(first_node);
		for (const std::pair<unsigned int,unsigned int> & entry : new_shape) {
			std::set<std::pair<unsigned int,unsigned int>> explore;

			if (entry.first+1 < max_x && (decomposed_finalchords.find(Chord(entry.first+1, entry.second, entry.first+1, entry.second+1)) == decomposed_finalchords.end())) {
				explore.insert(std::pair<unsigned int,unsigned int>(entry.first+1, entry.second));
			}
			if (entry.first > 0 && (decomposed_finalchords.find(Chord(entry.first, entry.second, entry.first, entry.second+1)) == decomposed_finalchords.end())) {
				explore.insert(std::pair<unsigned int,unsigned int>(entry.first-1, entry.second));
			}
			if (entry.second+1 < max_y && (decomposed_finalchords.find(Chord(entry.first, entry.second+1, entry.first+1, entry.second+1)) == decomposed_finalchords.end())) {
				explore.insert(std::pair<unsigned int,unsigned int>(entry.first, entry.second+1));
			}
			if (entry.second > 0 && (decomposed_finalchords.find(Chord(entry.first, entry.second, entry.first+1, entry.second)) == decomposed_finalchords.end())) {
				explore.insert(std::pair<unsigned int,unsigned int>(entry.first, entry.second-1));
			}
			for (const std::pair<unsigned int,unsigned int> & node : explore) {
				if ((std::find(new_shape.begin(), new_shape.end(), node) == new_shape.end()) && shape.find(node) != shape.end()) {
					new_shape.push_back(node);
					shape.erase(node);
				}
			}
		}
		new_shapes.push_back(new_shape);
	}

	// Divide remainder surfaces into rectangles by constructing additional chords from isolated concave shape vertices (vertically, which could actually be arbitrary as stated in the source)
	std::set<std::list<std::pair<unsigned int,unsigned int>>> final_rects;

	bool is_rectangular;
	for (std::list<std::pair<unsigned int,unsigned int>> & processed_shape : new_shapes) {
		is_rectangular = true;
		for (const std::pair<unsigned int, unsigned int> & concave_vert : isolated_concave_verts) {
			unsigned int v_i = concave_vert.first;
			unsigned int v_j = concave_vert.second;
			unsigned int noderect = 0;

			if (std::find(processed_shape.begin(), processed_shape.end(), std::pair<unsigned int, unsigned int>(v_i,v_j)) != processed_shape.end()) {
				noderect++;
			}
			if (std::find(processed_shape.begin(), processed_shape.end(), std::pair<unsigned int, unsigned int>(v_i-1,v_j)) != processed_shape.end()) {
				noderect++;
			}
			if (std::find(processed_shape.begin(), processed_shape.end(), std::pair<unsigned int, unsigned int>(v_i,v_j-1)) != processed_shape.end()) {
				noderect++;
			}
			if (std::find(processed_shape.begin(), processed_shape.end(), std::pair<unsigned int, unsigned int>(v_i-1,v_j-1)) != processed_shape.end()) {
				noderect++;
			}
			if (noderect == 3) { // isolated vert fully located within this shape. This additional check is needed to prevent and endless loop after dividing the shape and processing halves
				std::set<Chord> new_separator;

				std::list<std::pair<unsigned int, unsigned int>> shape_left;
				std::list<std::pair<unsigned int, unsigned int>> shape_right;
				unsigned int y = concave_vert.second;

				if (std::find(processed_shape.begin(), processed_shape.end(), concave_vert) == processed_shape.end() ||
					std::find(processed_shape.begin(), processed_shape.end(), std::pair<unsigned int, unsigned int>(concave_vert.first-1, concave_vert.second)) == processed_shape.end()) {
					// one above missing
					shape_left.push_back(std::pair<unsigned int, unsigned int>(concave_vert.first-1, concave_vert.second-1));
					shape_right.push_back(std::pair<unsigned int, unsigned int>(concave_vert.first, concave_vert.second-1));
					while (std::find(processed_shape.begin(), processed_shape.end(), std::pair<unsigned int, unsigned int>(concave_vert.first-1, y-1)) != processed_shape.end() &&
						   std::find(processed_shape.begin(), processed_shape.end(), std::pair<unsigned int, unsigned int>(concave_vert.first, y-1)) != processed_shape.end()) {
						new_separator.insert(Chord(concave_vert.first, y-1, concave_vert.first, y));
						y--;
					}
				} else {
					// one below missing
					shape_left.push_back(std::pair<unsigned int, unsigned int>(concave_vert.first-1, concave_vert.second));
					shape_right.push_back(concave_vert);
					while (std::find(processed_shape.begin(), processed_shape.end(), std::pair<unsigned int, unsigned int>(concave_vert.first-1, y)) != processed_shape.end() &&
						   std::find(processed_shape.begin(), processed_shape.end(), std::pair<unsigned int, unsigned int>(concave_vert.first, y)) != processed_shape.end()) {
						new_separator.insert(Chord(concave_vert.first, y, concave_vert.first, y+1));
						y++;
					}
				}

					std::vector<std::list<std::pair<unsigned int, unsigned int>>> half_shapes;
					half_shapes.push_back(shape_left);
					half_shapes.push_back(shape_right);

					for (std::list<std::pair<unsigned int, unsigned int>> & half_shape : half_shapes) {
						for (const std::pair<unsigned int,unsigned int> & entry : half_shape) {
							std::set<std::pair<unsigned int,unsigned int>> explore;

							if (entry.first+1 < max_x && (new_separator.find(Chord(entry.first+1, entry.second, entry.first+1, entry.second+1)) == new_separator.end())) {
								explore.insert(std::pair<unsigned int,unsigned int>(entry.first+1, entry.second));
							}
							if (entry.first > 0 && (new_separator.find(Chord(entry.first, entry.second, entry.first, entry.second+1)) == new_separator.end())) {
								explore.insert(std::pair<unsigned int,unsigned int>(entry.first-1, entry.second));
							}
							if (entry.second+1 < max_y && (new_separator.find(Chord(entry.first, entry.second+1, entry.first+1, entry.second+1)) == new_separator.end())) {
								explore.insert(std::pair<unsigned int,unsigned int>(entry.first, entry.second+1));
							}
							if (entry.second > 0 && (new_separator.find(Chord(entry.first, entry.second, entry.first+1, entry.second)) == new_separator.end())) {
								explore.insert(std::pair<unsigned int,unsigned int>(entry.first, entry.second-1));
							}
							for (const std::pair<unsigned int,unsigned int> & node : explore) {
								if (std::find(half_shape.begin(), half_shape.end(), node) == half_shape.end() &&
									std::find(processed_shape.begin(), processed_shape.end(), node) != processed_shape.end()) {
									half_shape.push_back(node);
								}
							}
						}
					}

					bool valid_split = !new_separator.empty();
					for (const std::list<std::pair<unsigned int, unsigned int>> & half_shape : half_shapes) {
						if (half_shape.empty() || half_shape.size() >= processed_shape.size()) {
							valid_split = false;
							break;
						}
					}

					if (!valid_split)
						continue;

					is_rectangular = false;
					for (const std::list<std::pair<unsigned int, unsigned int>> & half_shape : half_shapes) {
						new_shapes.push_back(half_shape);
					}
					break;
				}
			}
		if (is_rectangular) {
			final_rects.insert(processed_shape);
		}
	}

	// Transform final rectangles into corner coordinate representation and update output
	for (const std::list<std::pair<unsigned int,unsigned int>> & final_rect : final_rects) {
		unsigned int min_rect_x = final_rect.begin()->first;
		unsigned int min_rect_y = final_rect.begin()->second;
		unsigned int max_rect_x = min_rect_x;
		unsigned int max_rect_y = min_rect_y;

		for (const std::pair<unsigned int, unsigned int> & patch : final_rect) {
			min_rect_x = std::min(min_rect_x, patch.first);
			min_rect_y = std::min(min_rect_y, patch.second);
			max_rect_x = std::max(max_rect_x, patch.first);
			max_rect_y = std::max(max_rect_y, patch.second);
		}
		output.insert(std::pair<std::pair<unsigned int, unsigned int>,std::pair<unsigned int, unsigned int>>
					  (std::pair<unsigned int, unsigned int>(min_rect_x, min_rect_y), std::pair<unsigned int, unsigned int>(max_rect_x, max_rect_y)));
	}
}

std::vector<Rectangle> rectDecomposition_Greedy(const std::vector<std::vector<std::string>>& input) {
	int rows = input.size();
	int cols = input[0].size();
	std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
	std::vector<Rectangle> rectangles;

	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			if (visited[y][x]) continue;

			std::string name = input[y][x];
			int x_end = x;

			// Wächst horizontale Linie (in dieser Zeile)
			while (x_end < cols && input[y][x_end] == name && !visited[y][x_end]) {
				++x_end;
			}

			int y_end = y + 1;
			bool canGrow = true;

			// Wächst vertikal solange alle Zellen gleichfarbig und unverarbeitet
			while (canGrow && y_end < rows) {
				for (int xi = x; xi < x_end; ++xi) {
					if (visited[y_end][xi] || input[y_end][xi] != name) {
						canGrow = false;
						break;
					}
				}
				if (canGrow) ++y_end;
			}

			// Rechteck speichern und alle Zellen markieren
			for (int yi = y; yi < y_end; ++yi) {
				for (int xi = x; xi < x_end; ++xi) {
					visited[yi][xi] = true;
				}
			}

			rectangles.push_back({x, y, --x_end, --y_end, name});
		}
	}

	return rectangles;
}

} // namespace IBKMK
