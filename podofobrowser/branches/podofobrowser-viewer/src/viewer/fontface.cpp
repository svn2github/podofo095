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

#include "viewer/core.h"
#include "viewer/dispatcher.h"
#include "fontface.h"

#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QDebug>

FT_Library SimpleFTFace::ftlibrary = 0;

int SimpleFTFace::moveto ( const FT_Vector * to, void * user )
{
	SizedPath* sp = reinterpret_cast<SizedPath*> ( user );
	QPainterPath * p ( sp->p );
	double sf ( sp->s );
	p->moveTo ( to->x * sf , to->y * sf * -1.0 );
	return 0;
}

int SimpleFTFace::lineto ( const FT_Vector * to, void * user )
{
	SizedPath* sp = reinterpret_cast<SizedPath*> ( user );
	QPainterPath * p ( sp->p );
	double sf ( sp->s );
	p->lineTo ( to->x * sf, to->y  * sf * -1.0 );
	return  0;
}

int SimpleFTFace::conicto ( const FT_Vector * control, const FT_Vector * to, void * user )
{
	SizedPath* sp = reinterpret_cast<SizedPath*> ( user );
	QPainterPath * p ( sp->p );
	double sf ( sp->s );
	p->quadTo ( control->x * sf,control->y * sf * -1.0,to->x * sf,to->y * sf * -1.0 );
	return 0;
}

int SimpleFTFace::cubicto ( const FT_Vector * control1, const FT_Vector * control2, const FT_Vector * to, void * user )
{
	SizedPath* sp = reinterpret_cast<SizedPath*> ( user );
	QPainterPath * p ( sp->p );
	double sf ( sp->s );
	p->cubicTo ( control1->x * sf,control1->y * sf * -1.0,control2->x * sf,control2->y * sf * -1.0,to->x * sf,to->y  * sf * -1.0 );
	return 0;
}

SimpleFTFace::SimpleFTFace ( PoDoFo::PdfObject * pFont )
{
	enable();
	
	if ( pFont->IsDictionary() )
	{
		PoDoFo::PdfName subtype ( pFont->GetIndirectKey( PoDoFo::PdfName ( "Subtype" ) )->GetName() );
		if ( subtype == PoDoFo::PdfName ( "Type1" ) )
			makeType1 ( pFont );
		else if ( subtype == PoDoFo::PdfName ( "TrueType" ) )
			makeTrueType ( pFont );
		else
		{
			qWarning ( "Subtype not supported" );
			return;
		}
	}
}

SimpleFTFace::~ SimpleFTFace()
{
	
}

