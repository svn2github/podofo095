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

#include "PdfPagesTree.h"

#include "PdfArray.h"
#include "PdfDictionary.h"
#include "PdfObject.h"
#include "PdfOutputDevice.h"
#include "PdfPage.h"
#include "PdfVecObjects.h"

#include <iostream>
namespace PoDoFo {

PdfPagesTree::PdfPagesTree( PdfVecObjects* pParent )
    : PdfElement( "Pages", pParent )
{
    // PdfObject* kids = pParent->CreateObject( PdfArray() );
    Object()->GetDictionary().AddKey( "Kids", PdfArray() ); // kids->Reference() 
    Object()->GetDictionary().AddKey( "Count", PdfObject( (long) 0 ) );
}

PdfPagesTree::PdfPagesTree( PdfObject* pPagesRoot )
    : PdfElement( "Pages", pPagesRoot )
{
    if( !m_pObject ) 
    {
        RAISE_ERROR( ePdfError_InvalidHandle );
    }

    // pre-allocate enough elements
    // better performance and allows for a "sparse array"
    m_deqPageObjs.resize( GetTotalNumberOfPages() );
}

PdfPagesTree::~PdfPagesTree() 
{
    PdfPageObjects::iterator it = m_deqPageObjs.begin();

    while( it != m_deqPageObjs.end() )
    {
        delete (*it);
        ++it;
    }
        
    m_deqPageObjs.clear();
}

int PdfPagesTree::GetTotalNumberOfPages() const
{
    return ( ( m_pObject->GetDictionary().HasKey( "Count" ) ) ?
				m_pObject->GetDictionary().GetKeyAsLong( "Count", 0 ) : 0 );
}

PdfObject* PdfPagesTree::GetPageFromKidArray( PdfArray& inArray, int inIndex )
{
    PdfVariant & kidsVar = inArray[ inIndex ];

#ifdef _DEBUG
    std::string str;
    kidsVar.ToString( str );
#endif

    // is the kid a Pages tree node or a Page object?
    if ( !kidsVar.IsReference() )  
    {
        RAISE_ERROR( ePdfError_InvalidDataType );
        //return NULL;	// can't handle inline pages just yet...
    }
    
    return GetRoot()->GetParent()->GetObject( kidsVar.GetReference() );
}

PdfObject* PdfPagesTree::GetPageNode( int nPageNum, PdfObject* pPagesObject )
{
    // recurse through the pages tree nodes
    int        nPagesSeenSoFar = -1;	// initialize to -1 because nPageNum is 0-based
    PdfObject* pObj            = NULL;

    if( !pPagesObject->GetDictionary().HasKey( "Kids" ) )
        return NULL;

    pObj = pPagesObject->GetDictionary().GetKey( "Kids" );
    if( !pObj->IsArray() )
        return NULL;

    PdfArray&	kidsArray = pObj->GetArray();
    size_t	numKids   = kidsArray.size();
    size_t      kidsCount = pPagesObject->GetDictionary().GetKeyAsLong( "Count", 0 );

    // the pages tree node represented by pPagesObject has only page nodes in its kids array,
    // or pages nodes with a kid count of 1, so we can speed things up by going straight to the desired node
    if ( numKids == kidsCount )
    {
        if( nPageNum >= kidsArray.size() )
        {
            PdfError::LogMessage( eLogSeverity_Critical, "Requesting page index %i from array of size %i\n", nPageNum, kidsArray.size() );
            /*
            RAISE_ERROR( ePdfError_ValueOutOfRange );
            */
            nPageNum--;
        }

        PdfVariant pgVar = kidsArray[ nPageNum ];
        while ( true ) 
        {
            if ( !pgVar.IsReference() ) 
                return NULL;	// can't handle inline pages just yet...

            PdfObject* pgObject = GetRoot()->GetParent()->GetObject( pgVar.GetReference() );
            // make sure the object is a /Page and not a /Pages with a single kid
            if ( pgObject->GetDictionary().GetKeyAsName( PdfName( "Type" ) ) == PdfName( "Page" ) )
                return pgObject;

            // it's a /Pages with a single kid, so dereference and try again...
            if( !pgObject->GetDictionary().HasKey( "Kids" ) )
                return NULL;

            pgVar = *(pgObject->GetDictionary().GetKey( "Kids" ));
        }
    } 
    else 
    {
        for( unsigned int i = 0 ; i < numKids ; i++ )
        {
#if 0
            PdfVariant	kidsVar = kidsArray[ i ];
            
            // is the kid a Pages tree node or a Page object?
            if ( !kidsVar.IsReference() ) 
            {
                return NULL;	// can't handle inline pages just yet...
            }

            PdfObject* pgObject = GetRoot()->GetParent()->GetObject( kidsVar.GetReference() );
#else
            PdfObject* pgObject = GetPageFromKidArray( kidsArray, i );
#endif
            
            // if it's a Page, then is it the right page??
            // otherwise, it's a Pages, and we need to recurse
            if ( pgObject->GetDictionary().GetKeyAsName( PdfName( "Type" ) ) == PdfName( "Page" ) )
            {
                nPagesSeenSoFar++;
                if( nPagesSeenSoFar == nPageNum )
                {
                    return pgObject;
                }
            }
            else 
            {
                int thisKidCount = pgObject->GetDictionary().GetKeyAsLong( "Count", 0 );
                if( ( nPagesSeenSoFar + thisKidCount ) >= nPageNum )
                    return this->GetPageNode( nPageNum - ( nPagesSeenSoFar + 1 ), pgObject ) ;
                else
                    nPagesSeenSoFar += thisKidCount ;
            }
        }
    }

    // we should never exit from here - we should always have been able to return a page from above
    // assert( false ) ;
    return NULL;
}

PdfPage* PdfPagesTree::GetPage( int nIndex )
{
    PdfObject* pObj;
    PdfPage*   pPage;

    // if you try to get a page past the end, return NULL
    // we use >= since nIndex is 0 based
    if ( nIndex >= GetTotalNumberOfPages() )
        return NULL;
    
    // if we already have the page in our list, return it
    // otherwise, we need to find it, add it and return it
    pPage = m_deqPageObjs[ nIndex ];
    if ( !pPage ) 
    {
        pObj = GetPageNode( nIndex, GetRoot() );
        if ( pObj )
        {
            pPage = new PdfPage( pObj );
            m_deqPageObjs[ nIndex ] = pPage;
        }
    }
    
    return pPage;
}

PdfObject* PdfPagesTree::GetParent( PdfObject* inObject )
{
    PdfObject *pObj = inObject->GetIndirectKey( "Parent" );
    if( pObj && pObj->IsDictionary() )
        return pObj;
    else
        return NULL;
}

PdfObject* PdfPagesTree::GetKids( PdfObject* inPagesDict )
{
    PdfObject *pObj = inPagesDict->GetIndirectKey( "Kids" );
    if( pObj && pObj->IsArray() )
        return pObj;
    else
        return NULL;
}

int PdfPagesTree::GetPosInKids( PdfObject* inPageObj )
{
    // given a page or pages dictionary, return the index into its parents Kids array:
    PdfObject* parentObj = PdfPagesTree::GetParent( inPageObj ) ;
    
    // if inPageDict has no Parent, return -1; this would be the case when inserting a new page
    if( parentObj == NULL )
        return -1;
    
    // find inPageObj in parentObj
    PdfObject* theKids = PdfPagesTree::GetKids( parentObj ) ;
    PdfArray&	kidsArray = theKids->GetArray();
    size_t kidsLen = kidsArray.size();
    int kidsIndex;
    bool foundKid = false ;
    for( kidsIndex = 0 ; ( !foundKid ) && ( kidsIndex < kidsLen ) ; kidsIndex++ )
    {
        PdfObject* kidObj = GetPageFromKidArray( kidsArray, kidsIndex );
        if( inPageObj == kidObj || *kidObj == *inPageObj ) {
            foundKid = true ;
            break ;
        }
    }
    
    return (int)kidsIndex ;
}

void PdfPagesTree::InsertPage( int inAfterPageNumber, PdfPage* inPage )
{
    this->InsertPage( inAfterPageNumber, inPage->Object() );
}

void PdfPagesTree::InsertPage( int inAfterPageNumber, PdfObject* pPage )
{
    PdfObject*	parentObj    = GetRoot();
    PdfObject*	afterPageObj = ( PdfObject* )NULL;

    if( PageInsertBeforeFirstPage != inAfterPageNumber )
    {
        // get the page dictionary that we want to insert after, and get its parent pages dictionary
        afterPageObj = GetPageNode( inAfterPageNumber, GetRoot() ) ;
        if( !afterPageObj )
        {
            RAISE_ERROR( ePdfError_InvalidHandle );
        }
     
        parentObj = PdfPagesTree::GetParent( afterPageObj );
    }
    
    // find afterPageObj's position in its parent's Kids array
    int kidsIndex;
    if( PageInsertBeforeFirstPage == inAfterPageNumber )
        kidsIndex = -1 ;
    else
        kidsIndex = PdfPagesTree::GetPosInKids( afterPageObj ) ;

    // insert our page into the tree

    // TODO:
    // Passing parentObj instead of GetRoot() caused a crash that took me ours to fix.
    // Don't know wether I broke this with a change of mine before or not.
    // Maybe this fix is incorrect, too (I think though)
    // At least creation test works now again.
    InsertPages( kidsIndex, pPage, GetRoot(), 1 ); //parentObj, 1 ) ;
}

int PdfPagesTree::ChangePagesCount( PdfObject* inPageObj, int inDelta )
{
    // Increment or decrement inPagesDict's Count by inDelta, and return the new count.
    // Simply return the current count if inDelta is 0.
    int	cnt = inPageObj->GetDictionary().GetKey( "Count" )->GetNumber();
    if( 0 != inDelta ) 
    {
        cnt += inDelta ;
        inPageObj->GetDictionary().AddKey( "Count", PdfVariant( (long)cnt ) );
    }

    return cnt ;
}

void PdfPagesTree::InsertPages( int inAfterIndex, 
                                PdfObject* inPageOrPagesObj, 
                                PdfObject* inParentObj, 
                                int inNumPages )
{
    // insert inPageOrPagesDict in the parent's Kids array, and set inPageOrPagesDict's Parent
    int	insIdx = inAfterIndex + 1;
    
    PdfObject*	kidsArrObj = PdfPagesTree::GetKids( inParentObj ) ;
    PdfArray&	kidsArray = kidsArrObj->GetArray();

    kidsArray.insert( kidsArray.begin() + insIdx, inPageOrPagesObj->Reference() );
    inPageOrPagesObj->GetDictionary().AddKey( "Parent", inParentObj->Reference() ) ;

    // increment the pages count of all of the parent page nodes, walking up the tree
    PdfObject* tempParent = inParentObj ;
    while( NULL != tempParent ) {
        int theCount = this->ChangePagesCount( tempParent, inNumPages ) ;
        tempParent = PdfPagesTree::GetParent( tempParent ) ;
    }
}

PdfPage* PdfPagesTree::CreatePage( const PdfRect & rSize )
{
    int		last  = m_deqPageObjs.size()-1;
    PdfPage*	pPage = new PdfPage( rSize, GetRoot()->GetParent() );

    InsertPage( last, pPage );
    m_deqPageObjs.push_back( pPage );	// might as well add it here too...

    return pPage;
}

};
