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

#ifndef TEXTMAKER_H
#define TEXTMAKER_H

#include "viewer/core.h"

class FontFace;

class TextMaker : public GraphicItemMakerBase
{
	QStringList supportedOps;
	QStringList textShowOps;
	QStringList textStateOps;
	
	void manipTs(PdfContentIterator csIterator, GraphicState& gState);
	
	bool makeFace(PdfContentIterator csIterator,  GraphicState& gState);
	QMap<PoDoFo::PdfName , FontFace* > faces;

	void drawString(PdfContentIterator csIterator,  GraphicState& gState);
	
	public:
		TextMaker();
		PdfContentIterator item(PdfContentIterator csIterator,  GraphicState& gState);
		bool support(const QString& op) const ;
		
		void reset();
		

};


#endif // TEXTMAKER_H