void SimpleFTFace::makeType1 ( PoDoFo::PdfObject * pFont )
{
	qDebug()<<"SimpleFTFace::makeType1";
	FT_Face face = 0;
	PoDoFo::PdfObject * fontDescriptor ( pFont->GetIndirectKey ( PoDoFo::PdfName ( "FontDescriptor" ) ) );
	if ( !fontDescriptor )
	{
		qWarning ( "No /FontDescriptor" );
		return;
	}
	PoDoFo::PdfObject * fontFile ( fontDescriptor->GetIndirectKey ( PoDoFo::PdfName ( "FontFile" ) ) );
	if ( !fontFile )
	{
		fontFile = fontDescriptor->GetIndirectKey ( PoDoFo::PdfName ( "FontFile3" ) ) ;
		if ( !fontFile )
		{
			qWarning ( "Font not embedded not supported yet" );
			return;
		}

	}

	PoDoFo::PdfMemoryOutputStream outMemStream ( 1 );
	try
	{
		fontFile->GetStream()->GetFilteredCopy ( &outMemStream );
	}
	catch ( PoDoFo::PdfError & e )
	{
		qDebug() <<"Arg, unable to get a filtered copy of a fontfile stream";
	}
	outMemStream.Close();

	FT_Error error = FT_New_Memory_Face ( ftlibrary, reinterpret_cast<const unsigned char*> ( outMemStream.TakeBuffer() ), outMemStream.GetLength(), 0, &face );
	if ( error )
	{
		qWarning ( "Unable to get a face out of the /FontFile" );
		return;
	}
	else
	{
		QMap<FT_Encoding, FT_CharMap> cmaps;
		for ( int u = 0; u < face->num_charmaps; u++ )
		{
			cmaps [ face->charmaps[u]->encoding ] = face->charmaps[u];
		}

		QMap<int, PoDoFo::PdfName> customMap;
                bool hasSpace(false);

		PoDoFo::PdfName WinAnsi ( "WinAnsiEncoding" );
		PoDoFo::PdfName MacRoman ( "MacRomanEncoding" );
		PoDoFo::PdfName MacExpert ( "MacExpertEncoding" );

		PoDoFo::PdfName encName ( "Encoding" );
		PoDoFo::PdfName diffName ( "Differences" );
		PoDoFo::PdfName baseName ( "BaseName" );

		if ( pFont->IsDictionary() )
		{
			PoDoFo::PdfObject *enc ( pFont->GetIndirectKey ( encName ) );
			if ( enc )
			{
// 				qDebug()<<"Set Encoding";
				if ( enc->IsName() )
				{
// 					qDebug()<<"\tName";
					PoDoFo::PdfName baseEnc ( enc->GetName() );
					if ( baseEnc == WinAnsi )
					{
						FT_Set_Charmap ( face, cmaps[FT_ENCODING_ADOBE_LATIN_1] ); // not sure at all
					}
					else if ( baseEnc == MacRoman )
					{
						FT_Set_Charmap ( face, cmaps[FT_ENCODING_APPLE_ROMAN] );
					}
					else if ( baseEnc == MacExpert )
					{
						FT_Set_Charmap ( face, cmaps[FT_ENCODING_ADOBE_EXPERT] );
					}
					else
					{
						FT_Set_Charmap ( face, cmaps[FT_ENCODING_ADOBE_STANDARD] );
					}
				}
				else
				{
// 					qDebug()<<"\tArray";
					PoDoFo::PdfName baseEnc;
					if(enc->GetDictionary().HasKey(baseName))
						baseEnc = enc->GetIndirectKey(baseName)->GetName();
					if ( baseEnc == WinAnsi )
					{
						FT_Set_Charmap ( face, cmaps[FT_ENCODING_ADOBE_LATIN_1] ); // not sure at all
					}
					else if ( baseEnc == MacRoman )
					{
						FT_Set_Charmap ( face, cmaps[FT_ENCODING_APPLE_ROMAN] );
					}
					else if ( baseEnc == MacExpert )
					{
						FT_Set_Charmap ( face, cmaps[FT_ENCODING_ADOBE_EXPERT] );
					}
					else
					{
						FT_Set_Charmap ( face, cmaps[FT_ENCODING_ADOBE_STANDARD] );
					}
// 					qDebug()<<"\tFace encoded";
					if ( enc->GetDictionary().HasKey ( diffName ) )
					{
						PoDoFo::PdfArray differences = enc->GetIndirectKey ( diffName )->GetArray();
						if ( differences.GetSize() > 0 )
						{
							int dSize ( differences.GetSize() );
							int curOfset ( 0 );
							PoDoFo::PdfObject * curObject = 0;
							for ( int i ( 0 ); i< dSize; ++i )
							{
								curObject = &differences[i];
								if ( curObject->IsName() )
								{
									customMap[curOfset] = curObject->GetName();
                                                                        if(curObject->GetName() == PoDoFo::PdfName("space"))
                                                                        {
                                                                            space = curOfset;
                                                                            hasSpace = true;
                                                                        }
// 									qDebug()<<"D"<<curOfset<<  curObject->GetName().GetEscapedName().c_str();
									++curOfset;
								}
								else if ( curObject->IsNumber() )
								{
									curOfset = curObject->GetNumber();
								}
							}
						}
						else
							qDebug() <<"Differences array is empty";
					}
					else
					{
						qDebug() <<"Encoding has not Differences array";
					}
				}

			}
			else
			{
				if ( !enc )
					qDebug() <<"Font has not encoding dict";
			}
		}
		else
			qDebug() <<"Font is not a dict";

		// first fill the encoding with face built-in data
		// then, override or complete with custom mappings.
		QMap<unsigned int,unsigned int > tmpMap;
		FT_UInt gIndex ( 1 );
		FT_UInt cc =  FT_Get_First_Char ( face, &gIndex );
		while ( gIndex )
		{
                        tmpMap[cc] = gIndex;
// 			qDebug()<<"*Encode"<<cc<<gIndex;
			cc = FT_Get_Next_Char ( face, cc, &gIndex );
		}

		foreach ( int cc, customMap.keys() )
		{
			QString charname ( QString::fromStdString ( customMap[cc].GetEscapedName() ) );
			gIndex = FT_Get_Name_Index ( face, charname.toAscii().data() );
			tmpMap[cc] = gIndex;
// 			qDebug()<<"+Encode"<<cc<<charname<<gIndex;
			if(!gIndex)
				qDebug()<<"Cannot get an index for"<<cc<<charname;
		}
		
		foreach(unsigned int cc, tmpMap.keys())
		{
			makePath(face, cc, tmpMap[cc]);
		}
                if(!hasSpace)
                    space = 32;
	}

	qDebug()<<"Face Done:"<<face->family_name << face->style_name<<cmap.count();
	FT_Done_Face (face);
}

