/***************************************************************************
 *   Copyright (C) 2006 by Dominik Seichter                                *
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

#include "PdfAnnotation.h"
#include "PdfAction.h"
#include "PdfArray.h"
#include "PdfDictionary.h"
#include "PdfDate.h"
#include "PdfPage.h"
#include "PdfRect.h"
#include "PdfVariant.h"
#include "PdfXObject.h"

namespace PoDoFo {

const long  PdfAnnotation::s_lNumActions = 26;
const char* PdfAnnotation::s_names[] = {
    "Text",                       // - supported
    "Link",
    "FreeText",       // PDF 1.3  // - supported
    "Line",           // PDF 1.3  // - supported
    "Square",         // PDF 1.3
    "Circle",         // PDF 1.3
    "Polygon",        // PDF 1.5
    "PolyLine",       // PDF 1.5
    "Highlight",      // PDF 1.3
    "Underline",      // PDF 1.3
    "Squiggly",       // PDF 1.4
    "StrikeOut",      // PDF 1.3
    "Stamp",          // PDF 1.3
    "Caret",          // PDF 1.5
    "Ink",            // PDF 1.3
    "Popup",          // PDF 1.3
    "FileAttachement",// PDF 1.3
    "Sound",          // PDF 1.2
    "Movie",          // PDF 1.2
    "Widget",         // PDF 1.2  // - supported
    "Screen",         // PDF 1.5
    "PrinterMark",    // PDF 1.4
    "TrapNet",        // PDF 1.3
    "Watermark",      // PDF 1.6
    "3D",             // PDF 1.6
    NULL
};

PdfAnnotation::PdfAnnotation( PdfPage* pPage, EPdfAnnotation eAnnot, const PdfRect & rRect, PdfVecObjects* pParent )
    : PdfElement( "Annot", pParent ), m_eAnnotation( eAnnot ), m_pAction( NULL )
{
    PdfVariant    rect;
    PdfDate       date;
    PdfString     sDate;
    const PdfName name( TypeNameForIndex( eAnnot, s_names, s_lNumActions ) );

    if( !name.GetLength() )
    {
        RAISE_ERROR( ePdfError_InvalidHandle );
    }

    rRect.ToVariant( rect );

    m_pObject->GetDictionary().AddKey( PdfName::KeyRect, rect );

    rRect.ToVariant( rect );
    date.ToString( sDate );
    
    m_pObject->GetDictionary().AddKey( PdfName::KeySubtype, name );
    m_pObject->GetDictionary().AddKey( PdfName::KeyRect, rect );
    m_pObject->GetDictionary().AddKey( "P", pPage->GetObject()->Reference() );
    m_pObject->GetDictionary().AddKey( "M", sDate );
}

PdfAnnotation::PdfAnnotation( PdfObject* pObject )
    : PdfElement( "Annot", pObject ), m_eAnnotation( ePdfAnnotation_Unknown ), m_pAction( NULL )
{
    m_eAnnotation = static_cast<EPdfAnnotation>(TypeNameToIndex( m_pObject->GetDictionary().GetKeyAsName( PdfName::KeySubtype ).GetName().c_str(), s_names, s_lNumActions ));
}

PdfAnnotation::~PdfAnnotation()
{
    delete m_pAction;
}

PdfRect PdfAnnotation::GetRect() const
{
   if( m_pObject->GetDictionary().HasKey( PdfName::KeyRect ) )
        return PdfRect( m_pObject->GetDictionary().GetKey( PdfName::KeyRect )->GetArray() );

   return PdfRect();
}

void PdfAnnotation::SetAppearanceStream( PdfXObject* pObject )
{
    PdfDictionary dict;

    if( !pObject )
    {
        RAISE_ERROR( ePdfError_InvalidHandle );
    }

    dict.AddKey( "N", pObject->GetObject()->Reference() );

    m_pObject->GetDictionary().AddKey( "AP", dict );
}

void PdfAnnotation::SetFlags( pdf_uint32 uiFlags )
{
    m_pObject->GetDictionary().AddKey( "F", PdfVariant( static_cast<long>(uiFlags) ) );
}

pdf_uint32 PdfAnnotation::GetFlags() const
{
    if( m_pObject->GetDictionary().HasKey( "F" ) )
        return static_cast<pdf_uint32>(m_pObject->GetDictionary().GetKey( "F" )->GetNumber());

    return static_cast<pdf_uint32>(0);
}

void PdfAnnotation::SetTitle( const PdfString & sTitle )
{
    m_pObject->GetDictionary().AddKey( "T", sTitle );
}

PdfString PdfAnnotation::GetTitle() const
{
    if( m_pObject->GetDictionary().HasKey( "T" ) )
        return m_pObject->GetDictionary().GetKey( "T" )->GetString();

    return PdfString();
}


void PdfAnnotation::SetContents( const PdfString & sContents )
{
    m_pObject->GetDictionary().AddKey( "Contents", sContents );
}

PdfString PdfAnnotation::GetContents() const
{
    if( m_pObject->GetDictionary().HasKey( "Contents" ) )
        return m_pObject->GetDictionary().GetKey( "Contents" )->GetString();

    return PdfString();
}

void PdfAnnotation::SetDestination( const PdfDestination & rDestination )
{
    rDestination.AddToDictionary( m_pObject->GetDictionary() );
}

PdfDestination PdfAnnotation::GetDestination() const
{
    return PdfDestination( m_pObject->GetDictionary().GetKey( "Dest" ) );
}

bool PdfAnnotation::HasDestination() const
{
    return m_pObject->GetDictionary().HasKey( "Dest" );
}

void PdfAnnotation::SetAction( const PdfAction & rAction )
{
    m_pAction = new PdfAction( rAction );
    m_pObject->GetDictionary().AddKey( "A", m_pAction->GetObject()->Reference() );
}

PdfAction* PdfAnnotation::GetAction() const
{
    if( !m_pAction && HasAction() )
        const_cast<PdfAnnotation*>(this)->m_pAction = new PdfAction( m_pObject->GetIndirectKey( "A" ) );

    return m_pAction;
}

bool PdfAnnotation::HasAction() const
{
    return m_pObject->GetDictionary().HasKey( "A" );
}

void PdfAnnotation::SetOpen( bool b )
{
    m_pObject->GetDictionary().AddKey( "Open", b );
}

bool PdfAnnotation::GetOpen() const
{
    if( m_pObject->GetDictionary().HasKey( "Open" ) )
        return m_pObject->GetDictionary().GetKey( "Open" )->GetBool();

    return false;
}

const char* PdfAnnotation::AnnotationKey( EPdfAnnotation eAnnot )
{
    const char* pszKey;

    switch( eAnnot ) 
    {
        case ePdfAnnotation_Text:
            pszKey = "Text"; break;
        case ePdfAnnotation_Link:
            pszKey = "Link"; break;
        case ePdfAnnotation_FreeText:
            pszKey = "FreeText"; break;
        case ePdfAnnotation_Popup:
            pszKey = "Popup"; break;
        case ePdfAnnotation_Widget:
            pszKey = "Widget"; break;
        case ePdfAnnotation_Unknown:
        default:
            pszKey = NULL; break;
    }

    return pszKey;
}

};
