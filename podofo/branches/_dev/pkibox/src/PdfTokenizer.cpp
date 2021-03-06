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

#include "PdfTokenizer.h"

#include "PdfArray.h"
#include "PdfDictionary.h"
#include "PdfEncrypt.h"
#include "PdfInputDevice.h"
#include "PdfName.h"
#include "PdfString.h"
#include "PdfReference.h"
#include "PdfVariant.h"

#include <sstream>

#define PDF_BUFFER 4096

#define DICT_SEP_LENGTH 2
#define NULL_LENGTH     4
#define TRUE_LENGTH     4
#define FALSE_LENGTH    5
#define REF_LENGTH      1

namespace PoDoFo {

namespace PdfTokenizerNameSpace{

// Generate the delimiter character map at runtime
// so that it can be derived from the more easily
// maintainable structures in PdfDefines.h
const char * genDelMap()
{
    int    i;
    char* map = static_cast<char*>(malloc(256));
    for (i = 0; i < 256; i++)
        map[i] = '\0';
    for (i = 0; i < PoDoFo::s_nNumDelimiters; ++i)
        map[PoDoFo::s_cDelimiters[i]] = 1;
    return map;
}

// Generate the whitespace character map at runtime
// so that it can be derived from the more easily
// maintainable structures in PdfDefines.h
const char * genWsMap()
{
    int   i;
    char* map = static_cast<char*>(malloc(256));
    for (i = 0; i < 256; i++)
        map[i] = '\0';
    for (i = 0; i < PoDoFo::s_nNumWhiteSpaces; ++i)
        map[PoDoFo::s_cWhiteSpaces[i]] = 1;
    return map;
}


};

const char * const PdfTokenizer::m_delimiterMap  = PdfTokenizerNameSpace::genDelMap();
const char * const PdfTokenizer::m_whitespaceMap = PdfTokenizerNameSpace::genWsMap();
const char PdfTokenizer::m_octMap[]        = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0
};


PdfTokenizer::PdfTokenizer()
    : m_buffer( PDF_BUFFER )
{

}

PdfTokenizer::PdfTokenizer( const char* pBuffer, long lLen )
    : m_device( pBuffer, lLen ), m_buffer( PDF_BUFFER )
{

}

PdfTokenizer::PdfTokenizer( const PdfRefCountedInputDevice & rDevice, const PdfRefCountedBuffer & rBuffer )
    : m_device( rDevice ), m_buffer( rBuffer )
{
}

PdfTokenizer::~PdfTokenizer()
{
}

bool PdfTokenizer::GetNextToken( const char*& pszToken , EPdfTokenType* peType )
{
    int  c; 
    int  counter  = 0;

    // check first if there are quequed tokens and return them first
    if( m_deqQueque.size() )
    {
        TTokenizerPair pair = m_deqQueque.front();
        m_deqQueque.pop_front();

        if( peType )
            *peType = pair.second;

        strcpy( m_buffer.GetBuffer(), pair.first.c_str() );
        pszToken = m_buffer.GetBuffer();
	return true;
    }
    
    if( !m_device.Device() )
    {
        PODOFO_RAISE_ERROR( ePdfError_InvalidHandle );
    }

    if( peType )
        *peType = ePdfTokenType_Token;

    while( (c = m_device.Device()->Look()) != EOF && counter < m_buffer.GetSize() )
    {
        // ignore leading whitespaces
        if( !counter && IsWhitespace( c ) )
        {
            // Consume the whitespace character
            c = m_device.Device()->GetChar();
            continue;
        }
        // ignore comments
        else if( c == '%' ) 
        {
            // Consume all characters before the next line break
            do {
                c = m_device.Device()->GetChar();
            } while( c != EOF && c != 0x0A );
            // If we've already read one or more chars of a token, return them, since
            // comments are treated as token-delimiting whitespace. Otherwise keep reading
            // at the start of the next line.
            if (counter)
                break;
        }
        // special handling for << and >> tokens
        else if( !counter && (c == '<' || c == '>' ) )
        {
            if( peType )
                *peType = ePdfTokenType_Delimiter;

            // retrieve c really from stream
            c = m_device.Device()->GetChar();
            m_buffer.GetBuffer()[counter] = c;
            ++counter;

            char n = m_device.Device()->Look();
            // Is n another < or > , ie are we opening/closing a dictionary?
            // If so, consume that character too.
            if( n == c )
            {
                n = m_device.Device()->GetChar();
                m_buffer.GetBuffer()[counter] = n;
                ++counter;
            }
            // `m_buffer' contains one of < , > , << or >> ; we're done .
            break;
        }
        else if( counter && (IsWhitespace( c ) || IsDelimiter( c )) )
        {
            // Next (unconsumed) character is a token-terminating char, so
            // we have a complete token and can return it.
            break;
        }
        else
        {
            // Consume the next character and add it to the token we're building.
            c = m_device.Device()->GetChar();
            m_buffer.GetBuffer()[counter] = c;
            ++counter;

            if( IsDelimiter( c ) )
            {
                // All delimeters except << and >> (handled above) are
                // one-character tokens, so if we hit one we can just return it
                // immediately.
                if( peType )
                    *peType = ePdfTokenType_Delimiter;
                break;
            }
        }
    }

    m_buffer.GetBuffer()[counter] = '\0';


    if( c == EOF && !counter )
    {
        // No characters were read before EOF, so we're out of data.
        // Ensure the buffer points to NULL in case someone fails to check the return value.
        pszToken = 0;
        return false;
    }

    pszToken = m_buffer.GetBuffer();
    return true;
}

