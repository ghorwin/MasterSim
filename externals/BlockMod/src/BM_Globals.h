/*	BSD 3-Clause License

	This file is part of the BlockMod Library.

	Copyright (c) 2019, Andreas Nicolai
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its
	   contributors may be used to endorse or promote products derived from
	   this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
	FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BM_GlobalsH
#define BM_GlobalsH

#include <QColor>

namespace BLOCKMOD {

/*! Colors used to paint blocks, sockets, connectors and the grid.
	Use Theme::light() (default) or Theme::dark() or customize individual colors and
	set them via Globals::CurrentTheme. Afterwards, update the scenes/views.
*/
struct Theme {
	/*! Default light theme (for light widget palettes and printing/exporting). */
	static Theme light();
	/*! Dark theme (for dark widget palettes). */
	static Theme dark();

	QColor gridMinor;
	QColor gridMajor;

	QColor blockGradientLeft;
	QColor blockGradientRight;
	QColor blockSelectedGradientLeft;
	QColor blockSelectedGradientRight;
	QColor blockBorder;
	QColor blockSelectedBorder;
	QColor blockText;
	/*! Color that slave-specific block colors are blended with (white in light themes). */
	QColor blockTintBase;

	QColor socketOutline;
	QColor socketInletFill;
	QColor socketConnectedMark;
	QColor socketOutletFill;
	QColor socketHoverOutline;
	QColor socketHoverFill;
	QColor socketLabel;

	/*! Used for connectors without explicitly set color. */
	QColor connector;
	QColor connectorHighlighted;
	QColor connectorSelected;
	QColor connectorLabelBackground;
};


/*! Temporarily switches Globals::CurrentTheme (e.g. to light theme for printing/exporting),
	restores the previous theme on destruction.
*/
class ScopedTheme {
public:
	explicit ScopedTheme(const Theme & theme);
	~ScopedTheme();
private:
	ScopedTheme(const ScopedTheme &) = delete;
	ScopedTheme & operator=(const ScopedTheme &) = delete;
	Theme m_previousTheme;
};


class Globals {
public:

	/*! Tests, if a given grid distance is approximately zero (with respect to grid spacing).
		Function accounts for rounding errors.
	*/
	static bool nearZero(double gridDistance);

	/*! The grid spacing, used to align blocks/connectors/sockets and snap to while moving. */
	static double GridSpacing;

	/*! Size of labels to draw on sockets. */
	static double LabelFontSize;

	/*! Constant to identify hidden block used during connection operation. */
	static const char * const InvisibleLabel;

	/*! Colors used for painting (default: Theme::light()). */
	static Theme CurrentTheme;
};

} // namespace BLOCKMOD


#ifndef ASCONST_DEFINED
#define ASCONST_DEFINED
/*! Portable replacement for qAsConst() (deprecated Qt 6.6) and std::as_const() (C++17).
	Works with any C++11 compiler and any Qt version. */
template <typename T>
constexpr const T & asConst(T & t) noexcept { return t; }
/*! Deleted overload prevents binding to rvalues (would produce a dangling reference). */
template <typename T>
void asConst(const T &&) = delete;
#endif // ASCONST_DEFINED


#endif // BM_GlobalsH
