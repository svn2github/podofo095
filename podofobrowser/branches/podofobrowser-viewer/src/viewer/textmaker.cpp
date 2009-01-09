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

#include "dispatcher.h"
#include "textmaker.h"
#include "fontface.h"

TextMaker::TextMaker()
{
	supportedOps << QString::fromAscii ( "BT" )
	<< QString::fromAscii ( "ET" )
	<< QString::fromAscii ( "Tc" )
	<< QString::fromAscii ( "Tw" )
	<< QString::fromAscii ( "Tz" )
	<< QString::fromAscii ( "TL" )
	<< QString::fromAscii ( "Tf" )
	<< QString::fromAscii ( "Tr" )
	<< QString::fromAscii ( "Ts" )
	<< QString::fromAscii ( "Td" )
	<< QString::fromAscii ( "TD" )
	<< QString::fromAscii ( "Tm" )
	<< QString::fromAscii ( "T*" )
	<< QString::fromAscii ( "Tj" )
	<< QString::fromAscii ( "'" )
	<< QString::fromAscii ( "\"" )
	<< QString::fromAscii ( "TJ" );

	textShowOps << QString::fromAscii ( "Tj" )
	<< QString::fromAscii ( "'" )
	<< QString::fromAscii ( "\"" )
	<< QString::fromAscii ( "TJ" );

	textStateOps << QString::fromAscii ( "Tc" )
	<< QString::fromAscii ( "Tw" )
	<< QString::fromAscii ( "Tz" )
	<< QString::fromAscii ( "TL" )
	<< QString::fromAscii ( "Tf" )
	<< QString::fromAscii ( "Tr" )
	<< QString::fromAscii ( "Ts" )
	<< QString::fromAscii ( "Td" )
	<< QString::fromAscii ( "TD" )
	<< QString::fromAscii ( "Tm" )
	<< QString::fromAscii ( "T*" );
}

bool TextMaker::support ( const QString & op ) const
{
	return supportedOps.contains ( op );
}

PdfContentIterator TextMaker::item ( PdfContentIterator csIterator, GraphicState & gState )
{
	bool btBlock ( csIterator->key == QString::fromAscii ( "BT" ) );
	if ( btBlock )
	{
		PdfContentIterator it ( csIterator );
		while ( it->key != QString::fromAscii ( "ET" ) )
		{
			if ( support ( it->key ) )
			{
				if ( textStateOps.contains ( it->key ) )
				{
					manipTs ( it, gState );
				}
				else if ( textShowOps.contains ( it->key ) )
				{
					drawString ( it, gState );
				}
				++it;
			}
			else
				it = Dispatcher::getInstance()->run ( it,gState );

		}

		return ++it;
	}
	else if ( textStateOps.contains ( csIterator->key ) )
	{
		manipTs ( csIterator, gState );
		return ++csIterator;
	}

	return ++csIterator;
}

void TextMaker::manipTs ( PdfContentIterator csIterator, GraphicState& gState )
{
	if ( csIterator->key == QString::fromAscii ( "Tc" ) )
	{
		gState.text.Tc = csIterator->operands.first().GetReal();
	}
	else if ( csIterator->key == QString::fromAscii ( "Tw" ) )
	{
		gState.text.Tw = csIterator->operands.first().GetReal();
	}
	else if ( csIterator->key == QString::fromAscii ( "Tz" ) )
	{
		gState.text.Th = csIterator->operands.first().GetReal();
	}
	else if ( csIterator->key == QString::fromAscii ( "TL" ) )
	{
		gState.text.Tl = csIterator->operands.first().GetReal();
	}
	else if ( csIterator->key == QString::fromAscii ( "Tf" ) )
	{
		gState.text.Tf = csIterator->operands[0].GetName();
		gState.text.Tfs = csIterator->operands[1].GetReal();
	}
	else if ( csIterator->key == QString::fromAscii ( "Tr" ) )
	{
		gState.text.Tmode = csIterator->operands.first().GetNumber();
	}
	else if ( csIterator->key == QString::fromAscii ( "Ts" ) )
	{
		gState.text.Trise = csIterator->operands.first().GetReal();
	}
	else if ( csIterator->key == QString::fromAscii ( "Td" ) )
	{
		if ( checkVarCount ( 2,csIterator->operands ) )
		{
			double tx ( csIterator->operands[0].GetReal() );
			double ty ( csIterator->operands[1].GetReal() );

			QMatrix m ( gState.text.TLM );
			m.translate ( tx, ty );
			gState.text.Tm = gState.text.TLM = m;
		}
	}
	else if ( csIterator->key == QString::fromAscii ( "TD" ) )
	{
		if ( checkVarCount ( 2,csIterator->operands ) )
		{
			double tx ( csIterator->operands[0].GetReal() );
			double ty ( csIterator->operands[1].GetReal() );

			gState.text.Tl = -ty;

			QMatrix m ( gState.text.TLM );
			m.translate ( tx, ty );
			gState.text.Tm = gState.text.TLM = m;

		}
	}
	else if ( csIterator->key == QString::fromAscii ( "Tm" ) )
	{
		if ( checkVarCount ( 6,csIterator->operands ) )
		{
			double a ( csIterator->operands[0].GetReal() );
			double b ( csIterator->operands[1].GetReal() );
			double c ( csIterator->operands[2].GetReal() );
			double d ( csIterator->operands[3].GetReal() );
			double e ( csIterator->operands[4].GetReal() );
			double f ( csIterator->operands[5].GetReal() );

			QMatrix m ( a,b,c,d,e,f );
			gState.text.Tm = gState.text.TLM = m;
		}
	}
	else if ( csIterator->key == QString::fromAscii ( "T*" ) )
	{
		gState.text.dump();
		
		QMatrix m ( gState.text.TLM );
		// TODO explain why we need to minus gState.text.Tl when Reference says nothing like that
		m.translate ( 0, -gState.text.Tl );
		gState.text.Tm = gState.text.TLM = m;
	}

}

