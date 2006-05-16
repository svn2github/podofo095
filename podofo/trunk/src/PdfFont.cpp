/***************************************************************************
 *   Copyright (C) 2005 by Dominik Seichter                                *
 *   domseichter@web.de                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "PdfFont.h"

#include "PdfFontMetrics.h"
#include "PdfPage.h"
#include "PdfStream.h"
#include "PdfWriter.h"

#include <sstream>

#define FIRST_CHAR   0
#define LAST_CHAR  255

using namespace std;

namespace PoDoFo {

PdfFont::PdfFont( PdfFontMetrics* pMetrics, PdfWriter* pWriter, unsigned int objectno, unsigned int generationno )
    : PdfObject( objectno, generationno, "Font" ), m_pWriter( pWriter ), m_pMetrics( pMetrics )
{
    ostringstream out;

    m_fFontSize   = 12.0;

    m_bBold       = false;
    m_bItalic     = false;
    m_bUnderlined = false;

    // Implementation note: the identifier is always
    // Prefix+ObjectNo. Prefix is /Ft for fonts.
    out << "Ft" << this->ObjectNumber();
    m_Identifier = PdfName( out.str().c_str() );
}
    
PdfFont::~PdfFont()
{
    delete m_pMetrics;
}

PdfError PdfFont::Init( bool bEmbedd )
{
    PdfError      eCode;

    unsigned int  i;
    int           curPos = 0;
    PdfObject*    pWidth;
    PdfObject*    pDescriptor;
    PdfVariant    var;
    std::string   sTmp;

    // replace all spaces in the base font name as suggested in 
    // the PDF reference section 5.5.2
    sTmp = m_pMetrics->Fontname();
    for(i = 0; i < sTmp.size(); i++)
    {
        if(sTmp[i] != ' ')
            sTmp[curPos++] = sTmp[i];
    }
    sTmp.resize(curPos);
    m_BaseFont = PdfName( sTmp.c_str() );

    SAFE_OP( m_pMetrics->GetWidthArray( var, FIRST_CHAR, LAST_CHAR ) );
    pWidth = m_pWriter->CreateObject();
    if( !pWidth )
    {
        RAISE_ERROR( ePdfError_InvalidHandle );
    }

    pWidth->SetSingleValue( var );

    pDescriptor = m_pWriter->CreateObject( "FontDescriptor" );
    if( !pDescriptor )
    {
        RAISE_ERROR( ePdfError_InvalidHandle );
    }

    this->AddKey( PdfName::KeySubtype, PdfName("TrueType") );
    this->AddKey("BaseFont", m_BaseFont );
    this->AddKey("FirstChar", (long)FIRST_CHAR );
    this->AddKey("LastChar", (long)LAST_CHAR );
    this->AddKey("Encoding", PdfName("WinAnsiEncoding") );
    this->AddKey("Widths", pWidth->Reference() );
    this->AddKey( "FontDescriptor", pDescriptor->Reference() );

    SAFE_OP( m_pMetrics->GetBoundingBox( sTmp ) );

    pDescriptor->AddKey( "FontName", m_BaseFont );
    //pDescriptor->AddKey( "FontWeight", (long)m_pMetrics->Weight() );
    pDescriptor->AddKey( PdfName::KeyFlags, (long)32 ); // TODO: 0 ????
    pDescriptor->AddKey( "FontBBox", sTmp );
    pDescriptor->AddKey( "ItalicAngle", (long)m_pMetrics->ItalicAngle() );
    pDescriptor->AddKey( "Ascent", m_pMetrics->Ascent() );
    pDescriptor->AddKey( "Descent", m_pMetrics->Descent() );
    pDescriptor->AddKey( "CapHeight", m_pMetrics->Ascent() ); // //m_pMetrics->CapHeight() );
    pDescriptor->AddKey( "StemV", (long)1 ); //m_pMetrics->StemV() );

    if( bEmbedd )
    {
        SAFE_OP( EmbeddFont( pDescriptor ) );
    }

    return eCode;
}

PdfError PdfFont::EmbeddFont( PdfObject* pDescriptor )
{
    PdfError   eCode;
    PdfObject* pContents;
    FILE*      hFile;
    char*      pBuffer;
    long       lSize;

    pContents = m_pWriter->CreateObject();
    if( !pContents )
    {
        RAISE_ERROR( ePdfError_InvalidHandle );
    }

    pDescriptor->AddKey( "FontFile2", pContents->Reference() );

    hFile = fopen( m_pMetrics->Filename(), "rb" );
    if( !hFile )
    {
        RAISE_ERROR( ePdfError_FileNotFound );
    }

    fseek( hFile, 0, SEEK_END );
    lSize = ftell( hFile );
    fseek( hFile, 0, SEEK_SET );

    pBuffer = (char*)malloc( sizeof(char) * lSize );
    fread( pBuffer, lSize, sizeof(char), hFile ); 

    fclose( hFile );
    
    pContents->AddKey( "Length1", lSize );
    pContents->Stream()->Set( pBuffer, lSize );

    return eCode;
}

void PdfFont::SetFontSize( float fSize )
{
    m_pMetrics->SetFontSize( fSize );

    m_fFontSize = fSize;
}


};

