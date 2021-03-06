/***************************************************************************
 *   Copyright (C) 2011 by Dominik Seichter                                *
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

#include "PdfFontConfigWrapper.h"

#if defined(PODOFO_HAVE_FONTCONFIG)
#include <fontconfig/fontconfig.h>
#include "base/util/PdfMutexWrapper.h"
#endif

namespace PoDoFo {

#if defined(PODOFO_HAVE_FONTCONFIG)
Util::PdfMutex PdfFontConfigWrapper::m_FcMutex;
#endif


PdfFontConfigWrapper::PdfFontConfigWrapper()
    : m_pFontConfig( NULL )
{
#if defined(PODOFO_HAVE_FONTCONFIG)
    this->m_pFontConfig = new TRefCountedFontConfig();
    this->m_pFontConfig->m_lRefCount = 1;
    this->m_pFontConfig->m_bInitialized = false;
    this->m_pFontConfig->m_pFcConfig = NULL;
#endif
}

PdfFontConfigWrapper::PdfFontConfigWrapper(const PdfFontConfigWrapper & rhs)
{
    this->operator=(rhs);
}

PdfFontConfigWrapper::~PdfFontConfigWrapper()
{
    this->DerefBuffer();
}

const PdfFontConfigWrapper & PdfFontConfigWrapper::operator=(const PdfFontConfigWrapper & rhs)
{
    // Self assignment is a no-op
    if (this == &rhs)
        return rhs;

    DerefBuffer();

    this->m_pFontConfig = rhs.m_pFontConfig;
    if( m_pFontConfig )
    {
        this->m_pFontConfig->m_lRefCount++;
    }

    return *this;
}


void PdfFontConfigWrapper::DerefBuffer()
{
    if ( m_pFontConfig && !(--m_pFontConfig->m_lRefCount) )
    {
#if defined(PODOFO_HAVE_FONTCONFIG)
        if( this->m_pFontConfig->m_bInitialized )
        {
            Util::PdfMutexWrapper mutex(m_FcMutex);
            FcConfigDestroy( static_cast<FcConfig*>(m_pFontConfig->m_pFcConfig) );
        }
#endif

        delete m_pFontConfig;
    }

    // Whether or not it still exists, we no longer have anything to do with
    // the buffer we just released our claim on.
    m_pFontConfig = NULL;
}

void PdfFontConfigWrapper::InitializeFontConfig() 
{
#if defined(PODOFO_HAVE_FONTCONFIG)
    if( !this->m_pFontConfig->m_bInitialized )
    {
        Util::PdfMutexWrapper mutex(m_FcMutex);
        if( !this->m_pFontConfig->m_bInitialized ) 
        {
            this->m_pFontConfig->m_pFcConfig = static_cast<void*>(FcInitLoadConfigAndFonts());
            this->m_pFontConfig->m_bInitialized = true;
        }
    }
#endif
}

void* PdfFontConfigWrapper::GetFontConfig() 
{
    if( m_pFontConfig != NULL ) 
    {
        InitializeFontConfig();
        return m_pFontConfig->m_pFcConfig;
    } 
    else 
    {
        return NULL;
    }
}

#if defined(PODOFO_HAVE_FONTCONFIG)
Util::PdfMutex & PdfFontConfigWrapper::GetFontConfigMutex()
{
    return m_FcMutex;
}
#endif

};
