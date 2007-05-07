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

#ifndef _PDF_OUTPUT_STREAM_H_
#define _PDF_OUTPUT_STREAM_H_

#include "PdfDefines.h"

namespace PoDoFo {

#define INITIAL_SIZE 4096

class PdfOutputDevice;

/** An interface for writing blocks of data to 
 *  a data source.
 */     
class PODOFO_API PdfOutputStream {
 public:

    virtual ~PdfOutputStream() { };

    /** Write data to the output stream
     *  
     *  \param pBuffer the data is read from this buffer
     *  \param lLen    the size of the buffer 
     *
     *  \returns the number of bytes written, -1 if an error ocurred
     */
    virtual long Write( const char* pBuffer, long lLen ) = 0;

    /** Close the PdfOutputStream.
     *  This method may throw exceptions and has to be called 
     *  before the descructor to end writing.
     *
     *  No more data may be written to the output device
     *  after calling close.
     */
    virtual void Close() = 0;
};

/** An output stream that writes data to a file
 */
class PODOFO_API PdfFileOutputStream : public PdfOutputStream {
 public:
    
    /** Open a file for writing data
     *  
     *  \param pszFilename the filename of the file to read
     */
    PdfFileOutputStream( const char* pszFilename );

    /** Write data to the output stream
     *  
     *  \param pBuffer the data is read from this buffer
     *  \param lLen    the size of the buffer 
     *
     *  \returns the number of bytes written, -1 if an error ocurred
     */
    virtual long Write( const char* pBuffer, long lLen );

    /** Close the PdfOutputStream.
     *  This method may throw exceptions and has to be called 
     *  before the descructor to end writing.
     *
     *  No more data may be written to the output device
     *  after calling close.
     */
    virtual void Close();

 private:
    FILE* m_hFile;
};

/** An output stream that writes data to a memory buffer
 *  If the buffer is to small, it will be enlarged automatically.
 */
class PODOFO_API PdfMemoryOutputStream : public PdfOutputStream {
 public:
    
    /** 
     *  Construct a new PdfMemoryOutputStream
     *  \param lInitial initial size of the buffer
     */
    PdfMemoryOutputStream( long lInitial = INITIAL_SIZE);
    ~PdfMemoryOutputStream();

    /** Write data to the output stream
     *  
     *  \param pBuffer the data is read from this buffer
     *  \param lLen    the size of the buffer 
     *
     *  \returns the number of bytes written, -1 if an error ocurred
     */
    virtual long Write( const char* pBuffer, long lLen );

    /** Close the PdfOutputStream.
     *  This method may throw exceptions and has to be called 
     *  before the descructor to end writing.
     *
     *  No more data may be written to the output device
     *  after calling close.
     */
    virtual void Close() { }

    /** \returns the length of the written data
     */
    inline unsigned long GetLength() const;

    /**
     *  \returns a handle to the internal buffer.
     *
     *  The internal buffer is now owned by the caller
     *  and will not be deleted by PdfMemoryOutputStream.
     *  Further calls to write are not allowed.
     *
     *  The caller has to free the returned malloc'ed buffer!
     */
    inline char* TakeBuffer();

 private:
    char* m_pBuffer;
    char* m_pCur;

    long  m_lLen;
    long  m_lSize;
};

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline unsigned long PdfMemoryOutputStream::GetLength() const
{
    return m_lLen;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline char* PdfMemoryOutputStream::TakeBuffer()
{
    char* pBuffer = m_pBuffer;
    m_pBuffer = NULL;
    return pBuffer;
}

/** An output stream that writes to a PdfOutputDevice
 */
class PODOFO_API PdfDeviceOutputStream : public PdfOutputStream {
 public:
    
    /** 
     *  Write to an already opened input device
     * 
     *  \param pDevice an output device
     */
    PdfDeviceOutputStream( PdfOutputDevice* pDevice );

    /** Write data to the output stream
     *  
     *  \param pBuffer the data is read from this buffer
     *  \param lLen    the size of the buffer 
     *
     *  \returns the number of bytes written, -1 if an error ocurred
     */
    virtual long Write( const char* pBuffer, long lLen );

    /** Close the PdfOutputStream.
     *  This method may throw exceptions and has to be called 
     *  before the descructor to end writing.
     *
     *  No more data may be written to the output device
     *  after calling close.
     */
    virtual void Close() {}

 private:
    PdfOutputDevice* m_pDevice;
};

};

#endif // _PDF_OUTPUT_STREAM_H_