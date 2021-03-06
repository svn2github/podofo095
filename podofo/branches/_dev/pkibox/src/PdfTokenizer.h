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

#ifndef _PDF_TOKENIZER_H_
#define _PDF_TOKENIZER_H_

#include "PdfDefines.h"
#include "PdfRefCountedBuffer.h"
#include "PdfRefCountedInputDevice.h"

#include <deque>

namespace PoDoFo {

class PdfEncrypt;
class PdfVariant;

typedef enum EPdfTokenType {
    ePdfTokenType_Delimiter,
    ePdfTokenType_Token,

    ePdfTokenType_Unknown = 0xFF
};

typedef std::pair<std::string,EPdfTokenType> TTokenizerPair;
typedef std::deque<TTokenizerPair>           TTokenizerQueque;
typedef TTokenizerQueque::iterator           TITokenizerQueque;
typedef TTokenizerQueque::const_iterator     TCITokenizerQueque;


/**
 * A simple tokenizer for PDF files and PDF content streams
 */
class PODOFO_API PdfTokenizer {
 public:
    PdfTokenizer();

    PdfTokenizer( const char* pBuffer, long lLen );
    PdfTokenizer( const PdfRefCountedInputDevice & rDevice, const PdfRefCountedBuffer & rBuffer );

    virtual ~PdfTokenizer();

    /** Reads the next token from the current file position
     *  ignoring all comments.
     *
     *  \param[out] pszBuf On true return, set to a pointer to the read
     *                     token (a NULL-terminated C string). The pointer is
     *                     to memory owned by PdfTokenizer and must NOT be
     *                     freed.  The contents are invalidated on the next
     *                     call to GetNextToken(..) and by the destruction of
     *                     the PdfTokenizer. Undefined on false return.
     *
     *  \param[out] peType On true return, if not NULL the type of the read token
     *                     will be stored into this parameter. Undefined on false
     *                     return.
     * 
     *  \returns           True if a token was read, false if there are no
     *                     more tokens to read.
     *
     *  \see GetBuffer
     */
    bool GetNextToken( const char *& pszToken, EPdfTokenType* peType = NULL);

    /** Reads the next token from the current file position
     *  ignoring all comments and compare the passed token
     *  to the read token.
     *
     *  If there is no next token available, throws UnexpectedEOF.
     *
     *  \param pszToken a token that is compared to the 
     *                  read token
     *
     *  \returns true if the read token equals the passed token.
     */
    bool IsNextToken( const char* pszToken );

    /** Read the next number from the current file position
     *  ignoring all comments.
     *
     *  Raises NoNumber exception if the next token is no number, and
     *  UnexpectedEOF if no token could be read. No token is consumed if
     *  NoNumber is thrown.
     *
     *  \returns a number read from the input device.
     */
    long GetNextNumber();

    /** Read the next variant from the current file position
     *  ignoring all comments.
     *
     *  Raises an UnexpectedEOF exception if there is no variant left in the
     *  file.
     *
     *  \param rVariant write the read variant to this value
     *  \param pEncrypt an encryption object which is used to decrypt strings during parsing
     */
    void GetNextVariant( PdfVariant& rVariant, PdfEncrypt* pEncrypt );

    /** Returns true if the given character is a whitespace 
     *  according to the pdf reference
     *
     *  \returns true if it is a whitespace character otherwise false
     */
    PODOFO_NOTHROW inline static bool IsWhitespace(const unsigned char ch);

    /** Returns true if the given character is a delimiter
     *  according to the pdf reference
     *
     *  \returns true if it is a delimiter character otherwise false
     */
    PODOFO_NOTHROW inline static bool IsDelimiter(const unsigned char ch);

    /**
     * True if the passed character is a regular character according to the PDF
     * reference (Section 3.1.1, Character Set); ie it is neither a white-space
     * nor a delimeter character.
     */
    PODOFO_NOTHROW inline static bool IsRegular(const unsigned char ch);

    /**
     * True if the passed character is within the generally accepted "printable"
     * ASCII range.
     */
    PODOFO_NOTHROW inline static bool IsPrintable(const unsigned char ch);


