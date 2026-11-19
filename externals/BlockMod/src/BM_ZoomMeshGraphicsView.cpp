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

#include "BM_ZoomMeshGraphicsView.h"

#include <QWheelEvent>
#include <QApplication>
#include <QPainter>
#include <QScrollBar>

#include <algorithm>
#include <cmath>

#include "BM_SceneManager.h"

namespace BLOCKMOD {

ZoomMeshGraphicsView::ZoomMeshGraphicsView(QWidget *parent) :
	QGraphicsView(parent),
	m_resolution(1000), // 1000 px/m
	m_gridStep(0.1),
	m_gridEnabled( true ),
	m_zoomLevel(0),
	m_gridColor( 175, 175, 255 ),
	m_gridSpacingPixLast(0)
{
	setTransformationAnchor(AnchorUnderMouse);

	// this saves our overlays while drawing
	setViewportUpdateMode( QGraphicsView::FullViewportUpdate );
}


void ZoomMeshGraphicsView::wheelEvent(QWheelEvent *i_event){
	if(m_middleButtonPressed) {
		i_event->ignore();
		return;
	}
	if (i_event->angleDelta().y() < 0) {
		zoomOut();
	}
	else {
		zoomIn();
	}
	i_event->accept();
}


void ZoomMeshGraphicsView::mousePressEvent(QMouseEvent *event) {
	if (event->button() == Qt::MiddleButton) {
		m_middleButtonPressed = true;
		m_pos = event->position();
		setCursor(Qt::ClosedHandCursor);
		event->accept();
		return;
	}
	m_pos = event->position();
	QGraphicsView::mousePressEvent(event);
}


void ZoomMeshGraphicsView::mouseReleaseEvent(QMouseEvent *event) {
	if (event->button() == Qt::MiddleButton) {
		m_middleButtonPressed = false;
		setCursor(Qt::ArrowCursor);
		event->accept();
		// pass on as regular mouse move to update hover state of scene items
		mouseMoveEvent(event);
		return;
	}
	m_pos = event->position();
	QGraphicsView::mouseReleaseEvent(event);
}


void ZoomMeshGraphicsView::paintEvent(QPaintEvent *i_event){

	if (m_gridEnabled) {

		// view coordinates of scene origin
		QPointF p1 = mapFromScene( QPointF(0,0) );

		// compute grid spacing in pixels using resolution and current scaling
		const double scaleFactor = transform().m11();
		const double gridSpacingPix = m_resolution*m_gridStep*scaleFactor;

		// get dimensions of the drawing area (excluding scroll bars)
		const QSize windowSize = viewport()->size();

		// if reference point, grid spacing and window size still match cached values, do
		// not recompute grid lines
		if (gridSpacingPix != m_gridSpacingPixLast ||
			p1 != m_pLast ||
			windowSize != m_windowSizeLast)
		{
			// cache values used for calculation
			m_pLast = p1;
			m_gridSpacingPixLast = gridSpacingPix;
			m_windowSizeLast = windowSize;

			computeGridLines(m_majorGrid, p1, gridSpacingPix, windowSize);
			// minor grid has 10 times finer spacing
			computeGridLines(m_minorGrid, p1, gridSpacingPix*0.1, windowSize);
		}

		// we paint in view coordinates
		QPainter p(viewport());
		p.setPen( QColor(220,220,255) );
		p.drawLines(m_minorGrid.data(), m_minorGrid.size());

		p.setPen( m_gridColor );
		p.drawLines(m_majorGrid.data(), m_majorGrid.size());
	} // painter must be destroyed before QGraphicsView paints on the viewport

	QGraphicsView::paintEvent(i_event);
}


void ZoomMeshGraphicsView::computeGridLines(QVarLengthArray<QLineF, 100> & lines, const QPointF & origin,
											double gridSpacingPix, const QSize & windowSize)
{
	lines.clear();
	// only generate grid lines if spacing is big enough
	if (gridSpacingPix < 5)
		return;

	const int w = windowSize.width();
	const int h = windowSize.height();

	// calculate offset of first grid line in view - grid lines pass through scene origin;
	// number of lines is kept as double to avoid int overflow at extreme zoom levels
	const double linesXTillView = std::floor(-origin.x()/gridSpacingPix + 1);
	const double offsetX = linesXTillView*gridSpacingPix + origin.x();

	const double linesYTillView = std::floor(-origin.y()/gridSpacingPix + 1);
	const double offsetY = linesYTillView*gridSpacingPix + origin.y();

	for (double x = offsetX; x < w; x += gridSpacingPix)
		lines.append(QLineF(x, 0, x, h));
	for (double y = offsetY; y < h; y += gridSpacingPix)
		lines.append(QLineF(0, y, w, y));
}


void ZoomMeshGraphicsView::enterEvent(QEnterEvent * event) {
	Q_ASSERT(event->type() == QEvent::Enter);

	SceneManager * sceneManager = qobject_cast<SceneManager *>(scene());
	if (sceneManager) {
		// clear any override cursors that we had enabled when leaving the scene
		while (QApplication::overrideCursor() != nullptr)
			QApplication::restoreOverrideCursor();
	}
	QGraphicsView::enterEvent(event);
}


void ZoomMeshGraphicsView::leaveEvent(QEvent *event) {
	SceneManager * sceneManager = qobject_cast<SceneManager *>(scene());
	if (sceneManager) {
		if (sceneManager->isCurrentlyConnecting())
			sceneManager->finishConnection();
	}
	QApplication::restoreOverrideCursor(); // needed if we drag a segment out of the view
	QGraphicsView::leaveEvent(event);
}


void ZoomMeshGraphicsView::mouseMoveEvent(QMouseEvent *i_event) {
	if (m_middleButtonPressed) {
		const QPointF delta = i_event->position() - m_pos;
		horizontalScrollBar()->setValue(horizontalScrollBar()->value() - qRound(delta.x()));
		verticalScrollBar()->setValue(verticalScrollBar()->value() - qRound(delta.y()));
		m_pos = i_event->position();
		i_event->accept();
		return;
	}
	else {
		QGraphicsView::mouseMoveEvent(i_event);
		m_pos = i_event->position(); // view coordinates, same as in mousePressEvent()
		viewport()->update();
	}
}


void ZoomMeshGraphicsView::setGridColor( QColor color ) {
	m_gridColor = color;
	viewport()->update();
}


void ZoomMeshGraphicsView::setGridEnabled( bool enabled ) {
	m_gridEnabled = enabled;
	viewport()->update();
}


void ZoomMeshGraphicsView::zoomIn() {
	setZoomLevel(m_zoomLevel+1);
}


void ZoomMeshGraphicsView::zoomOut() {
	setZoomLevel(m_zoomLevel-1);
}


void ZoomMeshGraphicsView::setZoomLevel(int zoomLevel) {
	m_zoomLevel = std::clamp(zoomLevel, -3000, 3000);
	double factor = std::pow(10, m_zoomLevel/20.0);
	setTransform(QTransform(factor, 0, 0, factor, 0, 0));
	changeResolutionEvent();
}


void ZoomMeshGraphicsView::resetZoom() {
	setZoomLevel(0);
}


void ZoomMeshGraphicsView::setGridStep(double gridStep) {
	if (gridStep <=0) return;
	m_gridStep = gridStep;
	viewport()->update();
}


void ZoomMeshGraphicsView::setResolution(double res) {
	if (res <=0) return;
	m_resolution = res;
	viewport()->update();
}

} // namespace BLOCKMOD
