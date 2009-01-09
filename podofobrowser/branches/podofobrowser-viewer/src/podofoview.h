/***************************************************************************
 *   Copyright (C) 2009 by Pierre Marchand   *
 *   pierremarc@oep-h.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef PODOFOVIEW_H
#define PODOFOVIEW_H

#include "viewer/core.h"

#include <QByteArray>
#include <QGraphicsView>
#include <QList>
#include <QMap>
#include <QRectF>

class QGraphicsRectItem;

class PodofoView : public QGraphicsView
{
	Q_OBJECT
	public:
		PodofoView ( QWidget * parent );
		void clear();
		void setGeometry ( QRectF rect=QRectF() );

// 		void setPageContext ( PoDoFo::PdfPage* theValue );
// 		void setXobjectContext ( PoDoFo::PdfXObject* theValue );
// 		void setDocument ( PoDoFo::PdfMemDocument* theValue );
		
		void showPage(PoDoFo::PdfPage * page, PoDoFo::PdfMemDocument* doc);
		void renderStream ( const QByteArray& stream );
		
	public slots:
		void reloadPage();
		
	private:
		void resetPDFMatrix();
		QList<QGraphicsItem*> m_itemList;
		QGraphicsRectItem * pageRect;

		GraphicState gState;
		PoDoFo::PdfPage *m_pageContext;
// 		PoDoFo::PdfXObject *m_xobjectContext;
		PoDoFo::PdfMemDocument * m_document;
		
				// All about interactive
	protected:
		void mousePressEvent ( QMouseEvent * e ) ;
		void mouseReleaseEvent ( QMouseEvent * e )  ;
		void mouseMoveEvent ( QMouseEvent * e ) ;
		void wheelEvent ( QWheelEvent * e );
		void resizeEvent ( QResizeEvent * event );
	private:
		QPointF mouseStartPoint;
		QGraphicsRectItem *theRect;
		bool isSelecting;
		bool isPanning;

};

#endif // PODOFOVIEW_H