void SimpleFTFace::makeTrueType(PoDoFo::PdfObject * pFont)
{
	qDebug()<<"TT not yet supported";
}

bool SimpleFTFace::enable()
{
	if ( !ftlibrary )
	{
		if ( FT_Init_FreeType ( &ftlibrary ) )
			return false;
	}
	return true;
}


void SimpleFTFace::makePath ( FT_Face face, unsigned int charcode, unsigned int gIndex )
{
	double scalefactor ( 1000.0 / face->units_per_EM );
	FT_Error ft_error = FT_Load_Glyph ( face,  gIndex , FT_LOAD_NO_SCALE );
	if ( ft_error )
	{
		qWarning ( "Unable to load glyph" );
	}

	QPainterPath glyphPath ( QPointF ( 0.0,0.0 ) );
	SizedPath sp;
	sp.p = &glyphPath;
	sp.s = scalefactor;

	FT_Outline_Funcs of = {moveto, lineto, conicto, cubicto, 0, 0};
	FT_Outline_Decompose ( & ( face->glyph->outline ), &of, &sp );

	glyphPath.closeSubpath();

	cmap[charcode] = glyphPath;
	widths[charcode] = face->glyph->metrics.horiAdvance * scalefactor;
// 	qDebug()<<"W"<<widths[charcode];
}

void SimpleFTFace::drawGlyph ( unsigned int charcode, GraphicState& gState )
{
//        qDebug()<<"SimpleFTFace::drawGlyph========"<<charcode<<"==================";
// 	gState.dump();
	if ( !cmap.contains ( charcode ) )
	{
		qDebug()<<"Charcode not encoded"<<QChar(charcode)<<charcode<<gState.text.Tf.GetName().c_str();
		return;
	}

        bool ws(charcode == space);
	QGraphicsPathItem *gi ( new QGraphicsPathItem );
	gi->setPath ( cmap[charcode] );
	if ( gState.text.Tmode == 0 )
	{
		gi->setPen ( Qt::NoPen );
		gi->setBrush ( gState.brush );
	}
	else if ( gState.text.Tmode == 1 )
	{
		gi->setPen ( gState.pen );
		gi->setBrush ( Qt::NoBrush );
	}
	else if ( gState.text.Tmode == 2 )
	{
		gi->setPen ( gState.pen );
		gi->setBrush ( gState.brush );
	}
	else if ( gState.text.Tmode == 3 )
	{
		gi->setPen ( Qt::NoPen );
		gi->setBrush ( Qt::NoBrush );
	}



        QMatrix gToT ( 1.0/1000.0, 0,0, -1.0/1000.0 , 0 ,0 );
	QMatrix m (	gState.text.Tfs * ( gState.text.Th / 100.0 ) , 0 ,
			0 ,  gState.text.Tfs,
                        -gState.text.Tj /1000.0, gState.text.Trise );
        QTransform t ( gToT * m * gState.text.Tm * gState.cm * gState.xm);
	gi->setZValue ( gState.getZIndex() );
	gState.scene->addItem ( gi );
	gState.itemList->append ( gi );
	gi->setTransform ( t );

        double tx( ( ( (widths[charcode]/1000.0)  -  (gState.text.Tj/1000.0)) * gState.text.Tfs + gState.text.Tc + (ws ? gState.text.Tw : 0.0) ) * (gState.text.Th/100.0));
        double ty(0);
        gState.text.Tm.translate ( tx , ty );
}

