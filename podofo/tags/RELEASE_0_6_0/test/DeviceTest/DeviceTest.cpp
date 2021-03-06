/***************************************************************************
 *   Copyright (C) 2005 by Dominik Seichter                                *
 *   domseichter@web.de                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "../PdfTest.h"

#include "PdfRefCountedBuffer.h"
#include "PdfOutputStream.h"

#include <stdio.h>
#include <string.h>
#define BUFFER_SIZE 4096

using namespace PoDoFo;

void RefCountedBufferTest() 
{
    const char* pszTestString = "Hello World Buffer!";
    long        lLen          = strlen( pszTestString );

    PdfRefCountedBuffer buffer1;
    PdfRefCountedBuffer buffer2;

    // test simple append 
    printf("\t -> Appending\n");
    PdfBufferOutputStream stream1( &buffer1 );
    stream1.Write( pszTestString, lLen );
    stream1.Close();
    if( buffer1.GetSize() != lLen ) 
    {
        fprintf( stderr, "Buffer size does not match! Size=%i should be %i\n", buffer1.GetSize(), lLen );
        PODOFO_RAISE_ERROR( ePdfError_TestFailed );
    }
    
    if( strcmp( buffer1.GetBuffer(), pszTestString ) != 0 ) 
    {
        fprintf( stderr, "Buffer contents do not match!\n" );
        PODOFO_RAISE_ERROR( ePdfError_TestFailed );
    }

    // test assignment
    printf("\t -> Assignment\n");
    buffer2 = buffer1;
    if( buffer1.GetSize() != buffer2.GetSize() ) 
    {
        fprintf( stderr, "Buffer sizes does not match! Size1=%i Size2=%i\n", buffer1.GetSize(), buffer2.GetSize() );
        PODOFO_RAISE_ERROR( ePdfError_TestFailed );
    }

    if( strcmp( buffer1.GetBuffer(), buffer2.GetBuffer() ) != 0 ) 
    {
        fprintf( stderr, "Buffer contents do not match after assignment!\n" );
        PODOFO_RAISE_ERROR( ePdfError_TestFailed );
    }

    // test detach
    printf("\t -> Detaching\n");
    PdfBufferOutputStream stream( &buffer2 );
    stream.Write( pszTestString, lLen );
    stream.Close();
    if( buffer2.GetSize() != lLen * 2 ) 
    {
        fprintf( stderr, "Buffer size after detach does not match! Size=%i should be %i\n", buffer2.GetSize(), lLen * 2 );
        PODOFO_RAISE_ERROR( ePdfError_TestFailed );
    }

    if( buffer1.GetSize() != lLen ) 
    {
        fprintf( stderr, "Buffer1 size seems to be modified\n");
        PODOFO_RAISE_ERROR( ePdfError_TestFailed );
    }
    
    if( strcmp( buffer1.GetBuffer(), pszTestString ) != 0 ) 
    {
        fprintf( stderr, "Buffer1 contents seem to be modified!\n" );
        PODOFO_RAISE_ERROR( ePdfError_TestFailed );
    }
        
    // large appends
    PdfBufferOutputStream streamLarge( &buffer1 );
    for( int i=0;i<100;i++ ) 
    {
        streamLarge.Write( pszTestString, lLen );
    }
    streamLarge.Close();

    if( buffer1.GetSize() != (lLen * 100 + lLen) ) 
    {
        fprintf( stderr, "Buffer1 size is wrong after 100 attaches: %i\n", buffer1.GetSize() );
        PODOFO_RAISE_ERROR( ePdfError_TestFailed );
    }
    
}

int main( int argc, char* argv[] )
{
    printf("Device Test\n");
    printf("==============\n");

    if( argc != 1 )
    {
        printf("Usage: DeviceTest\n");
        return 0;
    }

    try {
        printf("-> Testing PdfRefCountedBuffer...\n");
        RefCountedBufferTest();
    } catch( const PdfError & e ) {
        e.PrintErrorMsg();
        return e.GetError();
    }

    printf("All tests successfull!\n");

    return 0;
}
