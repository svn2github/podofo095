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

#include "graphicstackmaker.h"

GraphicStackMaker::GraphicStackMaker()
{
	supportedOps << QString::fromAscii ( "q" )
			<< QString::fromAscii ( "Q" );
}

bool GraphicStackMaker::support(const QString & op) const
{
	return supportedOps.contains ( op );
}

PdfContentIterator GraphicStackMaker::item(PdfContentIterator csIterator, GraphicState & gState)
{
// 	qDebug()<<"GraphicStackMaker"<<csIterator->key;
	if(csIterator->key == QString::fromAscii("q"))
	{
		GraphicStateStack::getInstance()->stack( gState );
	}
	else if(csIterator->key == QString::fromAscii("Q"))
	{
		gState = GraphicStateStack::getInstance()->unstack();
	}
	return ++csIterator;
}