Type3Collection::Type3Collection ( PoDoFo::PdfObject * pFont, GraphicState& gState )
{
	// We assume pFont is valid and really represents a T3 font

	// fontmatrix first
	PoDoFo::PdfName fontmatrixName ( "FontMatrix" );
	if ( pFont->GetDictionary().HasKey ( fontmatrixName ) )
	{
		PoDoFo::PdfArray& fmArray ( pFont->GetIndirectKey ( fontmatrixName )->GetArray() );
		if ( fmArray.GetSize() == 6 )
		{
			m_fontMatrix = QMatrix ( fmArray[0].GetReal(),
					fmArray[1].GetReal(),
							fmArray[2].GetReal(),
									fmArray[3].GetReal(),
										fmArray[4].GetReal(),
										fmArray[5].GetReal() );
		}
	}

        // resources dictionary
        PoDoFo::PdfName resourcesName ( "Resources" );
        if ( pFont->GetDictionary().HasKey ( resourcesName ) )
        {
                m_resources =  pFont->GetDictionary().GetKey(resourcesName);
        }
	
	// encoding vector
	QMap<PoDoFo::PdfName, unsigned int> reverseCodes;
	PoDoFo::PdfName encName("Encoding");
	PoDoFo::PdfName difName("Differences");
	if ( pFont->GetDictionary().HasKey ( encName ) )
	{
		PoDoFo::PdfObject * encObj(pFont->GetIndirectKey (encName));
		if(encObj->IsDictionary() && encObj->GetDictionary().HasKey (difName))
		{
			PoDoFo::PdfArray& encArray(encObj->GetIndirectKey(difName)->GetArray());
			if ( encArray.GetSize() > 0 )
			{
				int dSize ( encArray.GetSize() );
				int curOfset ( 0 );
				PoDoFo::PdfObject * curObject = 0;
				for ( int i ( 0 ); i< dSize; ++i )
				{
					curObject = &encArray[i];
					if ( curObject->IsName() )
					{
						reverseCodes[curObject->GetName()] = curOfset;
                                                qDebug()<<"E"<<  QString::fromStdString( curObject->GetName().GetEscapedName() ) << curOfset;
						++curOfset;
					}
					else if ( curObject->IsNumber() )
					{
						curOfset = curObject->GetNumber();
					}
				}
			}
		}
	}
	
	
	// map of streams as found in charprocs
	PoDoFo::PdfName cpName("CharProcs");
	if ( pFont->GetDictionary().HasKey ( cpName ) )
	{
		PoDoFo::PdfObject * cpObj(pFont->GetIndirectKey(cpName));

		if(cpObj->IsDictionary())
		{
			qDebug()<<"Got a charprocs dictionary :)";
			PoDoFo::TKeyMap& cpMap(cpObj->GetDictionary().GetKeys());
			for(PoDoFo::TCIKeyMap cpIt(cpMap.begin());cpIt !=  cpMap.end(); ++cpIt)
			{
				PoDoFo::PdfObject * cs(cpIt->second);
				qDebug()<<"T"<< cs->GetDataTypeString();
				while(cs && cs->IsReference() )
				{
// 					if( cs->GetOwner() )
// 					{
						cs = gState.document->GetObjects().GetObject( cs->GetReference() );
// 						qDebug()<<"\t"<< cs->GetDataTypeString();
// 					}
// 					else
// 						qDebug()<<"Ooops - cs has no owner";
				}
				PoDoFo::PdfName cn(cpIt->first);
				PoDoFo::PdfMemoryOutputStream bufferStream ( 1 );
	
				if(cs->HasStream())
				{	
					const PoDoFo::PdfStream * const stream = cs->GetStream();
					try
					{
						stream->GetFilteredCopy ( &bufferStream );
					}
					catch ( PoDoFo::PdfError & e )
					{
						qDebug()<<"AAAAAaaaaaarrrrrrrrrrgh";
						return;
					}
				}
				else
					qDebug()<<"Ooops - cs has no stream";
			
				bufferStream.Close();
				m_collection[ reverseCodes[cn] ] = QByteArray ( bufferStream.TakeBuffer(), bufferStream.GetLength() );
			}
			
		}
		else
			qDebug()<<"Oops - charprocs vanished";
	}
	else
		qDebug()<<"Oops - no charprocs dictionary";

	// widths
	PoDoFo::PdfName wName("Widths");
	PoDoFo::PdfName fcName("FirstChar");
// 	PoDoFo::PdfName lcName("LastChar");
	int fc(0);
// 	int lc(0);
	if ( pFont->GetDictionary().HasKey ( fcName ) )
		fc = pFont->GetIndirectKey(fcName)->GetNumber();
// 	if ( pFont->GetDictionary().HasKey ( lcName ) )
// 		fc = pFont->GetIndirectKey(lcName)->GetNumber();
	if ( pFont->GetDictionary().HasKey(wName) )
	{
		PoDoFo::PdfObject * wObj(pFont->GetIndirectKey(wName));
		if(wObj && wObj->IsArray())
		{
			PoDoFo::PdfArray& wArray(wObj->GetArray());
			int dSize ( wArray.GetSize() );
			for ( int i ( 0 ); i< dSize; ++i )
			{
				m_widths[i + fc] = wArray[i].GetNumber();
			}
		}
	}
}

