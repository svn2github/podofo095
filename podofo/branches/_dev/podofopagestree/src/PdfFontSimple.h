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

#ifndef _PDF_FONT_SIMPLE_H_
#define _PDF_FONT_SIMPLE_H_

#include "PdfDefines.h"
#include "PdfFont.h"

namespace PoDoFo {

/** This is a common base class for simple fonts
 *  like truetype or type1 fonts.
 */
class PdfFontSimple : public PdfFont {
 public:

    /** Create a new PdfFont object which will introduce itself
     *  automatically to every page object it is used on.
     *
     *  The font has a default font size of 12.0pt.
     *
     *  \param pMetrics pointer to a font metrics object. The font in the PDF
     *         file will match this fontmetrics object. The metrics object is 
     *         deleted along with the font.
     *  \param pEncoding the encoding of this font. The encoding will be owned 
     *                   by this font and will be deleted along with it. 
     *  \param pParent parent of the font object
     *  
     */
    PdfFontSimple( PdfFontMetrics* pMetrics, const PdfEncoding* const pEncoding, 
                   PdfVecObjects* pParent );

    /** Create a PdfFont based on an existing PdfObject
     *  \param pMetrics pointer to a font metrics object. The font in the PDF
     *         file will match this fontmetrics object. The metrics object is 
     *         deleted along with the font.
     *  \param pEncoding the encoding of this font. The encoding will be owned 
     *                   by this font and will be deleted along with it. 
     *  \param pObject an existing PdfObject
     */
    PdfFontSimple( PdfFontMetrics* pMetrics, const PdfEncoding* const pEncoding, 
                   PdfObject* pObject );

 protected:
    /** Initialize this font object.
     *
     *  \param bEmbed if true embed the font data into the PDF file.
     *  \param rsSubType the subtype of the real font.
     */
    void Init( bool bEmbed, const PdfName & rsSubType );

    /** Embed the font file directly into the PDF file.
     *
     *  \param pDescriptor font descriptor object
     */
    virtual void EmbedFont( PdfObject* pDescriptor ) = 0;
};

};

#endif /* _PDF_FONT_SIMPLE_H_ */
