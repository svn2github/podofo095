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

#include "PdfString.h"

#include "PdfFilter.h"
#include "PdfOutputDevice.h"

namespace PoDoFo {

const PdfString PdfString::StringNull      = PdfString();

PdfString::PdfString()
    : m_bHex( false )
{
}

PdfString::PdfString( const char* pszString )
    : m_bHex( false )
{
    if( pszString )
        Init( pszString, strlen( pszString ) );
}

PdfString::PdfString( const char* pszString, long lLen, bool bHex )
    : m_bHex( bHex )
{
    Init( pszString, lLen );
}

PdfString::PdfString( const PdfString & rhs )
    : m_bHex( false )
{
    this->operator=( rhs );
}

PdfString::~PdfString()
{
}

void PdfString::SetHexData( const char* pszHex, long lLen )
{
    if( !pszHex ) 
    {
        RAISE_ERROR( ePdfError_InvalidHandle );
    }

    if( lLen == -1 )
        lLen = strlen( pszHex );

    m_bHex   = true;
    m_buffer = PdfRefCountedBuffer( lLen + 1);

    memcpy( m_buffer.Buffer(), pszHex, lLen );
    m_buffer.Buffer()[lLen] = '\0';
}

void PdfString::Write ( PdfOutputDevice* pDevice ) const
{
    // Strings in PDF documents may contain \0 especially if they are encrypted
    // this case has to be handled!

    pDevice->Print( m_bHex ? "<" : "(" );
    pDevice->Write( m_buffer.Buffer(), m_buffer.Size()-1 );
    pDevice->Print( m_bHex ? ">" : ")" );
}

const PdfString & PdfString::operator=( const PdfString & rhs )
{
    this->m_bHex    = rhs.m_bHex;
    this->m_buffer  = rhs.m_buffer;

    return *this;
}

void PdfString::Init( const char* pszString, long lLen )
{
    const PdfFilter* pFilter;
    char* pBuf;
    long  lBufLen;

    // TODO: escape characters inside of strings!

    if( pszString ) 
    {
        if( m_bHex ) 
        {
            pFilter = PdfFilterFactory::Create( ePdfFilter_ASCIIHexDecode );
            if( pFilter ) 
            {
                pFilter->Encode( pszString, lLen, &pBuf, &lBufLen );
                m_buffer = PdfRefCountedBuffer( pBuf, lBufLen );
            }
            else
            {
                RAISE_ERROR( ePdfError_UnsupportedFilter );
            }
        }
        else
        {
            m_buffer = PdfRefCountedBuffer( lLen + 1);
            memcpy( m_buffer.Buffer(), pszString, lLen );
            m_buffer.Buffer()[lLen] = '\0';
        }
    }
}

};


