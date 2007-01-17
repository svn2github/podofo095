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

#include "PdfDictionary.h"
#include "PdfFiltersPrivate.h"

#include <map>

namespace PoDoFo {

std::map<EPdfFilter,PdfFilter*> PdfFilterFactory::s_mapFilters;

const PdfFilter* PdfFilterFactory::Create( const EPdfFilter eFilter ) 
{
    PdfFilter* pFilter = s_mapFilters[eFilter];

    if( !pFilter ) 
    {
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
                
            case ePdfFilter_CCITTFaxDecode:
            case ePdfFilter_JBIG2Decode:
            case ePdfFilter_DCTDecode:
            case ePdfFilter_JPXDecode:
            case ePdfFilter_Crypt:
            case ePdfFilter_Unknown:
            default:
                break;
        }

        if( pFilter )
            s_mapFilters[eFilter] = pFilter;
    }

    return pFilter;
}

};
