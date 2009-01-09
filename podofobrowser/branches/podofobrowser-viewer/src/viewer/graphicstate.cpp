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

#include "graphicstate.h"

#include <QDebug>

double GraphicState::zIndexCount = 1.0;

PoDoFo::PdfObject * GraphicState::getResource(const PoDoFo::PdfName & resname, const PoDoFo::PdfName & dictname)
{
// 	qDebug()<<"GraphicState::getResource"<<QString::fromStdString( resname.GetEscapedName() ) << QString::fromStdString( dictname.GetEscapedName() );
	PoDoFo::PdfObject * ret(0);
	PoDoFo::PdfName rName("Resources");
	if(page && !xobject)
	{
// 		ret = page->GetFromResources(dictname, resname);
		if(!ret)
		{
			PoDoFo::PdfObject *rparent = page->GetObject();
			while ( rparent && rparent->IsDictionary() )
			{
				if( rparent->GetDictionary().HasKey( rName ) )
				{
					PoDoFo::PdfObject * resources(rparent->GetDictionary().GetKey(rName));
					if(resources) 
					{
						while(resources->IsReference())
							resources = document->GetObjects().GetObject( resources->GetReference() );
						if(resources->IsDictionary() && resources->GetDictionary().HasKey( dictname ) )
						{
							PoDoFo::PdfObject* pType = resources->GetDictionary().GetKey( dictname );
							while(pType->IsReference())
								pType = document->GetObjects().GetObject( pType->GetReference() );
							if( pType->IsDictionary() && pType->GetDictionary().HasKey( resname ) )
							{
								ret = pType->GetDictionary().GetKey( resname );
								if(ret)
								{
									while (ret->IsReference())
										ret = document->GetObjects().GetObject( ret->GetReference() );
									return ret;
								}
							}
						}
						
					}
				}
				rparent = rparent->GetIndirectKey ( "Parent" );
			}
		}
	}
	else if(xobject)
	{
// 		return xobject->GetFromResources(dictname, resname);
		// TODO extract resources for xobjects
	}
	
	return ret;
}

void GraphicState::Reset()
{
	QBrush defaultBrush(QColor(Qt::transparent), Qt::SolidPattern);
	QPen defaultPen(QColor(Qt::transparent));
	
	pen = defaultPen;
	brush = defaultBrush;
	
	zIndexCount = 0;
	
	text.Tc = 0.0;
	text.Tw = 0.0;
	text.Th = 100.0;
	text.Tl = 0.0;
	text.Tfs = 0.0;
	text.Tmode = 0;
	text.Trise = 0.0;
	text.Tj = 0.0;
}


