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

#include "pathitemmaker.h"

#include <QGraphicsPathItem>

// Paths
PathItemMaker::PathItemMaker()
{
	supportedOps << QString::fromAscii ( "m" )
			<< QString::fromAscii ( "l" )
			<< QString::fromAscii ( "c" )
			<< QString::fromAscii ( "v" )
			<< QString::fromAscii ( "y" )
			<< QString::fromAscii ( "h" )
			<< QString::fromAscii ( "re" );
}

bool PathItemMaker::support ( const QString & op ) const
{
	return supportedOps.contains ( op );
}

PdfContentIterator PathItemMaker::item ( PdfContentIterator csIterator,  GraphicState & gState )
{
// 	qDebug()<<"PathItemMaker"<<csIterator->key;
	PdfContentIterator it ( csIterator );
	QPainterPath p;
	while ( 1 )
	{
		if ( support ( it->key ) )
		{
			if ( it->key == QString::fromAscii ( "m" ) )
			{
				if ( checkVarCount ( 2, it->operands ) )
				{
					double dPosX ( it->operands[0].GetReal() );
					double dPosY ( it->operands[1].GetReal() );
					p.moveTo ( QPointF ( dPosX, dPosY ) );
				}
				else
				{
					qDebug ( QString::fromAscii ( "Wrong number of operands (%1) for operator m " ).arg ( it->operands.count() ).toUtf8() );
				}
			}
			else if ( it->key == QString::fromAscii ( "l" ) )
			{
				if ( checkVarCount ( 2, it->operands ) )
				{
					double dPosX ( it->operands[0].GetReal() );
					double dPosY ( it->operands[1].GetReal() );
					p.lineTo ( QPointF ( dPosX, dPosY ) );
				}
				else
				{
					qDebug ( QString::fromAscii ( "Wrong number of operands (%1) for operator l " ).arg ( it->operands.count() ).toUtf8() );
				}
			}
			else if ( it->key == QString::fromAscii ( "c" ) )
			{
				if ( checkVarCount ( 6, it->operands ) )
				{
					double x1 ( it->operands[0].GetReal() );
					double y1 ( it->operands[1].GetReal() );
					double x2 ( it->operands[2].GetReal() );
					double y2 ( it->operands[3].GetReal() );
					double x3 ( it->operands[4].GetReal() );
					double y3 ( it->operands[5].GetReal() );

					p.cubicTo ( QPointF ( x1, y1 ) ,
							QPointF ( x2, y2 ) ,
									QPointF ( x3, y3 )  );
				}
				else
				{
					qDebug ( QString::fromAscii ( "Wrong number of operands (%1) for operator c " ).arg ( it->operands.count() ).toUtf8() );
				}
			}
			else if ( it->key == QString::fromAscii ( "v" ) )
			{
				if ( checkVarCount ( 4, it->operands ) )
				{
					double x1 ( p.currentPosition().x() );
					double y1 ( p.currentPosition().y() );
					double x2 ( it->operands[0].GetReal() );
					double y2 ( it->operands[1].GetReal() );
					double x3 ( it->operands[2].GetReal() );
					double y3 ( it->operands[3].GetReal() );

					p.cubicTo ( QPointF ( x1, y1 ) ,
							QPointF ( x2, y2 ) ,
									QPointF ( x3, y3 )  );
				}
				else
				{
					qDebug ( QString::fromAscii ( "Wrong number of operands (%1) for operator v " ).arg ( it->operands.count() ).toUtf8() );
				}
			}
			else if ( it->key == QString::fromAscii ( "y" ) )
			{
				if ( checkVarCount ( 4, it->operands ) )
				{
					double x1 ( it->operands[0].GetReal() );
					double y1 ( it->operands[1].GetReal() );
					double x2 ( it->operands[2].GetReal() );
					double y2 ( it->operands[3].GetReal() );
					double x3 ( it->operands[2].GetReal() );
					double y3 ( it->operands[3].GetReal() );

					p.cubicTo ( QPointF ( x1, y1 ) ,
							QPointF ( x2, y2 ) ,
									QPointF ( x3, y3 )  );
				}
				else
				{
					qDebug ( QString::fromAscii ( "Wrong number of operands (%1) for operator y " ).arg ( it->operands.count() ).toUtf8() );
				}
			}
			else if ( it->key == QString::fromAscii ( "h" ) )
			{
				p.closeSubpath();
			}
			else if ( it->key == QString::fromAscii ( "re" ) )
			{
				if ( checkVarCount ( 4, it->operands ) )
				{
					double x1 ( it->operands[0].GetReal() );
					double y1 ( it->operands[1].GetReal() );
					double width ( it->operands[2].GetReal() );
					double height ( it->operands[3].GetReal() );

					p.addRect ( x1, y1 , width, height );
				}
				else
				{
					qDebug ( QString::fromAscii ( "Wrong number of operands (%1) for operator re " ).arg ( it->operands.count() ).toUtf8() );
				}
			}
			else
			{
				qDebug ( QString::fromAscii ( "Unsupported Path operator :\"%1\"" ).arg ( it->key ).toUtf8() );
			}

			it++;
		}
		else // path must be finished!
		{
			gState.itemList->append ( gState.scene->addPath ( p * gState.cm, gState.pen, gState.brush ) );
			gState.itemList->last()->setZValue( gState.getZIndex() );
			break;
		}
	}

	return it;
}




