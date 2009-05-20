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

#include "podofoview.h"

#include "viewer/dispatcher.h"
#include "textmaker.h"

#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QTransform>

#include <QWheelEvent>
#include <QScrollBar>
#include <QApplication>
#include <QDesktopWidget>

#include <QDebug>


PodofoView::PodofoView ( QWidget * parent )
		:QGraphicsView ( parent )
{
	setScene ( new QGraphicsScene );
	
	setRenderHint(QPainter::Antialiasing, true);
	setRenderHint(QPainter::TextAntialiasing, true);
	setRenderHint(QPainter::SmoothPixmapTransform, true);
	setBackgroundBrush(Qt::lightGray);
	resetPDFMatrix();
	pageRect = new QGraphicsRectItem(QRectF());
	scene()->addItem(pageRect);
	pageRect->setBrush(Qt::white);
	
	theRect = scene()->addRect ( QRectF(),QPen ( QColor ( 10,10,200 ) ), QColor ( 10,10,200,100 ) );
	theRect->setZValue ( 1000000.0 );
	setInteractive ( true );
	isSelecting = false;
	isPanning = false;
	setAlignment ( Qt::AlignVCenter | Qt::AlignHCenter );
	setTransformationAnchor ( QGraphicsView::NoAnchor );

}

void PodofoView::resetPDFMatrix()
{
	double horiScaleT (( double ) QApplication::desktop()->physicalDpiX() / 72.0);
	double vertScaleT ( ( double ) QApplication::desktop()->physicalDpiY() / 72.0);
// 	qDebug()<<"S"<<horiScaleT<<horiScaleT;
	QTransform t(horiScaleT,0,0,-vertScaleT,0,0);
// 	t.scale ( horiScaleT,-vertScaleT );
	setTransform ( t, false );
}



void PodofoView::clear()
{
	foreach ( QGraphicsItem *item, m_itemList )
	{
		if ( item )
			delete item;
	}
	m_itemList.clear();
	setSceneRect ( QRectF() );
	pageRect->setRect( QRectF() );
}

void PodofoView::setGeometry ( QRectF rect )
{
	setSceneRect ( rect );
	pageRect->setRect(rect);
}


void PodofoView::renderStream ( const QByteArray & stream )
{
	PoDoFo::PdfContentsTokenizer * tokenizer = new PoDoFo::PdfContentsTokenizer ( stream.data(), stream.length() );

	const char*      pszToken = NULL;
	PoDoFo::PdfVariant       var;
	PoDoFo::EPdfContentsType eType;
	std::string      str;

	PdfContentStream contentStream;

	QList<PoDoFo::PdfVariant> stack;

	while ( tokenizer->ReadNext ( eType, pszToken, var ) )
	{
		if ( eType == PoDoFo::ePdfContentsType_Keyword )
		{
			contentStream << PdfOperation ( QString::fromAscii ( pszToken ), stack );
			stack.clear();
		}
		else if ( eType == PoDoFo::ePdfContentsType_Variant )
		{
			stack << var;
		}
		else
		{
			// Impossible; type must be keyword or variant
			qWarning ( "Impossible; type must be keyword or variant" );
		}
	}

	delete tokenizer;
// 	qDebug() <<"Content Stream tokenized";
	

	PdfContentIterator it ( contentStream.begin() );
	Dispatcher *d(Dispatcher::getInstance());
	while ( it != contentStream.end() )
	{
		it = d->run(it, gState);
	}


}



// void PodofoView::setPageContext ( PoDoFo::PdfPage* theValue )
// {
// 	m_pageContext = theValue;
// 	m_xobjectContext = 0;
// }
// 
// 
// void PodofoView::setXobjectContext ( PoDoFo::PdfXObject* theValue )
// {
// 	m_xobjectContext = theValue;
// 	m_pageContext = 0;
// }


// void PodofoView::setDocument ( PoDoFo::PdfMemDocument* theValue )
// {
// 	m_document = theValue;
// }

