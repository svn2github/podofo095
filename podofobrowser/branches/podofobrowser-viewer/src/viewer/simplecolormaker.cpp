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

#include "simplecolormaker.h"

// Simple colors

SimpleColorMaker::SimpleColorMaker()
{
	supportedOps << QString::fromAscii ( "g" )
			<< QString::fromAscii ( "G" )
			<< QString::fromAscii ( "rg" )
			<< QString::fromAscii ( "RG" )
			<< QString::fromAscii ( "k" )
			<< QString::fromAscii ( "K" );
}

bool SimpleColorMaker::support ( const QString & op ) const
{
	return supportedOps.contains ( op );
}

PdfContentIterator SimpleColorMaker::item ( PdfContentIterator csIterator,  GraphicState & gState )
{
// 	qDebug()<<"SimpleColorMaker"<<csIterator->key;
	QColor color;
	if ( ( csIterator->key == QString::fromAscii ( "G" ) ) )
	{
		if ( checkVarCount ( 1, csIterator->operands ) )
		{
			double g ( csIterator->operands[0].GetReal() );
			color.setRgbF ( g,g,g,1.0 );
			gState.pen.setColor ( color );
		}
	}
	else if ( ( csIterator->key == QString::fromAscii ( "g" ) ) )
	{
		if ( checkVarCount ( 1, csIterator->operands ) )
		{
			double g ( csIterator->operands[0].GetReal() );
			color.setRgbF ( g,g,g,1.0 );
			gState.brush.setColor ( color );
		}
	}
	else if ( ( csIterator->key == QString::fromAscii ( "RG" ) ) )
	{
		if ( checkVarCount ( 3, csIterator->operands ) )
		{
			double r ( csIterator->operands[0].GetReal() );
			double g ( csIterator->operands[1].GetReal() );
			double b ( csIterator->operands[2].GetReal() );
			color.setRgbF ( r,g,b,1.0 );
			gState.pen.setColor ( color );
		}
	}
	else if ( ( csIterator->key == QString::fromAscii ( "rg" ) ) )
	{
		if ( checkVarCount ( 3, csIterator->operands ) )
		{
			double r ( csIterator->operands[0].GetReal() );
			double g ( csIterator->operands[1].GetReal() );
			double b ( csIterator->operands[2].GetReal() );
			color.setRgbF ( r,g,b,1.0 );
			gState.brush.setColor ( color );
		}
	}
	else if ( ( csIterator->key == QString::fromAscii ( "K" ) ) )
	{
		if ( checkVarCount ( 4, csIterator->operands ) )
		{
			double c ( csIterator->operands[0].GetReal() );
			double m ( csIterator->operands[1].GetReal() );
			double y ( csIterator->operands[2].GetReal() );
			double k ( csIterator->operands[3].GetReal() );
			color.setCmykF ( c,m,y,k,1.0 );
			gState.pen.setColor ( color );
		}
	}
	else if ( ( csIterator->key == QString::fromAscii ( "k" ) ) )
	{
		if ( checkVarCount ( 4, csIterator->operands ) )
		{
			double c ( csIterator->operands[0].GetReal() );
			double m ( csIterator->operands[1].GetReal() );
			double y ( csIterator->operands[2].GetReal() );
			double k ( csIterator->operands[3].GetReal() );
			color.setCmykF ( c,m,y,k,1.0 );
			gState.brush.setColor ( color );
		}
	}

	return ++csIterator;
}



