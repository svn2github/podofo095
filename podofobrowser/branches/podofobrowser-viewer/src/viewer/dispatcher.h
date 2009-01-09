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

#ifndef VIEWER_DISPATCHER_H
#define VIEWER_DISPATCHER_H

#include "viewer/core.h"

#include <QMap>

class Dispatcher
{
	Dispatcher();
	~Dispatcher();
	static Dispatcher * instance;
	
	
	QMap<QString, GraphicItemMakerBase*> m_itemMakers;
	GraphicItemMakerBase* NoOp;
	
	public:
		static Dispatcher * getInstance();
		
		PdfContentIterator run(PdfContentIterator csIterator,  GraphicState& gState);
		
		GraphicItemMakerBase* getMaker(const QString& mname){return m_itemMakers.contains(mname) ? m_itemMakers[mname] : 0;}
};

#endif