bool PdfTokenizer::IsNextToken( const char* pszToken )
{
    if( !pszToken )
    {
        PODOFO_RAISE_ERROR( ePdfError_InvalidHandle );
    }

    const char* pszRead;
    bool gotToken = this->GetNextToken( pszRead, NULL );

    if (!gotToken)
    {
        PODOFO_RAISE_ERROR( ePdfError_UnexpectedEOF );
    }

    return (strcmp( pszToken, pszRead ) == 0);
}

long PdfTokenizer::GetNextNumber()
{
    EPdfTokenType eType;
    const char* pszRead;
    bool gotToken = this->GetNextToken( pszRead, &eType );

    if( !gotToken )
    {
        PODOFO_RAISE_ERROR_INFO( ePdfError_UnexpectedEOF, "Expected number" );
    }

    char* end;
    long l = strtol( pszRead, &end, 10 );
    if( end == pszRead )
    {
        // Don't consume the token
        this->QuequeToken( pszRead, eType );
        PODOFO_RAISE_ERROR( ePdfError_NoNumber );
    }

    return l;
}

void PdfTokenizer::GetNextVariant( PdfVariant& rVariant, PdfEncrypt* pEncrypt )
{
   EPdfTokenType eTokenType;
   const char*   pszRead;
   bool gotToken = this->GetNextToken( pszRead, &eTokenType );

   if (!gotToken)
   {
       PODOFO_RAISE_ERROR_INFO( ePdfError_UnexpectedEOF, "Expected variant." );
   }

   this->GetNextVariant( pszRead, eTokenType, rVariant, pEncrypt );
}

void PdfTokenizer::GetNextVariant( const char* pszToken, EPdfTokenType eType, PdfVariant& rVariant, PdfEncrypt* pEncrypt )
{
    EPdfDataType eDataType = this->DetermineDataType( pszToken, eType, rVariant );
    
    if( eDataType == ePdfDataType_Null ||
        eDataType == ePdfDataType_Bool ||
        eDataType == ePdfDataType_Number ||
        eDataType == ePdfDataType_Real ||
        eDataType == ePdfDataType_Reference )
    {
        // the data was already read into rVariant by the DetermineDataType function
        return;
    }

    this->ReadDataType( eDataType, rVariant, pEncrypt );
}

