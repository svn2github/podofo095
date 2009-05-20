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

#include "viewer/dispatcher.h"

#include "viewer/graphicstackmaker.h"
#include "viewer/simplecolormaker.h"
#include "viewer/simpletransformmaker.h"
#include "viewer/pathitemmaker.h"
#include "viewer/textmaker.h"
#include "viewer/graphicstatemanip.h"
#include "viewer/xobjectmaker.h"


Dispatcher * Dispatcher::instance = 0;

Dispatcher::Dispatcher()
{
	// makers
	m_itemMakers [QString::fromUtf8("SimpleColorMaker")] = ( new SimpleColorMaker );
	m_itemMakers [QString::fromUtf8("PathItemMaker")] = ( new PathItemMaker );
	m_itemMakers [QString::fromUtf8("SimpleTransformMaker")] = ( new SimpleTransformMaker );
	m_itemMakers [QString::fromUtf8("GraphicStackMaker")] = ( new GraphicStackMaker );
	m_itemMakers [QString::fromUtf8("TextMaker")] = ( new TextMaker );
	m_itemMakers [QString::fromUtf8("GraphicStateManip")] = ( new GraphicStateManip );
        m_itemMakers [QString::fromUtf8("XObjectMaker")] = ( new XObjectMaker );
	// (...)
	// last one just passes, kind of "default" clause
// 	m_itemMakers [QString::fromUtf8("NoOpMaker")] = ( new NoOpMaker );
	NoOp = ( new NoOpMaker );
}

Dispatcher::~ Dispatcher()
{
	foreach(GraphicItemMakerBase* gim, m_itemMakers)
	{
		delete gim;
	}
}

Dispatcher * Dispatcher::getInstance()
{
	if(!instance)
	{
		instance = new Dispatcher;
		Q_ASSERT(instance);
	}
	return instance;
}

PdfContentIterator Dispatcher::run(PdfContentIterator csIterator, GraphicState & gState)
{	
	PdfContentIterator it(csIterator);
	foreach ( GraphicItemMakerBase *gim, m_itemMakers.values() )
	{
		if ( gim->support ( it->key ) )
		{
			return gim->item ( it, gState );
	
		}
	}
	return NoOp->item( it, gState );
}

