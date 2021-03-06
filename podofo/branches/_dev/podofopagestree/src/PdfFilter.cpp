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

#include "PdfDefines.h"
#include "PdfFilter.h"

#include "PdfArray.h"
#include "PdfDictionary.h"
#include "PdfFiltersPrivate.h"
#include "PdfOutputStream.h"

namespace PoDoFo {

// All known filters
static const char* aszFilters[] = {
    "ASCIIHexDecode",
    "ASCII85Decode",
    "LZWDecode",
    "FlateDecode",
    "RunLengthDecode",
    "CCITTFaxDecode", 
    "JBIG2Decode",
    "DCTDecode",
    "JPXDecode",
    "Crypt",
    NULL
};


/** Create a filter that is a PdfOutputStream.
 *
 *  All data written to this stream is encoded using a
 *  filter and written to another PdfOutputStream.
 *
 *  The passed output stream is owned by this PdfOutputStream
 *  and deleted along with it.
 */
class PdfFilteredEncodeStream : public PdfOutputStream{
 public:
    /** Create a filtered output stream.
     *
     *  All data written to this stream is encoded using the passed filter type
     *  and written to the passed output stream which will be deleted 
     *  by this PdfFilteredEncodeStream.
     *
     *  \param pOutputStream write all data to this output stream after encoding the data.
     *  \param eFilter use this filter for encoding.
     *  \param bOwnStream if true pOutputStream will be deleted along with this filter
     */
    PdfFilteredEncodeStream( PdfOutputStream* pOutputStream, const EPdfFilter eFilter, bool bOwnStream )
        : m_pOutputStream( pOutputStream )
    {
        m_filter = PdfFilterFactory::Create( eFilter );

        if( !m_filter.get() ) 
        {
            PODOFO_RAISE_ERROR( ePdfError_UnsupportedFilter );
        }

        m_filter->BeginEncode( pOutputStream );

        if( !bOwnStream )
            m_pOutputStream = NULL;
    }

    virtual ~PdfFilteredEncodeStream()
    {
        delete m_pOutputStream;
    }

    /** Write data to the output stream
     *  
     *  \param pBuffer the data is read from this buffer
     *  \param lLen    the size of the buffer 
     */
    virtual long Write( const char* pBuffer, long lLen )
    {
        m_filter->EncodeBlock( pBuffer, lLen );
        
        return 0;
    }

    virtual void Close() 
    {
        m_filter->EndEncode();
    }

private:
    PdfOutputStream*         m_pOutputStream;
    std::auto_ptr<PdfFilter> m_filter;
};

/** Create a filter that is a PdfOutputStream.
 *
 *  All data written to this stream is decoded using a
 *  filter and written to another PdfOutputStream.
 *
 *  The passed output stream is owned by this PdfOutputStream
 *  and deleted along with it.
 */
class PdfFilteredDecodeStream : public PdfOutputStream {
 public:
    /** Create a filtered output stream.
     *
     *  All data written to this stream is decoded using the passed filter type
     *  and written to the passed output stream which will be deleted 
     *  by this PdfFilteredEncodeStream.
     *
     *  \param pOutputStream write all data to this output stream after decoding the data.
     *                       The PdfOutputStream is deleted along with this object.
     *  \param eFilter use this filter for decoding.
     *  \param bOwnStream if true pOutputStream will be deleted along with this filter
     *  \param pDecodeParms additional parameters for decoding
     */
    PdfFilteredDecodeStream( PdfOutputStream* pOutputStream, const EPdfFilter eFilter, bool bOwnStream,
                             const PdfDictionary* pDecodeParms = NULL )
        : m_pOutputStream( pOutputStream )
    {
        m_filter = PdfFilterFactory::Create( eFilter );
        if( !m_filter.get() ) 
        {
            PODOFO_RAISE_ERROR( ePdfError_UnsupportedFilter );
        }

        m_filter->BeginDecode( pOutputStream, pDecodeParms );

        if( !bOwnStream )
            m_pOutputStream = NULL;
    }

    virtual ~PdfFilteredDecodeStream()
    {
        delete m_pOutputStream;
    }

    /** Write data to the output stream
     *  
     *  \param pBuffer the data is read from this buffer
     *  \param lLen    the size of the buffer 
     */
    virtual long Write( const char* pBuffer, long lLen )
    {
        m_filter->DecodeBlock( pBuffer, lLen );
        return 0;
    }

