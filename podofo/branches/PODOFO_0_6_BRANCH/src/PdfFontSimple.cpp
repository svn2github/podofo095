/***************************************************************************
 *   Copyright (C) 2007 by Dominik Seichter                                *
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

#include "PdfFontSimple.h"


#include "PdfArray.h"
#include "PdfDictionary.h"
#include "PdfFilter.h"
#include "PdfName.h"
#include "PdfStream.h"

namespace PoDoFo {


PdfFontSimple::PdfFontSimple( PdfFontMetrics* pMetrics, const PdfEncoding* const pEncoding, PdfVecObjects* pParent )
    : PdfFont( pMetrics, pEncoding, pParent )
{
}


void PdfFontSimple::Init( bool bEmbed, const PdfName & rsSubType )
{
    PdfObject*    pWidth;
    PdfObject*    pDescriptor;
    PdfVariant    var;
    PdfArray      array;

    pWidth = m_pObject->GetOwner()->CreateObject();
    if( !pWidth )
    {
        PODOFO_RAISE_ERROR( ePdfError_InvalidHandle );
    }

    m_pMetrics->GetWidthArray( *pWidth, m_pEncoding->GetFirstChar(), m_pEncoding->GetLastChar() );

    pDescriptor = m_pObject->GetOwner()->CreateObject( "FontDescriptor" );
    if( !pDescriptor )
    {
        PODOFO_RAISE_ERROR( ePdfError_InvalidHandle );
    }

    m_pObject->GetDictionary().AddKey( PdfName::KeySubtype, rsSubType );
    m_pObject->GetDictionary().AddKey("BaseFont", this->GetBaseFont() );
    m_pObject->GetDictionary().AddKey("FirstChar", PdfVariant( static_cast<long>(m_pEncoding->GetFirstChar()) ) );
    m_pObject->GetDictionary().AddKey("LastChar", PdfVariant( static_cast<long>(m_pEncoding->GetLastChar()) ) );
    m_pEncoding->AddToDictionary( m_pObject->GetDictionary() ); // Add encoding key

    m_pObject->GetDictionary().AddKey("Widths", pWidth->Reference() );
    m_pObject->GetDictionary().AddKey( "FontDescriptor", pDescriptor->Reference() );

    m_pMetrics->GetBoundingBox( array );

    pDescriptor->GetDictionary().AddKey( "FontName", this->GetBaseFont() );
    //pDescriptor->GetDictionary().AddKey( "FontWeight", (long)m_pMetrics->Weight() );
    pDescriptor->GetDictionary().AddKey( PdfName::KeyFlags, PdfVariant( 32L ) ); // TODO: 0 ????
    pDescriptor->GetDictionary().AddKey( "FontBBox", array );
    pDescriptor->GetDictionary().AddKey( "ItalicAngle", PdfVariant( static_cast<long>(m_pMetrics->GetItalicAngle()) ) );
    pDescriptor->GetDictionary().AddKey( "Ascent", m_pMetrics->GetPdfAscent() );
    pDescriptor->GetDictionary().AddKey( "Descent", m_pMetrics->GetPdfDescent() );
    pDescriptor->GetDictionary().AddKey( "CapHeight", m_pMetrics->GetPdfAscent() ); // m_pMetrics->CapHeight() );
    pDescriptor->GetDictionary().AddKey( "StemV", PdfVariant( 1L ) );               // m_pMetrics->StemV() );

    if( bEmbed )
        this->EmbedFont( pDescriptor );
}

};
