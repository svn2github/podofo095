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

#include "PdfPage.h" 
#include "PdfDictionary.h"
#include "PdfDocument.h"
#include "PdfRect.h"
#include "PdfVariant.h"
#include "PdfWriter.h"


namespace PoDoFo {

PdfPage::PdfPage( const PdfRect & rSize, PdfVecObjects* pParent )
    : PdfElement( "Page", pParent ), PdfCanvas()
{
    PdfVariant mediabox;
    rSize.ToVariant( mediabox );
    m_pObject->GetDictionary().AddKey( "MediaBox", mediabox );

    // The PDF specification suggests that we send all available PDF Procedure sets
    m_pObject->GetDictionary().AddKey( "Resources", PdfObject( PdfDictionary() ) );

    m_pResources = m_pObject->GetIndirectKey( "Resources" );
    m_pResources->GetDictionary().AddKey( "ProcSet", PdfCanvas::GetProcSet() );

    m_pContents = new PdfContents( pParent );
    m_pObject->GetDictionary().AddKey( PdfName::KeyContents, m_pContents->GetContents()->Reference());
}

PdfPage::PdfPage( PdfObject* pObject )
    : PdfElement( "Page", pObject ), PdfCanvas()
{
    m_pResources = m_pObject->GetIndirectKey( "Resources" );
    m_pContents = new PdfContents( m_pObject->GetIndirectKey( "Contents" ) );
}

PdfPage::~PdfPage()
{
    TIMapAnnotation it = m_vecAnnotations.begin();

    while( it != m_vecAnnotations.end() )
    {
        delete (*it).second;

        ++it;
    }

    delete m_pContents;	// just clears the C++ object from memory, NOT the PdfObject
}

PdfRect PdfPage::CreateStandardPageSize( const EPdfPageSize ePageSize )
{
    PdfRect rect;

    switch( ePageSize ) 
    {
        case ePdfPageSize_A4:
            rect.SetWidth( 595.0 );
            rect.SetHeight( 842.0 );
            break;

        case ePdfPageSize_Letter:
            rect.SetWidth( 612.0 );
            rect.SetHeight( 792.0 );
            break;
            
        case ePdfPageSize_Legal:
            rect.SetWidth( 612.0 );
            rect.SetHeight( 1008.0 );
            break;
            
        case ePdfPageSize_A3:
            rect.SetWidth( 842.0 );
            rect.SetHeight( 1190.0 );
            break;
            
        default:
            break;
    }

    return rect;
}

PdfObject* PdfPage::GetInheritedKeyFromObject( const char* inKey, PdfObject* inObject ) const
{
    PdfObject* pObj = NULL;

    // check for it in the object itself
    if ( inObject->GetDictionary().HasKey( inKey ) ) 
    {
        pObj = inObject->GetDictionary().GetKey( inKey );
        if ( !pObj->IsNull() ) 
            return pObj;
    }
    
    // if we get here, we need to go check the parent - if there is one!
    if( inObject->GetDictionary().HasKey( "Parent" ) ) 
    {
        pObj = inObject->GetIndirectKey( "Parent" );
        if( pObj )
            pObj = GetInheritedKeyFromObject( inKey, pObj );
    }

    return pObj;
}

const PdfRect PdfPage::GetPageBox( const char* inBox ) const
{
    PdfRect	 pageBox;
    PdfObject*   pObj;
        
    // Take advantage of inherited values - walking up the tree if necessary
    pObj = GetInheritedKeyFromObject( inBox, m_pObject );
    
    // assign the value of the box from the array
    if ( pObj && pObj->IsArray() )
        pageBox.FromArray( pObj->GetArray() );
    
    return pageBox;
}

const int PdfPage::GetRotation() const 
{ 
    int rot = 0;
    
    PdfObject* pObj = GetInheritedKeyFromObject( "Rotate", m_pObject ); 
    if ( pObj && pObj->IsNumber() )
        rot = pObj->GetNumber();
    
    return rot;
}

PdfObject* PdfPage::GetAnnotationsArray( bool bCreate ) const
{
    PdfObject* pObj;

    // check for it in the object itself
    if ( m_pObject->GetDictionary().HasKey( "Annots" ) ) 
    {
        pObj = m_pObject->GetIndirectKey( "Annots" );
        if( pObj && pObj->IsArray() )
            return pObj;
    }
    else if( bCreate ) 
    {
        PdfArray array;
        const_cast<PdfPage*>(this)->m_pObject->GetDictionary().AddKey( "Annots", array );
        return m_pObject->GetDictionary().GetKey( "Annots" );
    }

    return NULL;
}

const int PdfPage::GetNumAnnots() const
{
    PdfObject* pObj = this->GetAnnotationsArray();

    return pObj ? static_cast<int>(pObj->GetArray().size()) : 0;
}

PdfAnnotation* PdfPage::CreateAnnotation( EPdfAnnotation eType, const PdfRect & rRect )
{
    PdfAnnotation* pAnnot = new PdfAnnotation( this, eType, rRect, m_pObject->GetParent() );
    PdfObject*     pObj   = this->GetAnnotationsArray( true );
    PdfReference   ref    = pAnnot->GetObject()->Reference();

    pObj->GetArray().push_back( ref );
    m_vecAnnotations[ref] = pAnnot;

    return pAnnot;
}

PdfAnnotation* PdfPage::GetAnnotation( int index )
{
    PdfObject*     pObj;
    PdfAnnotation* pAnnot;
    PdfReference   ref;

    // check for it in the object itself
    if ( !m_pObject->GetDictionary().HasKey( "Annots" ) ) 
    {
        RAISE_ERROR( ePdfError_InvalidKey );
    }
    
    pObj = m_pObject->GetIndirectKey( "Annots" );
    if( !pObj || !pObj->IsArray() )
    {
        RAISE_ERROR( ePdfError_InvalidDataType );
    }
    
    if( index < 0 && index >= pObj->GetArray().size() )
    {
        RAISE_ERROR( ePdfError_ValueOutOfRange );
    }

    ref    = pObj->GetArray()[index].GetReference();
    pAnnot = m_vecAnnotations[ref];
    if( !pAnnot )
    {
        pObj = m_pObject->GetParent()->GetObject( ref );
        if( !pObj )
        {
            RAISE_ERROR( ePdfError_NoObject );
        }
     
        pAnnot = new PdfAnnotation( pObj );
        m_vecAnnotations[ref] = pAnnot;
    }

    return pAnnot;
}

void PdfPage::DeleteAnnotation( int index )
{

}

void PdfPage::DeleteAnnotation( const PdfReference & ref )
{

}

};
