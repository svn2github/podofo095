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

#include "PdfXObject.h" 

#include "PdfDictionary.h"
#include "PdfImage.h"
#include "PdfRect.h"
#include "PdfPage.h"
#include "PdfVariant.h"
#include "PdfDocument.h"
#include "PdfLocale.h"
#include "PdfDefinesPrivate.h"

#include <sstream>

using namespace std;

namespace PoDoFo {

PdfArray PdfXObject::s_matrix;

PdfXObject::PdfXObject( const PdfRect & rRect, PdfDocument* pParent, const char* pszPrefix )
    : PdfElement( "XObject", pParent ), PdfCanvas(), m_rRect( rRect )
{
    InitXObject( rRect, pszPrefix );
}

PdfXObject::PdfXObject( const PdfRect & rRect, PdfVecObjects* pParent, const char* pszPrefix )
    : PdfElement( "XObject", pParent ), PdfCanvas(), m_rRect( rRect )
{
    InitXObject( rRect, pszPrefix );
}

PdfXObject::PdfXObject( const PdfMemDocument & rDoc, int nPage, PdfDocument* pParent, const char* pszPrefix )
    : PdfElement( "XObject", pParent ), PdfCanvas()
{
    m_rRect = PdfRect();

    InitXObject( m_rRect, pszPrefix );

    // Implementation note: source document must be different from distination
    if ( pParent == reinterpret_cast<const PdfDocument*>(&rDoc) )
    {
        PODOFO_RAISE_ERROR( ePdfError_InternalLogic );
    }
    // After filling set correct BBox
    m_rRect = pParent->FillXObjectFromDocumentPage( this, rDoc, nPage );

    PdfVariant    var;
    m_rRect.ToVariant( var );
    this->GetObject()->GetDictionary().AddKey( "BBox", var );

    PdfArray      matrix;
    matrix.push_back( PdfVariant( static_cast<pdf_int64>(1LL) ) );
    matrix.push_back( PdfVariant( static_cast<pdf_int64>(0LL) ) );
    matrix.push_back( PdfVariant( static_cast<pdf_int64>(0LL) ) );
    matrix.push_back( PdfVariant( static_cast<pdf_int64>(1LL) ) );
	if( m_rRect.GetLeft() != 0 )
	    matrix.push_back( PdfVariant( m_rRect.GetLeft() * (-1.0) ) );
	else
	    matrix.push_back( PdfVariant( static_cast<pdf_int64>(0LL) ) );
	if( m_rRect.GetBottom() != 0 )
	    matrix.push_back( PdfVariant( m_rRect.GetBottom() * (-1.0) ) );
	else
	    matrix.push_back( PdfVariant( static_cast<pdf_int64>(0LL) ) );
    this->GetObject()->GetDictionary().AddKey( "Matrix", matrix );
}

PdfXObject::PdfXObject( PdfObject* pObject )
    : PdfElement( "XObject", pObject ), PdfCanvas()
{
    ostringstream out;
    PdfLocaleImbue(out);
    // Implementation note: the identifier is always
    // Prefix+ObjectNo. Prefix is /XOb for XObject.
    out << "XOb" << this->GetObject()->Reference().ObjectNumber();

    
    m_pResources = pObject->GetIndirectKey( "Resources" );
    m_Identifier = PdfName( out.str().c_str() );
    m_rRect      = PdfRect( this->GetObject()->GetIndirectKey( "BBox" )->GetArray() );
    m_Reference  = this->GetObject()->Reference();
}

void PdfXObject::InitXObject( const PdfRect & rRect, const char* pszPrefix )
{
    PdfVariant    var;
    ostringstream out;
    PdfLocaleImbue(out);

    // Initialize static data
    if( s_matrix.empty() )
    {
        // This matrix is the same for all PdfXObjects so cache it
        s_matrix.push_back( PdfVariant( static_cast<pdf_int64>(1LL) ) );
        s_matrix.push_back( PdfVariant( static_cast<pdf_int64>(0LL) ) );
        s_matrix.push_back( PdfVariant( static_cast<pdf_int64>(0LL) ) );
        s_matrix.push_back( PdfVariant( static_cast<pdf_int64>(1LL) ) );
        s_matrix.push_back( PdfVariant( static_cast<pdf_int64>(0LL) ) );
        s_matrix.push_back( PdfVariant( static_cast<pdf_int64>(0LL) ) );
    }

    rRect.ToVariant( var );
    this->GetObject()->GetDictionary().AddKey( "BBox", var );
    this->GetObject()->GetDictionary().AddKey( PdfName::KeySubtype, PdfName("Form") );
    this->GetObject()->GetDictionary().AddKey( "FormType", PdfVariant( static_cast<pdf_int64>(1LL) ) ); // only 1 is only defined in the specification.
    this->GetObject()->GetDictionary().AddKey( "Matrix", s_matrix );

    // The PDF specification suggests that we send all available PDF Procedure sets
    this->GetObject()->GetDictionary().AddKey( "Resources", PdfObject( PdfDictionary() ) );
    m_pResources = this->GetObject()->GetDictionary().GetKey( "Resources" );
    m_pResources->GetDictionary().AddKey( "ProcSet", PdfCanvas::GetProcSet() );

    // Implementation note: the identifier is always
    // Prefix+ObjectNo. Prefix is /XOb for XObject.
	if ( pszPrefix == NULL )
	    out << "XOb" << this->GetObject()->Reference().ObjectNumber();
	else
	    out << pszPrefix << this->GetObject()->Reference().ObjectNumber();

    m_Identifier = PdfName( out.str().c_str() );
    m_Reference  = this->GetObject()->Reference();
}

PdfXObject::PdfXObject( const char* pszSubType, PdfDocument* pParent, const char* pszPrefix )
    : PdfElement( "XObject", pParent ) 
{
    ostringstream out;
    PdfLocaleImbue(out);
    // Implementation note: the identifier is always
    // Prefix+ObjectNo. Prefix is /XOb for XObject.
	if ( pszPrefix == NULL )
	    out << "XOb" << this->GetObject()->Reference().ObjectNumber();
	else
	    out << pszPrefix << this->GetObject()->Reference().ObjectNumber();

    m_Identifier = PdfName( out.str().c_str() );
    m_Reference  = this->GetObject()->Reference();

    this->GetObject()->GetDictionary().AddKey( PdfName::KeySubtype, PdfName( pszSubType ) );
}

PdfXObject::PdfXObject( const char* pszSubType, PdfVecObjects* pParent, const char* pszPrefix )
    : PdfElement( "XObject", pParent ) 
{
    ostringstream out;
    PdfLocaleImbue(out);
    // Implementation note: the identifier is always
    // Prefix+ObjectNo. Prefix is /XOb for XObject.
	if ( pszPrefix == NULL )
	    out << "XOb" << this->GetObject()->Reference().ObjectNumber();
	else
	    out << pszPrefix << this->GetObject()->Reference().ObjectNumber();

    m_Identifier = PdfName( out.str().c_str() );
    m_Reference  = this->GetObject()->Reference();

    this->GetObject()->GetDictionary().AddKey( PdfName::KeySubtype, PdfName( pszSubType ) );
}

PdfXObject::PdfXObject( const char* pszSubType, PdfObject* pObject )
    : PdfElement( "XObject", pObject ) 
{
    ostringstream out;
    PdfLocaleImbue(out);

    if( this->GetObject()->GetDictionary().GetKeyAsName( PdfName::KeySubtype ) != pszSubType ) 
    {
        PODOFO_RAISE_ERROR( ePdfError_InvalidDataType );
    }

    // Implementation note: the identifier is always
    // Prefix+ObjectNo. Prefix is /XOb for XObject.
    out << "XOb" << this->GetObject()->Reference().ObjectNumber();

    m_Identifier = PdfName( out.str().c_str() );
    m_Reference  = this->GetObject()->Reference();
}

};
