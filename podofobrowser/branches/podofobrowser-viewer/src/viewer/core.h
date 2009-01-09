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

#ifndef VIEWER_CORE_H
#define VIEWER_CORE_H

#include <podofo/podofo.h>
#include "graphicstate.h"

#include <QByteArray>
#include <QGraphicsView>
#include <QList>
#include <QMap>
#include <QRectF>

#include <QDebug>


class QGraphicsScene;
class QGraphicsItem;

struct  PdfOperation
{
	QString key;
	QList<PoDoFo::PdfVariant> operands;
	
	PdfOperation(const QString& k, const QList<PoDoFo::PdfVariant>& ops)
	:key(k), operands(ops)
	{}
};
typedef QList<PdfOperation> PdfContentStream;
typedef QList<PdfOperation>::iterator PdfContentIterator;


class GraphicItemMakerBase
{
	public:
		GraphicItemMakerBase();
		virtual ~GraphicItemMakerBase();
		
		virtual PdfContentIterator item(PdfContentIterator csIterator,  GraphicState& gState) = 0;
		virtual bool support(const QString& op) const {return false;}
		
	protected:
		bool checkVarCount(unsigned int control, const QList<PoDoFo::PdfVariant>& ops) const 
		{
			return ( control == ops.count() ) ? true : false;
		}
};


class NoOpMaker : public GraphicItemMakerBase
{
	public:
		NoOpMaker(){}
		PdfContentIterator item(PdfContentIterator csIterator,  GraphicState& gState)
		{
			return ++csIterator;
		}
		bool support(const QString& op) const {return true;}
};


#endif // VIEWER_CORE_H