EPdfDataType PdfTokenizer::DetermineDataType( const char* pszToken, EPdfTokenType eTokenType, PdfVariant& rVariant )
{
    if( eTokenType == ePdfTokenType_Token ) 
    {
        // check for the two special datatypes
        // null and boolean.
        // check for numbers
        if( strncmp( "null", pszToken, NULL_LENGTH ) == 0 )
        {
            rVariant = PdfVariant();
            return ePdfDataType_Null;
        }
        else if( strncmp( "true", pszToken, TRUE_LENGTH ) == 0 )
        {
            rVariant = PdfVariant( true );
            return ePdfDataType_Bool;
        }
        else if( strncmp( "false", pszToken, FALSE_LENGTH ) == 0 )
        {
            rVariant = PdfVariant( false );
            return ePdfDataType_Bool;
        }

        EPdfDataType eDataType = ePdfDataType_Number;
        const char*  pszStart  = pszToken;

        while( *pszStart )
        {
            if( *pszStart == '.' )
                eDataType = ePdfDataType_Real;
            else if( !(isdigit( static_cast<const unsigned char>(*pszStart) ) || *pszStart == '-' || *pszStart == '+' ) )
            {
                eDataType = ePdfDataType_Unknown;
                break;
            }
            
            ++pszStart;
        }

        if( eDataType == ePdfDataType_Real ) 
        {
            rVariant = PdfVariant( strtod( pszToken, NULL ) );
            return ePdfDataType_Real;
        }
        else if( eDataType == ePdfDataType_Number ) 
        {
            rVariant = PdfVariant( strtol( pszToken, NULL, 10 ) );

            // read another two tokens to see if it is a reference
            // we cannot be sure that there is another token
            // on the input device, so if we hit EOF just return
            // ePdfDataType_Number .
            EPdfTokenType eSecondTokenType;
            bool gotToken = this->GetNextToken( pszToken, &eSecondTokenType );
            if (!gotToken)
                // No next token, so it can't be a reference
                return eDataType;
            if( eSecondTokenType != ePdfTokenType_Token ) 
            {
                this->QuequeToken( pszToken, eSecondTokenType );
                return eDataType;
            }
            
            pszStart = pszToken;
            long  l   = strtol( pszStart, const_cast<char**>(&pszToken), 10 );
            if( pszToken == pszStart ) 
            {
                this->QuequeToken( pszStart, eSecondTokenType );
                return eDataType;
            }

            std::string backup( pszStart );
            EPdfTokenType eThirdTokenType;
            gotToken = this->GetNextToken( pszToken, &eThirdTokenType );
            if (!gotToken)
                // No third token, so it can't be a reference
                return eDataType;
            if( eThirdTokenType == ePdfTokenType_Token &&
                pszToken[0] == 'R' && pszToken[1] == '\0' )
            {
                rVariant = PdfReference( rVariant.GetNumber(), l);
                return ePdfDataType_Reference;
            }
            else
            {
                this->QuequeToken( backup.c_str(), eSecondTokenType );
                this->QuequeToken( pszToken, eThirdTokenType );
                return eDataType;
            }
        } 
    }
    else if( eTokenType == ePdfTokenType_Delimiter ) 
    {
        if( strncmp( "<<", pszToken, DICT_SEP_LENGTH ) == 0 )
            return ePdfDataType_Dictionary;
        else if( pszToken[0] == '[' )
            return ePdfDataType_Array;
        else if( pszToken[0] == '(' )
            return ePdfDataType_String;
        else if( pszToken[0] == '<' ) 
            return ePdfDataType_HexString;
        else if( pszToken[0] == '/' )
            return ePdfDataType_Name;
    }

    if( false ) 
    {
        std::ostringstream ss;
#ifdef _MSC_VER
        ss << "Got unexpected PDF data in" << __FILE__ << ", line " << __LINE__
#else
        ss << "Got unexpected PDF data in" << __FUNCTION__
#endif
           << ": \""
           << pszToken
           << "\". Current read offset is "
           << m_device.Device()->Tell()
           << " which should be around the problem.\n";
        PdfError::DebugMessage(ss.str().c_str());
    }

    return ePdfDataType_Unknown;
}

void PdfTokenizer::ReadDataType( EPdfDataType eDataType, PdfVariant& rVariant, PdfEncrypt* pEncrypt )
{
    switch( eDataType ) 
    {
        case ePdfDataType_Dictionary:
            this->ReadDictionary( rVariant, pEncrypt );
            break;
        case ePdfDataType_Array:
            this->ReadArray( rVariant, pEncrypt );
            break;
        case ePdfDataType_String:
            this->ReadString( rVariant, pEncrypt );
            break;
        case ePdfDataType_HexString:
            this->ReadHexString( rVariant, pEncrypt );
            break;
        case ePdfDataType_Name:
            this->ReadName( rVariant );
            break;

        // The following datatypes are not handled by read datatype
        // but are already parsed by DetermineDatatype
        case ePdfDataType_Null:
        case ePdfDataType_Bool:
        case ePdfDataType_Number:
        case ePdfDataType_Real:
        case ePdfDataType_Reference:
        case ePdfDataType_Unknown:
        case ePdfDataType_RawData:

        default:
        {
            PdfError::LogMessage( eLogSeverity_Debug, "Got Datatype: %i\n", eDataType );
            PODOFO_RAISE_ERROR( ePdfError_InvalidDataType );
        }
    }
}

void PdfTokenizer::ReadDictionary( PdfVariant& rVariant, PdfEncrypt* pEncrypt )
{
    PdfVariant    val;
    PdfName       key;
    PdfDictionary dict;
    EPdfTokenType eType;
    const char *  pszToken;

    for( ;; ) 
    {
        bool gotToken = this->GetNextToken( pszToken, &eType );
        if (!gotToken)
        {
            PODOFO_RAISE_ERROR_INFO(ePdfError_UnexpectedEOF, "Expected dictionary key name or >> delim.");
        }
        if( eType == ePdfTokenType_Delimiter && strncmp( ">>", pszToken, DICT_SEP_LENGTH ) == 0 )
            break;

        this->GetNextVariant( pszToken, eType, val, pEncrypt );
        // Convert the read variant to a name; throws InvalidDataType if not a name.
        key = val.GetName();
        // Get the next variant. If there isn't one, it'll throw UnexpectedEOF.
        this->GetNextVariant( val, pEncrypt );

        dict.AddKey( key, val );
    }

    rVariant = dict;
}

