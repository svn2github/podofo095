/***************************************************************************
 *   Copyright (C) 2008 by Dominik Seichter, Craig Ringer                  *
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

#include "../PdfDefines.h"
#include "../PdfDefinesPrivate.h"

#if ! defined(PODOFO_MULTI_THREAD)
#error "Not a multi-thread build. PdfMutex_null.h should be used instead"
#endif

#if defined(_WIN32)
#error "win32 build. PdfMutex_win32.h should be used instead"
#endif

#include <pthread.h>
#include <errno.h>

namespace PoDoFo {
namespace Util {

/**
 * A platform independent reentrant mutex, pthread implementation.
 *  
 * PdfMutex is *NOT* part of PoDoFo's public API.
 *
 * This is the pthread implementation, which is
 * entirely inline.
 */
class PdfMutexImpl {
    pthread_mutex_t m_mutex;
  public:

    PdfMutexImpl();

    ~PdfMutexImpl();

    void Init( const pthread_mutexattr_t *attr );

    /**
     * Lock the mutex
     */
    void Lock();

    /**
     * Try locking the mutex. 
     *
     * \returns true if the mutex was locked
     * \returns false if the mutex is already locked
     *                by some other thread
     */
    bool TryLock();

    /**
     * Unlock the mutex
     */
    void UnLock();
};

}; // Util
}; // PoDoFo
