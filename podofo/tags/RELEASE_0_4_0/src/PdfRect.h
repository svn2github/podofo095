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

#ifndef _PDF_RECT_H_
#define _PDF_RECT_H_

#include "PdfDefines.h"


namespace PoDoFo {

class PdfArray;
class PdfPage;
class PdfVariant;
   
/** A rectangle as defined by the PDF reference
 */
class PdfRect {
 public:
    /** Create an empty rectangle with bottom=left=with=height=0
     */
    PdfRect();

    /** Create a rectangle with a given size and position
     *  All values are in PDF units
     *	NOTE: since PDF is bottom-left origined, we pass the bottom instead of the top
     */
    PdfRect( double left, double bottom, double width, double height );
    
    /** Create a rectangle from an array
     *  All values are in PDF units
     */
    PdfRect( const PdfArray& inArray );
    
    /** Copy constructor 
     */
    PdfRect( const PdfRect & rhs );
    
    /** Converts the rectangle into an array
     *  based on PDF units and adds the array into an variant.
     *  \param var the variant to store the Rect
     */
    void ToVariant( PdfVariant & var ) const;

    /** Returns a string representation of the PdfRect
     * \returns std::string representation as [ left bottom right top ]
     */
    std::string ToString() const;

    /** Assigns the values of this PdfRect from the 4 values in the array
     *  \param inArray the array to load the values from
     */
    void FromArray( const PdfArray& inArray );

    /** Get the bottom coordinate of the rectangle
     *  \returns bottom
     */
    inline double GetBottom() const;

    /** Set the bottom coordinate of the rectangle
     *  \param dBottom
     */
    inline void SetBottom( double dBottom );

    /** Get the left coordinate of the rectangle
     *  \returns left in PDF units
     */
    inline double GetLeft() const;

    /** Set the left coordinate of the rectangle
     *  \param lLeft in PDF units
     */
    inline void SetLeft( double lLeft );

    /** Get the width of the rectangle
     *  \returns width in PDF units
     */
    inline double GetWidth() const;

    /** Set the width of the rectangle
     *  \param lWidth in PDF units
     */
    inline void SetWidth( double lWidth );

    /** Get the height of the rectangle
     *  \returns height in PDF units
     */
    inline double GetHeight() const;

    /** Set the height of the rectangle
     *  \param lHeight in PDF units
     */
    inline void SetHeight( double lHeight );

    PdfRect & operator=( const PdfRect & rhs );

 private:
    double m_dLeft;
    double m_dBottom;
    double m_dWidth;
    double m_dHeight;
};

// -----------------------------------------------------
// 
// -----------------------------------------------------
double PdfRect::GetBottom() const
{
    return m_dBottom;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
void PdfRect::SetBottom( double dBottom )
{
    m_dBottom = dBottom;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
double PdfRect::GetLeft() const
{
    return m_dLeft;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
void PdfRect::SetLeft( double dLeft )
{
    m_dLeft = dLeft;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
double PdfRect::GetWidth() const
{
    return m_dWidth;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
void PdfRect::SetWidth( double dWidth )
{
    m_dWidth = dWidth;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
double PdfRect::GetHeight() const
{
    return m_dHeight;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
void PdfRect::SetHeight( double dHeight )
{
    m_dHeight = dHeight;
}

};

#endif /* _PDF_RECT_H_ */
