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

#include "PdfRect.h"

#include "PdfArray.h"
#include "PdfPage.h"
#include "PdfVariant.h"
#include "PdfDefinesPrivate.h"

#include <iostream>
#include <sstream>
#include <iomanip>

namespace PoDoFo {

PdfRect::PdfRect()
{
    m_dBottom = m_dLeft = m_dWidth = m_dHeight = 0;
}

PdfRect::PdfRect( double dLeft, double dBottom, double dWidth, double dHeight )
{
    m_dBottom = dBottom;
    m_dLeft   = dLeft;
    m_dWidth  = dWidth;
    m_dHeight = dHeight;
}

PdfRect::PdfRect( const PdfArray& inArray )
{
    m_dBottom = m_dLeft = m_dWidth = m_dHeight = 0;
    FromArray( inArray );
}

PdfRect::PdfRect( const PdfRect & rhs )
{
    this->operator=( rhs );
}

void PdfRect::ToVariant( PdfVariant & var ) const
{
    PdfArray array;
    
    array.push_back( PdfVariant( m_dLeft ) );
    array.push_back( PdfVariant( m_dBottom ) );
    array.push_back( PdfVariant( (m_dWidth+m_dLeft) ) );
    array.push_back( PdfVariant( (m_dHeight+m_dBottom) ) );

    var = array;
}

std::string PdfRect::ToString() const
{
    PdfVariant  var;
    std::string str;
    this->ToVariant( var );
    var.ToString( str );

    return str;

    /*
    std::ostringstream	oStr;
    oStr << "[ ";
    oStr << std::setprecision( 3 ) << m_dLeft << " ";
    oStr << std::setprecision( 3 ) << m_dBottom << " ";
    oStr << std::setprecision( 3 ) << m_dWidth + m_dLeft << " ";
    oStr << std::setprecision( 3 ) << m_dHeight - m_dBottom << " ]";
    
    return oStr.str();
    */
}

void PdfRect::FromArray( const PdfArray& inArray )
{
    if ( inArray.size() == 4 ) 
    {
        m_dLeft   = inArray[0].GetReal();
        m_dBottom = inArray[1].GetReal();
        m_dWidth  = inArray[2].GetReal() - m_dLeft;
        m_dHeight = inArray[3].GetReal() - m_dBottom;
    }
    else 
    {
        PODOFO_RAISE_ERROR( ePdfError_ValueOutOfRange );
    }
}

PdfRect & PdfRect::operator=( const PdfRect & rhs )
{
    this->m_dBottom = rhs.m_dBottom;
    this->m_dLeft   = rhs.m_dLeft;
    this->m_dWidth  = rhs.m_dWidth;
    this->m_dHeight = rhs.m_dHeight;

    return *this;
}

};
