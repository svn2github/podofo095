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

#ifndef _PDF_LIST_VIEW_ITEM_H_
#define _PDF_LIST_VIEW_ITEM_H_

#include "qlistview.h"

namespace PoDoFo {
    class PdfObject;
};

class PdfListViewItem : public QListViewItem {
 public:
    PdfListViewItem( QListView* parent, PoDoFo::PdfObject* object );
    PdfListViewItem( QListViewItem* parent, PoDoFo::PdfObject* object, const QString & key );
    ~PdfListViewItem();

    inline PoDoFo::PdfObject* object() const; 

    int compare( QListViewItem* i, int col, bool ascending ) const;

 private:
    void init();

 private:
    PoDoFo::PdfObject* m_pObject;
    QString            m_sName;
};

PoDoFo::PdfObject* PdfListViewItem::object() const
{
    return m_pObject;
}

#endif // _PDF_LIST_VIEW_ITEM_H_
