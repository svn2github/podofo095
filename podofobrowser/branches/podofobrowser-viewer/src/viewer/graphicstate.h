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

#ifndef VIEWER_GRAPHICSTATE_H
#define VIEWER_GRAPHICSTATE_H

#include <QBrush>
#include <QList>
#include <QMatrix>
#include <QPen>
#include <QDebug>

#include <podofo/podofo.h>

class QGraphicsScene;
class QGraphicsItem;


struct GraphicState
{
	GraphicState(){Reset();}
	GraphicState(const GraphicState& other)
	{
		pen = other.pen;
		brush = other.brush;
		cm = other.cm;
		text = other.text;
		scene = other.scene;
		itemList = other.itemList;
		document = other.document;
		page = other.page;
		xobject = other.xobject;
	}
	
	QPen pen;
	QBrush brush;
	QMatrix cm;	
	
	struct Text
	{
		Text(){}
		Text(const Text& other)
		{
			Tc = other.Tc;
			Tw = other.Tw;
			Th = other.Th;
			Tl = other.Tl;
			Tf = other.Tf;
			Tfs = other.Tfs;
			Tmode = other.Tmode;
			Trise = other.Trise;
			Tj = other.Tj;
			Tm = other.Tm;
			TLM = other.TLM;
			TRM = other.TRM;
		}
		double Tc;
		double Tw;
		double Th;
		double Tl;
		PoDoFo::PdfName Tf;
		double Tfs;
		int Tmode;
		double Trise;
		
		double Tj;
		
		QMatrix Tm;
		QMatrix TLM;
		QMatrix TRM;
		
		void dump(int tabs=0)
		{
			qDebug()<<QString(tabs, QChar(0x0009))<<"Tc"<<Tc;
			qDebug()<<QString(tabs, QChar(0x0009))<<"Tw"<<Tw;
			qDebug()<<QString(tabs, QChar(0x0009))<<"Th"<<Th;
			qDebug()<<QString(tabs, QChar(0x0009))<<"Tl"<<Tl;
			qDebug()<<QString(tabs, QChar(0x0009))<<"Tf"<<Tf.GetName().c_str();
			qDebug()<<QString(tabs, QChar(0x0009))<<"Tfs"<<Tfs;
			qDebug()<<QString(tabs, QChar(0x0009))<<"Tmode"<<Tmode;
			qDebug()<<QString(tabs, QChar(0x0009))<<"Trise"<<Trise;
			qDebug()<<QString(tabs, QChar(0x0009))<<"Tj"<<Tj;
			qDebug()<<QString(tabs, QChar(0x0009))<<"Tm"<<Tm;
			qDebug()<<QString(tabs, QChar(0x0009))<<"TLM"<<TLM;
			qDebug()<<QString(tabs, QChar(0x0009))<<"TRM"<<TRM;
		}
	};
	
	Text text;
	
	// Qt context
	QGraphicsScene * scene;
	QList<QGraphicsItem*> * itemList;
	
	// PDF context
	PoDoFo::PdfMemDocument * document;
	PoDoFo::PdfPage * page;
	PoDoFo::PdfXObject * xobject;
	PoDoFo::PdfObject * getResource(const PoDoFo::PdfName& resname, const PoDoFo::PdfName& dictname);
// 	PoDoFo::PdfVariant * getValue(const QString& key);
	
	void dump()
	{
		qDebug()<<"pen"<<pen;
		qDebug()<<"brush"<<brush;
		qDebug()<<"cm"<<cm;
		text.dump(1);
	}
	
	void Reset();
	
	double getZIndex()
	{
		zIndexCount += 1.0;
		return zIndexCount;
	}
		
	private:
		static double zIndexCount;
		
	
};


#endif //  VIEWER_GRAPHICSTATE_H