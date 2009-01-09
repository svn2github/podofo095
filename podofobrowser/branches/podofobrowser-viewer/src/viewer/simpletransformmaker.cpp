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

#include "simpletransformmaker.h"

// simple ctm
SimpleTransformMaker::SimpleTransformMaker()
{
	supportedOps << QString::fromAscii ( "cm" );
}

bool SimpleTransformMaker::support ( const QString & op ) const
{
	return supportedOps.contains ( op );
}

PdfContentIterator SimpleTransformMaker::item ( PdfContentIterator csIterator, GraphicState & gState )
{
// 	qDebug()<<"SimpleTransformMaker"<<csIterator->key;
	if ( checkVarCount ( 6, csIterator->operands ) )
	{
		double a ( csIterator->operands[0].GetReal() );
		double b ( csIterator->operands[1].GetReal() );
		double c ( csIterator->operands[2].GetReal() );
		double d ( csIterator->operands[3].GetReal() );
		double e ( csIterator->operands[4].GetReal() );
		double f ( csIterator->operands[5].GetReal() );

		QMatrix m ( a,b,c,d,e,f );
		gState.cm = gState.cm * m;
	}
	return ++csIterator;
}
