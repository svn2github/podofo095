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

#ifndef PODOFOBROWSER_H
#define PODOFOBROWSER_H

#include "podofobrowserbase.h"

#include <podofo.h>

#include <qstring.h>

class QListViewItem;

class PoDoFoBrowser: public PoDoFoBrowserBase
{
    Q_OBJECT

 public:
    PoDoFoBrowser();
    ~PoDoFoBrowser();

 private:
    void fileNew();

    void fileOpen();
    void fileOpen( const QString & filename );

    bool fileSave();
    bool fileSave( const QString & filename );
    bool fileSaveAs();

    void fileExit();

    void editInsertKey();
    void editInsertObject();

    void editDeleteKey();
    void editDeleteObject();

    void toolsToHex();
    void toolsFromHex();

    void loadConfig();
    void saveConfig();

    void parseCmdLineArgs();
    void clear();

    void podofoError( const PoDoFo::PdfError & eCode );

    bool saveObject();

    void streamChanged( PoDoFo::PdfObject* );

    void helpAbout();

 private slots:
    void objectChanged( QListViewItem* );
    void slotImportStream();
    void slotExportStream();

 private:
    void loadObjects();
    bool trySave();

 private:

    QListViewItem*        m_lastItem;
    QString               m_filename;
    PoDoFo::PdfParser*    m_parser;
    PoDoFo::PdfWriter*    m_writer;
    PoDoFo::PdfObject*    m_pCurObject;

    bool                  m_bEditableStream;
    bool                  m_bChanged;
};


#endif