    virtual void Close() 
    {
        m_filter->EndDecode();
    }

private:
    PdfOutputStream*         m_pOutputStream;
    std::auto_ptr<PdfFilter> m_filter;
};


// -----------------------------------------------------
// Actual PdfFilter code
// -----------------------------------------------------


PdfFilter::PdfFilter() 
    : m_pOutputStream( NULL )
{
}

void PdfFilter::Encode( const char* pInBuffer, long lInLen, char** ppOutBuffer, long* plOutLen ) const
{
    if( !this->CanEncode() )
    {
        PODOFO_RAISE_ERROR( ePdfError_UnsupportedFilter );
    }

    PdfMemoryOutputStream stream;

    const_cast<PdfFilter*>(this)->BeginEncode( &stream );
    const_cast<PdfFilter*>(this)->EncodeBlock( pInBuffer, lInLen );
    const_cast<PdfFilter*>(this)->EndEncode();

    *ppOutBuffer = stream.TakeBuffer();
    *plOutLen    = stream.GetLength();
}

void PdfFilter::Decode( const char* pInBuffer, long lInLen, char** ppOutBuffer, long* plOutLen, 
                        const PdfDictionary* pDecodeParms ) const
{
    if( !this->CanDecode() )
    {
        PODOFO_RAISE_ERROR( ePdfError_UnsupportedFilter );
    }

    PdfMemoryOutputStream stream;

    const_cast<PdfFilter*>(this)->BeginDecode( &stream, pDecodeParms );
    const_cast<PdfFilter*>(this)->DecodeBlock( pInBuffer, lInLen );
    const_cast<PdfFilter*>(this)->EndDecode();

    *ppOutBuffer = stream.TakeBuffer();
    *plOutLen    = stream.GetLength();
}

// -----------------------------------------------------
// PdfFilterFactory code
// -----------------------------------------------------

std::auto_ptr<PdfFilter> PdfFilterFactory::Create( const EPdfFilter eFilter ) 
{
    PdfFilter* pFilter = NULL;
    switch( eFilter )
    {
        case ePdfFilter_ASCIIHexDecode:
            pFilter = new PdfHexFilter();
            break;
            
        case ePdfFilter_ASCII85Decode:
            pFilter = new PdfAscii85Filter();
            break;
            
        case ePdfFilter_LZWDecode:
            pFilter = new PdfLZWFilter();
            break;
            
        case ePdfFilter_FlateDecode:
            pFilter = new PdfFlateFilter();
            break;
            
        case ePdfFilter_RunLengthDecode:
            pFilter = new PdfRLEFilter();
            break;
            
        case ePdfFilter_DCTDecode:
#ifdef PODOFO_HAVE_JPEG_LIB
            pFilter = new PdfDCTFilter();
            break;
#else
            break;
#endif // PODOFO_HAVE_JPEG_LIB

        case ePdfFilter_CCITTFaxDecode:
#ifdef PODOFO_HAVE_TIFF_LIB
            pFilter = new PdfCCITTFilter();
            break;
#else
            break;
#endif // PODOFO_HAVE_TIFF_LIB


        case ePdfFilter_JBIG2Decode:
        case ePdfFilter_JPXDecode:
        case ePdfFilter_Crypt:
        default:
            break;
    }

    return std::auto_ptr<PdfFilter>(pFilter);
}

PdfOutputStream* PdfFilterFactory::CreateEncodeStream( const TVecFilters & filters, PdfOutputStream* pStream ) 
{
    TVecFilters::const_iterator it = filters.begin();

    PODOFO_RAISE_LOGIC_IF( !filters.size(), "Cannot create an EncodeStream from an empty list of filters" );

    PdfFilteredEncodeStream* pFilter = new PdfFilteredEncodeStream( pStream, *it, false );
    ++it;

    while( it != filters.end() ) 
    {
        pFilter = new PdfFilteredEncodeStream( pFilter, *it, true );
        ++it;
    }

    return pFilter;
}

PdfOutputStream* PdfFilterFactory::CreateDecodeStream( const TVecFilters & filters, PdfOutputStream* pStream,
                                                       const PdfDictionary* pDictionary ) 
{
    TVecFilters::const_reverse_iterator it = filters.rbegin();

    PODOFO_RAISE_LOGIC_IF( !filters.size(), "Cannot create an DecodeStream from an empty list of filters" );

    // TODO: support arrays and indirect objects here and the short name /DP
    if( pDictionary && pDictionary->HasKey( "DecodeParms" ) && pDictionary->GetKey( "DecodeParms" )->IsDictionary() )
        pDictionary = &(pDictionary->GetKey( "DecodeParms" )->GetDictionary());

    PdfFilteredDecodeStream* pFilter = new PdfFilteredDecodeStream( pStream, *it, false, pDictionary );
    ++it;

    while( it != filters.rend() ) 
    {
        pFilter = new PdfFilteredDecodeStream( pFilter, *it, true, pDictionary );
        ++it;
    }

    return pFilter;
}

EPdfFilter PdfFilterFactory::FilterNameToType( const PdfName & name )
{
    int i = 0;

    while( aszFilters[i] )
    {
        if( name == aszFilters[i] )
            return static_cast<EPdfFilter>(i);
        
        ++i;
    }

    PODOFO_RAISE_ERROR_INFO( ePdfError_UnsupportedFilter, name.GetName().c_str() );
}

const char* PdfFilterFactory::FilterTypeToName( EPdfFilter eFilter )
{
    return aszFilters[static_cast<int>(eFilter)];
}

TVecFilters PdfFilterFactory::CreateFilterList( const PdfObject* pObject )
{
    TVecFilters filters;

    const PdfObject* pObj    = NULL;

    if( pObject->IsDictionary() && pObject->GetDictionary().HasKey( "Filter" ) )
        pObj = pObject->GetDictionary().GetKey( "Filter" );
    else if( pObject->IsArray() )
        pObj = pObject;
    else if( pObject->IsName() ) 
        pObj = pObject;


    if (!pObj)
	// Object had no /Filter key . Return a null filter list.
	return filters;

    if( pObj->IsName() ) 
        filters.push_back( PdfFilterFactory::FilterNameToType( pObj->GetName() ) );
    else if( pObj->IsArray() ) 
    {
        TCIVariantList it = pObj->GetArray().begin();

        while( it != pObj->GetArray().end() )
        {
            if (! (*it).IsName() )
			{
                PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidDataType, "Filter array contained unexpected non-name type" );
			}
            filters.push_back( PdfFilterFactory::FilterNameToType( (*it).GetName() ) );
                
            ++it;
        }
    }

    return filters;
}

};
