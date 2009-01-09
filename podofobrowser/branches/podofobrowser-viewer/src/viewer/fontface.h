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

#ifndef FONTFACE_H
#define FONTFACE_H

#include "graphicstate.h"

#include <QMap>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

/**
	A base class for fonts.
*/

class FontFace
{
	public:
		FontFace(){};
		virtual ~FontFace(){};
		
		virtual void drawGlyph( unsigned int charcode, GraphicState& gState ) = 0;
		
};

class Type3Collection : public FontFace
{
	QMatrix m_fontMatrix; // FontMatrix
	QMap<unsigned int, QByteArray> m_collection; // mapped Charprocs
	QMap<unsigned int, double> m_widths; // Widths
	
	public:
		Type3Collection(PoDoFo::PdfObject * pFont, GraphicState& gState);
		~Type3Collection(){}
		void drawGlyph( unsigned int charcode, GraphicState& gState );
		
		
};

class SimpleFTFace : public FontFace
{
	struct SizedPath{
		QPainterPath* p;
		double s;
		double xadvance;
	};
	static int moveto(const FT_Vector*  to, void*   user);
	static int lineto(const FT_Vector*  to, void*   user);
	static int conicto(const FT_Vector* control, const FT_Vector*  to, void*   user);
	static int cubicto(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector*  to, void*   user);
	
	QMap<unsigned int, QPainterPath> cmap; // charcode, outlines
	QMap<unsigned int, double> widths;// xadvances
	
	void makeType1(PoDoFo::PdfObject * pFont);
	void makeTrueType(PoDoFo::PdfObject * pFont);
	void makePath(FT_Face face, unsigned int charcode, unsigned int gIndex);
	
	protected:
		static FT_Library ftlibrary;
		bool enable();
	public:
		SimpleFTFace(PoDoFo::PdfObject * pFont);
		~SimpleFTFace();
		void drawGlyph( unsigned int charcode, GraphicState& gState );
		
};

#endif // FONTFACE_H
