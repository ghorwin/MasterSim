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

#include "BM_Globals.h"

#include <cmath>

namespace BLOCKMOD {

bool Globals::nearZero(double gridDistance) {
	return std::fabs(gridDistance/Globals::GridSpacing) < 1e-6;
}

double Globals::GridSpacing = 8; // in pixel

double Globals::LabelFontSize = 8;

const char * const Globals::InvisibleLabel = "[(-I am invisible-)]";

Theme Globals::CurrentTheme = Theme::light();


Theme Theme::light() {
	Theme t;
	t.gridMinor						= QColor(220,220,255);
	t.gridMajor						= QColor(175,175,255);
	t.blockGradientLeft				= QColor(196,196,255);
	t.blockGradientRight			= QColor(220,220,255);
	t.blockSelectedGradientLeft		= QColor(230,255,230);
	t.blockSelectedGradientRight	= QColor(200,240,180);
	t.blockBorder					= Qt::black;
	t.blockSelectedBorder			= QColor(0,128,0);
	t.blockText						= Qt::black;
	t.blockTintBase					= Qt::white;
	t.socketOutline					= Qt::black;
	t.socketInletFill				= Qt::white;
	t.socketConnectedMark			= Qt::black;
	t.socketOutletFill				= QColor(0,0,196);
	t.socketHoverOutline			= QColor(192,0,0);
	t.socketHoverFill				= QColor(96,0,0);
	t.socketLabel					= Qt::black;
	t.connector						= Qt::black;
	t.connectorHighlighted			= QColor(0,0,110);
	t.connectorSelected				= QColor(192,0,0);
	t.connectorLabelBackground		= Qt::white;
	return t;
}


Theme Theme::dark() {
	Theme t;
	t.gridMinor						= QColor(52,52,64);
	t.gridMajor						= QColor(72,72,110);
	t.blockGradientLeft				= QColor(58,58,105);
	t.blockGradientRight			= QColor(72,72,122);
	t.blockSelectedGradientLeft		= QColor(45,95,45);
	t.blockSelectedGradientRight	= QColor(60,110,48);
	t.blockBorder					= QColor(200,200,200);
	t.blockSelectedBorder			= QColor(110,210,110);
	t.blockText						= QColor(235,235,235);
	t.blockTintBase					= QColor(30,30,30);
	t.socketOutline					= QColor(200,200,200);
	t.socketInletFill				= QColor(42,42,42);
	t.socketConnectedMark			= QColor(225,225,225);
	t.socketOutletFill				= QColor(100,140,255);
	t.socketHoverOutline			= QColor(255,100,100);
	t.socketHoverFill				= QColor(200,60,60);
	t.socketLabel					= QColor(225,225,225);
	t.connector						= QColor(210,210,210);
	t.connectorHighlighted			= QColor(120,170,255);
	t.connectorSelected				= QColor(255,100,100);
	t.connectorLabelBackground		= QColor(42,42,42);
	return t;
}


ScopedTheme::ScopedTheme(const Theme & theme) :
	m_previousTheme(Globals::CurrentTheme)
{
	Globals::CurrentTheme = theme;
}


ScopedTheme::~ScopedTheme() {
	Globals::CurrentTheme = m_previousTheme;
}


} // namespace BLOCKMOD