void PodofoView::showPage(PoDoFo::PdfPage * page, PoDoFo::PdfMemDocument* doc)
{
// 	qDebug()<<"showPage";
	clear();
	m_document = doc;
	
	
	gState.Reset();
	GraphicItemMakerBase* gimBase( Dispatcher::getInstance()->getMaker(QString::fromUtf8("TextMaker")) );
	if(gimBase)
	{
		TextMaker * gimText = reinterpret_cast<TextMaker*>(gimBase);
		if(gimText)
			gimText->reset();
	}
	
	gState.document = m_document;
	gState.page = page;
        gState.resource = 0;
	
	gState.scene = scene();
	gState.itemList = &m_itemList;
	

	PoDoFo::PdfRect mb(page->GetMediaBox());
	QRectF pr(mb.GetLeft(), mb.GetBottom(), mb.GetWidth(), mb.GetHeight());
	setGeometry(pr);
	
	PoDoFo::PdfObject * content (page->GetContents());
	PoDoFo::PdfMemoryOutputStream bufferStream ( 1 );
	
	if(content->HasStream())
	{	
		const PoDoFo::PdfStream * const stream = content->GetStream();
		try
		{
			stream->GetFilteredCopy ( &bufferStream );
		}
		catch ( PoDoFo::PdfError & e )
		{
			return;
		}
	}
	else if(content->IsArray())
	{
		PoDoFo::PdfArray carray ( page->GetContents()->GetArray() );
		for ( unsigned int ci = 0; ci < carray.GetSize(); ++ci )
		{
			if ( carray[ci].HasStream() )
			{
				try
				{
					carray[ci].GetStream()->GetFilteredCopy ( &bufferStream );
				}
				catch ( PoDoFo::PdfError & e )
				{
					return;
				}
				
			}
			else if ( carray[ci].IsReference() )
			{
				PoDoFo::PdfObject *co = m_document->GetObjects().GetObject ( carray[ci].GetReference() );

				while ( co != NULL )
				{
					if ( co->IsReference() )
					{
						co = m_document->GetObjects().GetObject ( co->GetReference() );
					}
					else if ( co->HasStream() )
					{
						try
						{
							co->GetStream()->GetFilteredCopy ( &bufferStream );
						}
						catch ( PoDoFo::PdfError & e )
						{
							return;
						}
						break;
					}
				}

			}

		}
	}
	
	bufferStream.Close();
	QByteArray ba ( bufferStream.TakeBuffer(), bufferStream.GetLength() );
	renderStream ( ba );
}

void PodofoView::reloadPage()
{
	showPage(gState.page, gState.document); // tough job
}

void PodofoView::mousePressEvent(QMouseEvent * e)
{
	if ( e->button() == Qt::MidButton )
	{
		mouseStartPoint =  e->pos() ;
		isPanning = true;
		QApplication::setOverrideCursor (QCursor(Qt::ClosedHandCursor));
	}
	else
	{
		mouseStartPoint = mapToScene ( e->pos() );
		isSelecting = true;
	}
}

void PodofoView::mouseReleaseEvent(QMouseEvent * e)
{
	if ( isPanning )
	{
		isPanning = false;
		QApplication::restoreOverrideCursor();
		return;
	}
	if ( !isSelecting )
		return;
	if ( mouseStartPoint.toPoint() == mapToScene ( e->pos() ).toPoint() )
	{
		resetPDFMatrix();
		isSelecting = false;
		theRect->setRect ( QRectF() );
		return;
	}

	QRect zoomRect ( mouseStartPoint.toPoint(),mapToScene ( e->pos() ).toPoint() );
	ensureVisible ( zoomRect );
	isSelecting = false;
	fitInView ( theRect->sceneBoundingRect(), Qt::KeepAspectRatio );
	theRect->setRect ( QRectF() );

}

void PodofoView::mouseMoveEvent(QMouseEvent * e)
{
	if ( isPanning )
	{
		QPointF pos ( e->pos() );
		int vDelta ( qRound(mouseStartPoint.y() - pos.y()) );
		int hDelta ( qRound(mouseStartPoint.x() - pos.x()) );
		verticalScrollBar()->setValue ( verticalScrollBar()->value() + vDelta );
		horizontalScrollBar()->setValue ( horizontalScrollBar()->value() + hDelta );
		mouseStartPoint = pos;
		return;
	}
	if ( !isSelecting )
		return;

	QRectF r ( mouseStartPoint,mapToScene ( e->pos() ) );
	theRect->setRect ( r );
}

void PodofoView::wheelEvent(QWheelEvent * e)
{
	if ( e->modifiers().testFlag ( Qt::ControlModifier ) && e->orientation() == Qt::Vertical )
	{
		double d(  1.0 + ( static_cast<double>(e->delta()) / 1000.0 ) );
		QTransform trans;
		trans.scale ( d,d );
		setTransform(trans,true);
	}
	else
	{
		if ( e->orientation() == Qt::Vertical )
			verticalScrollBar()->setValue ( verticalScrollBar()->value() - e->delta() );
		if ( e->orientation() == Qt::Horizontal )
			horizontalScrollBar()->setValue ( horizontalScrollBar()->value() - e->delta() );
	}
}

void PodofoView::resizeEvent(QResizeEvent * event)
{
	// nothing :) all is quiet, the music is slow, the sun shines
}
