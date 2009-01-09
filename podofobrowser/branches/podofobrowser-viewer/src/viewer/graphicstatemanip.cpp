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

#include "graphicstatemanip.h"

GraphicStateManip::GraphicStateManip()
{
	supportedOps << QString::fromAscii("w") 
			<< QString::fromAscii("j") 
			<< QString::fromAscii("J") 
			<< QString::fromAscii("M") 
			<< QString::fromAscii("d") 
			<< QString::fromAscii("ri") 
			<< QString::fromAscii("i") 
			<< QString::fromAscii("gs") ;
}

PdfContentIterator GraphicStateManip::item(PdfContentIterator csIterator, GraphicState & gState)
{
	if(csIterator->key == QString::fromAscii("w"))
	{
		gState.pen.setWidthF(csIterator->operands.first().GetReal());
	}
	else if(csIterator->key == QString::fromAscii("j"))
	{
		switch(csIterator->operands.first().GetNumber())
		{
			case 0 : gState.pen.setCapStyle(Qt::FlatCap) ;
			break;
			case 1 : gState.pen.setCapStyle(Qt::RoundCap) ;
			break;
			case 2 : gState.pen.setCapStyle(Qt::SquareCap) ;
			break;
			default : break;
		}
	}
	else if(csIterator->key == QString::fromAscii("J"))
	{
		switch(csIterator->operands.first().GetNumber())
		{
			case 0 : gState.pen.setJoinStyle(Qt::MiterJoin) ;
			break;
			case 1 : gState.pen.setJoinStyle(Qt::RoundJoin) ;
			break;
			case 2 : gState.pen.setJoinStyle(Qt::BevelJoin) ;
			break;
			default : break;
		}
	}
	else if(csIterator->key == QString::fromAscii("M"))
	{
		gState.pen.setMiterLimit(csIterator->operands.first().GetReal());
	}
	else if(csIterator->key == QString::fromAscii("d"))
	{
		// TODO dashline
	}
	else if(csIterator->key == QString::fromAscii("ri"))
	{
		// ### rendering intent
	}
	else if(csIterator->key == QString::fromAscii("i"))
	{
		// pffiu
	}
	else if(csIterator->key == QString::fromAscii("gs"))
	{
		loadGS(csIterator, gState);
	}
	
	return ++csIterator;
}

bool GraphicStateManip::support(const QString & op) const
{
	return supportedOps.contains(op);
}

void GraphicStateManip::loadGS(PdfContentIterator csIterator, GraphicState & gState)
{
	PoDoFo::PdfObject * gs(gState.getResource(csIterator->operands.first().GetName(), PoDoFo::PdfName("ExtGState")));
	if(!gs)
	{
		qDebug()<<"Error loading GraphicState";
		return;
	}
	while(gs->IsReference())
		gs = gState.document->GetObjects().GetObject(gs->GetReference());
	if(gs->IsDictionary())
	{
		PoDoFo::TKeyMap& gsMap(gs->GetDictionary().GetKeys());
		for(PoDoFo::TCIKeyMap gsIt(gsMap.begin());gsIt !=  gsMap.end(); ++gsIt)
		{
			PoDoFo::PdfName rname(gsIt->first);
			PoDoFo::PdfObject * rObj(gsIt->second);
			
			if(rname == PoDoFo::PdfName("LW"))
			{
				gState.pen.setWidthF(csIterator->operands.first().GetReal());
			}
			else if(rname == PoDoFo::PdfName("LC"))
			{
				switch(csIterator->operands.first().GetNumber())
				{
					case 0 : gState.pen.setCapStyle(Qt::FlatCap) ;
					break;
					case 1 : gState.pen.setCapStyle(Qt::RoundCap) ;
					break;
					case 2 : gState.pen.setCapStyle(Qt::SquareCap) ;
					break;
					default : break;
				}
			}
			else if(rname == PoDoFo::PdfName("LJ"))
			{
				switch(csIterator->operands.first().GetNumber())
				{
					case 0 : gState.pen.setJoinStyle(Qt::MiterJoin) ;
					break;
					case 1 : gState.pen.setJoinStyle(Qt::RoundJoin) ;
					break;
					case 2 : gState.pen.setJoinStyle(Qt::BevelJoin) ;
					break;
					default : break;
				}
			}
			else if(rname == PoDoFo::PdfName("ML"))
			{
				gState.pen.setMiterLimit(csIterator->operands.first().GetReal());
			}
			else if(rname == PoDoFo::PdfName("D"))
			{
			}
			else if(rname == PoDoFo::PdfName("RI"))
			{
			}
			else if(rname == PoDoFo::PdfName("OP"))
			{
			}
			else if(rname == PoDoFo::PdfName("op"))
			{
			}
			else if(rname == PoDoFo::PdfName("OPM"))
			{
			}
			else if(rname == PoDoFo::PdfName("Font"))
			{
			}
			else if(rname == PoDoFo::PdfName("BG"))
			{
			}
			else if(rname == PoDoFo::PdfName("BG2"))
			{
			}
			else if(rname == PoDoFo::PdfName("UCR"))
			{
			}
			else if(rname == PoDoFo::PdfName("UCR2"))
			{
			}
			else if(rname == PoDoFo::PdfName("TR"))
			{
			}
			else if(rname == PoDoFo::PdfName("TR2"))
			{
			}
			else if(rname == PoDoFo::PdfName("HT"))
			{
			}
			else if(rname == PoDoFo::PdfName("FL"))
			{
			}
			else if(rname == PoDoFo::PdfName("SM"))
			{
			}
			else if(rname == PoDoFo::PdfName("SA"))
			{
			}
			else if(rname == PoDoFo::PdfName("BM"))
			{
			}
			else if(rname == PoDoFo::PdfName("SMask"))
			{
			}
			else if(rname == PoDoFo::PdfName("CA"))
			{
			}
			else if(rname == PoDoFo::PdfName("ca"))
			{
			}
			else if(rname == PoDoFo::PdfName("AIS"))
			{
			}
			else if(rname == PoDoFo::PdfName("TK"))
			{
			}
		}
	}
	
}