 protected:
    /** Read the next variant from the current file position
     *  ignoring all comments.
     *
     *  Raises an exception if there is no variant left in the file.
     *
     *  \param pszToken a token that has already been read
     *  \param eType type of the passed token
     *  \param rVariant write the read variant to this value
     *  \param pEncrypt an encryption object which is used to decrypt strings during parsing
     */
    void GetNextVariant( const char* pszToken, EPdfTokenType eType, PdfVariant& rVariant, PdfEncrypt* pEncrypt );

    /** Determine the possible datatype of a token.
     *  Numbers, reals, bools or NULL values are parsed directly by this function
     *  and saved to a variant.
     *
     *  \returns the expected datatype
     */
    EPdfDataType DetermineDataType( const char* pszToken, EPdfTokenType eType, PdfVariant& rVariant );

    void ReadDataType( EPdfDataType eDataType, PdfVariant& rVariant, PdfEncrypt* pEncrypt );

    /** Read a dictionary from the input device
     *  and store it into a variant.
     * 
     *  \param rVariant store the dictionary into this variable
     *  \param pEncrypt an encryption object which is used to decrypt strings during parsing
     */
    void ReadDictionary( PdfVariant& rVariant, PdfEncrypt* pEncrypt );

    /** Read an array from the input device
     *  and store it into a variant.
     * 
     *  \param rVariant store the array into this variable
     *  \param pEncrypt an encryption object which is used to decrypt strings during parsing
     */
    void ReadArray( PdfVariant& rVariant, PdfEncrypt* pEncrypt );

    /** Read a string from the input device
     *  and store it into a variant.
     * 
     *  \param rVariant store the string into this variable
     *  \param pEncrypt an encryption object which is used to decrypt strings during parsing
     */
    void ReadString( PdfVariant& rVariant, PdfEncrypt* pEncrypt );

    /** Read a hex string from the input device
     *  and store it into a variant.
     * 
     *  \param rVariant store the hex string into this variable
     *  \param pEncrypt an encryption object which is used to decrypt strings during parsing
     */
    void ReadHexString( PdfVariant& rVariant, PdfEncrypt* pEncrypt );

    /** Read a name from the input device
     *  and store it into a variant.
     * 
     *  Throws UnexpectedEOF if there is nothing to read.
     *
     *  \param rVariant store the name into this variable
     */
    void ReadName( PdfVariant& rVariant );

    /** Add a token to the queque of tokens.
     *  GetNextToken() will return all quequed tokens first before
     *  reading new tokens from the input device.
     *
     *  \param pszToken string of the token
     *  \param eType type of the token
     *
     *  \see GetNextToken
     */
    void QuequeToken( const char* pszToken, EPdfTokenType eType );

 protected:
    PdfRefCountedInputDevice m_device;
    PdfRefCountedBuffer      m_buffer;

 private:
    // 256-byte array mapping character ordinal values to a truth value
    // indicating whether or not they are whitespace according to the PDF
    // standard.
    static const char * const m_delimiterMap;
    static const char * const m_whitespaceMap;
    static const char m_octMap[];


    TTokenizerQueque m_deqQueque;

    // A vector which is used as a buffer to read strings.
    // It is a member of the class to avoid reallocations while parsing.
    std::vector<char> m_vecBuffer; // we use a vector instead of a string
                                   // because we might read a unicode
                                   // string which is allowed to contain 0 bytes.

};

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline bool PdfTokenizer::IsWhitespace(const unsigned char ch)
{
    return ( PdfTokenizer::m_whitespaceMap[ch] != 0 );
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline bool PdfTokenizer::IsDelimiter(const unsigned char ch)
{
    return ( PdfTokenizer::m_delimiterMap[ch] != 0 );
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline bool PdfTokenizer::IsRegular(const unsigned char ch)
{
    return !IsWhitespace(ch) && !IsDelimiter(ch);
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline bool PdfTokenizer::IsPrintable(const unsigned char ch)
{
    return ch > 32 && ch < 125;
}

};

#endif // _PDF_TOKENIZER_H_