void Type3Collection::drawGlyph(unsigned int charcode, GraphicState & gState)
{
	qDebug()<<"Type3Collection::drawGlyph"<<charcode;
	if(!m_collection.contains(charcode))
		return;
	
	QByteArray& stream = m_collection[charcode];
	
//	qDebug()<<"*************************************";
//	qDebug()<<stream;
//	qDebug()<<"*************************************";
	PoDoFo::PdfContentsTokenizer * tokenizer = new PoDoFo::PdfContentsTokenizer ( stream.data(), stream.length() );

	const char*      pszToken = NULL;
	PoDoFo::PdfVariant       var;
	PoDoFo::EPdfContentsType eType;
	std::string      str;

	PdfContentStream contentStream;

	QList<PoDoFo::PdfVariant> stack;

	while ( tokenizer->ReadNext ( eType, pszToken, var ) )
	{
		if ( eType == PoDoFo::ePdfContentsType_Keyword )
		{
			contentStream << PdfOperation ( QString::fromAscii ( pszToken ), stack );
			stack.clear();
		}
		else if ( eType == PoDoFo::ePdfContentsType_Variant )
		{
			stack << var;
		}
		else
		{
			// Impossible; type must be keyword or variant
			qWarning ( "Impossible; type must be keyword or variant" );
		}
	}

        GraphicState nState ( gState );
        nState.cm = gState.cm * gState.text.Tm;
        nState.cm.scale(nState.text.Tfs / 1000.0, nState.text.Tfs / 1000.0);
        nState.resource = m_resources;

	delete tokenizer;
// 	qDebug() <<"Content Stream tokenized";
	

	PdfContentIterator it ( contentStream.begin() );
	Dispatcher *d(Dispatcher::getInstance());
	while ( it != contentStream.end() )
	{
		it = d->run(it, nState);
	}

}