void TextMaker::drawString ( PdfContentIterator csIterator ,  GraphicState& gState )
{
// 	qDebug()<<"TextMaker::drawString";
	if ( ( !faces.contains ( gState.text.Tf ) ) )
	{
		if ( !makeFace ( csIterator, gState ) )

// 		qDebug()<<"DS - Unable to get the font"<<QString::fromStdString(gState.text.Tf.GetName());
			return;
	}
	if ( !faces[ gState.text.Tf] )
		return;

	FontFace* font ( faces[ gState.text.Tf] );
// 	qDebug() <<"F"<<font;
	if ( csIterator->key == QString::fromAscii ( "Tj" ) )
	{
		QString  s;
		try
		{
			s = QString::fromStdString ( csIterator->operands.first().GetString().GetStringUtf8() );
		}
		catch ( PoDoFo::PdfError & e )
		{
			qDebug() <<"Unable to get a string";
			return;
		}
		foreach ( QChar c, s )
		{
			font->drawGlyph ( c.unicode() , gState );
		}
	}
	else if ( csIterator->key == QString::fromAscii ( "'" ) )
	{
		QMatrix m ( gState.text.TLM );
		m.translate ( 0, gState.text.Tl );
		gState.text.Tm = gState.text.TLM = m;

		QString  s;
		try
		{
			s = QString::fromStdString ( csIterator->operands.first().GetString().GetStringUtf8() );
		}
		catch ( PoDoFo::PdfError & e )
		{
			qDebug() <<"Unable to get a string";
			return;
		}
		foreach ( QChar c, s )
		{
			font->drawGlyph ( c.unicode() , gState );
		}

	}
	else if ( csIterator->key == QString::fromAscii ( "\"" ) )
	{
		if ( checkVarCount ( 3,csIterator->operands ) )
		{
			double aw ( csIterator->operands[0].GetReal() );
			double ac ( csIterator->operands[1].GetReal() );
			gState.text.Tw = aw;
			gState.text.Tc = ac;

			QString  s;
			try
			{
				s = QString::fromStdString ( csIterator->operands[2].GetString().GetStringUtf8() );
			}
			catch ( PoDoFo::PdfError & e )
			{
				qDebug() <<"Unable to get a string";
				return;
			}
			foreach ( QChar c, s )
			{
				font->drawGlyph ( c.unicode() , gState );
			}
		}

	}
	else if ( csIterator->key == QString::fromAscii ( "TJ" ) )
	{
		PoDoFo::PdfArray  parray;
		try
		{
			parray =  csIterator->operands.first().GetArray() ;
		}
		catch ( PoDoFo::PdfError & e )
		{
			qDebug() <<"TJ operand is not an array";
		}
		const unsigned int psize ( parray.GetSize() );

		for ( int pi ( 0 ); pi < psize; ++pi )
		{
			PoDoFo::PdfObject& po ( parray[pi] );
			if ( po.IsReal() )
			{
				gState.text.Tj = po.GetReal();
			}
			else if ( po.IsString() )
			{
				QString s = QString::fromStdString ( po.GetString().GetStringUtf8() );
				foreach ( QChar c, s )
				{
					font->drawGlyph ( c.unicode() , gState );
				}
			}
		}
	}
}

bool TextMaker::makeFace ( PdfContentIterator csIterator, GraphicState & gState )
{
	qDebug() <<"makeFace for"<<QString::fromStdString ( gState.text.Tf.GetName() );

	PoDoFo::PdfFont* pCurFont;

	PoDoFo::PdfObject* pFont = gState.getResource ( gState.text.Tf, PoDoFo::PdfName ( "Font" ) );
	if ( !pFont )
	{
		qWarning ( "Unable to get font object" );
		faces[gState.text.Tf] = 0;
		return false;
	}

	if ( pFont->IsDictionary() )
	{
		PoDoFo::PdfName subtype ( pFont->GetDictionary().GetKeyAsName ( PoDoFo::PdfName ( "Subtype" ) ) );
		if ( subtype == PoDoFo::PdfName ( "Type3" ) )
			faces[gState.text.Tf] = new Type3Collection ( pFont, gState );
		else
			faces[gState.text.Tf] = new SimpleFTFace ( pFont );
	}
	return true;

}

void TextMaker::reset()
{
	foreach ( FontFace *f, faces.values() )
	{
		delete ( f );
	}
	faces.clear();
}