void PdfTokenizer::ReadArray( PdfVariant& rVariant, PdfEncrypt* pEncrypt )
{
    const char*   pszToken;
    EPdfTokenType eType;
    PdfVariant    var;
    PdfArray      array;

    for( ;; ) 
    {
        bool gotToken = this->GetNextToken( pszToken, &eType );
        if (!gotToken)
        {
            PODOFO_RAISE_ERROR_INFO(ePdfError_UnexpectedEOF, "Expected array item or ] delim.");
        }
        if( eType == ePdfTokenType_Delimiter && pszToken[0] == ']' )
            break;

        this->GetNextVariant( pszToken, eType, var, pEncrypt );
        array.push_back( var );
    }

    rVariant = array;
}

void PdfTokenizer::ReadString( PdfVariant& rVariant, PdfEncrypt* pEncrypt )
{
    int               c;

    bool              bIgnore       = false;
    bool              bOctEscape    = false;
    int               nOctCount     = 0;
    char              cOctValue     = 0;
    int               nBalanceCount = 0; // Balanced parathesis do not have to be escaped in strings

    m_vecBuffer.clear();

    while( (c = m_device.Device()->GetChar()) != EOF )
    {
        // end of stream reached
        if( !bIgnore && !nBalanceCount && c == ')' )
            break;

        if( !bIgnore && c == '(' )
            ++nBalanceCount;
        else if( !bIgnore && c == ')' )
            --nBalanceCount;

        if( bIgnore && isdigit( c ) )
            // The last character we have read was a '\\',
            // so we check now for a digit to find stuff like \005
            bOctEscape = true;

        if( bOctEscape ) 
        {
            if( !m_octMap[c] || nOctCount > 2 )
            {
                m_vecBuffer.push_back ( cOctValue );
                bOctEscape = false;
                nOctCount  = 0;
                cOctValue  = 0;
            } 
            else
            {
                cOctValue <<= 3;
                cOctValue  |= (c-'0' & 0x07);
                ++nOctCount;
            }
        }

        bIgnore = (c == '\\') && !bIgnore;
        if( !bIgnore && !bOctEscape ) 
            m_vecBuffer.push_back( static_cast<char>(c) );
    }

    // In case the string ends with a octal escape sequence
    if( bOctEscape )
        m_vecBuffer.push_back ( cOctValue );

    if( pEncrypt && m_vecBuffer.size() )
        pEncrypt->Encrypt( reinterpret_cast<unsigned char*>(&(m_vecBuffer[0])), m_vecBuffer.size() );

    if( m_vecBuffer.size() )
        rVariant = PdfString( &(m_vecBuffer[0]), m_vecBuffer.size() );
    else
        rVariant = PdfString("");
}

void PdfTokenizer::ReadHexString( PdfVariant& rVariant, PdfEncrypt* pEncrypt )
{
    int        c;

    m_vecBuffer.clear();

    while( (c = m_device.Device()->GetChar()) != EOF )
    {
        // end of stream reached
        if( c == '>' )
            break;

        // only a hex digits
        if( isdigit( c ) || 
            ( c >= 'A' && c <= 'F') ||
            ( c >= 'a' && c <= 'f'))
            m_vecBuffer.push_back( c );
    }

    // pad to an even length if necessary
    if( m_vecBuffer.size() % 2 )
        m_vecBuffer.push_back( '0' );

    PdfString string;
    string.SetHexData( m_vecBuffer.size() ? &(m_vecBuffer[0]) : "", m_vecBuffer.size(), pEncrypt );

    rVariant = string;
}

void PdfTokenizer::ReadName( PdfVariant& rVariant )
{
    EPdfTokenType eType;
    const char*   pszToken;
    bool gotToken = this->GetNextToken( pszToken, &eType );

    if( !gotToken || eType != ePdfTokenType_Token )
    {
        // We got an empty name which is legal according to the PDF specification
        // Some weird PDFs even use them.
        rVariant = PdfName();

        // Queque the token again
        if( gotToken )
            QuequeToken( pszToken, eType );
    }
    else
        rVariant = PdfName::FromEscaped( pszToken );
}

void PdfTokenizer::QuequeToken( const char* pszToken, EPdfTokenType eType )
{
    m_deqQueque.push_back( TTokenizerPair( std::string( pszToken ), eType ) );
}

};
