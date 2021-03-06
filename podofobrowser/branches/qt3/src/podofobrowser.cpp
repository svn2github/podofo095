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


#include "podofobrowser.h"
#include "pdflistviewitem.h"
#include "podofoaboutdlg.h"

#include <qapplication.h>
#include <qcursor.h>
#include <qfiledialog.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qprogressdialog.h> 
#include <qsettings.h>
#include <qsplitter.h>
#include <qstatusbar.h>
#include <qtable.h>
#include <qtextedit.h>

using namespace PoDoFo;

class ObjectsComperator { 
public:
    ObjectsComperator( unsigned long lObject, unsigned long lGeneration )
        {
            m_lObject     = lObject;
            m_lGeneration = lGeneration;
        }
    
    bool operator()(const PdfObject* p1) { 
        return (p1->Reference().ObjectNumber() == m_lObject && p1->Reference().GenerationNumber() == m_lGeneration );
    }
private:
    unsigned long m_lObject;
    unsigned long m_lGeneration;
};

PoDoFoBrowser::PoDoFoBrowser()
    : PoDoFoBrowserBase( 0, "PoDoFoBrowser", WDestructiveClose ), m_pDocument( NULL )
{
    clear();

    listObjects->setSorting( -1 );

    connect( listObjects, SIGNAL( selectionChanged( QListViewItem* ) ), this, SLOT( objectChanged( QListViewItem* ) ) );
    connect( buttonImport, SIGNAL( clicked() ), this, SLOT( slotImportStream() ) );
    connect( buttonExport, SIGNAL( clicked() ), this, SLOT( slotExportStream() ) );
    connect( tableKeys, SIGNAL( valueChanged(int,int) ), this, SLOT( slotTableChanged() ) );

    show();
    statusBar()->message( tr("Ready"), 2000 );

    loadConfig();
    parseCmdLineArgs();
}

PoDoFoBrowser::~PoDoFoBrowser()
{
    saveConfig();

    delete m_pDocument;
}

void PoDoFoBrowser::loadConfig()
{
    int w,h;

    QValueList<int> list;
    QSettings       settings;

    settings.setPath( "podofo.sf.net", "podofobrowser" );

    w = settings.readNumEntry("/geometry/width", width() );
    h = settings.readNumEntry("/geometry/height", height() );

    list << width()/3;
    list << (width()/3 * 2);

    splitter2->setSizes( list );
    splitter3->setSizes( list );

    this->resize( w, h );
}

void PoDoFoBrowser::saveConfig()
{
    QSettings settings;

    settings.setPath( "podofo.sf.net", "podofobrowser" );

    settings.writeEntry("/geometry/width", width() );
    settings.writeEntry("/geometry/height", height() );
}

void PoDoFoBrowser::parseCmdLineArgs()
{
    if( qApp->argc() >= 2 )
    {
        fileOpen( QString( qApp->argv()[1] ) );
    }
}

void PoDoFoBrowser::clear()
{
    m_filename = QString::null;
    setCaption( "PoDoFoBrowser" );
    
    delete m_pDocument;

    m_pCurObject      = NULL;
    m_lastItem        = NULL;
    m_bEditableStream = false;
    m_bChanged        = false;
    m_pDocument       = NULL;
    m_bObjectChanged  = false;

    listObjects->clear();
    tableKeys->setNumRows( 0 );
    textStream->clear();
}

void PoDoFoBrowser::fileNew()
{
   PdfError         eCode;

   if( !trySave() ) 
       return;

   this->clear();

   m_pDocument = new PdfDocument();

   loadObjects();
}

void PoDoFoBrowser::fileOpen( const QString & filename )
{
    QStringList      lst;

    clear();

    QApplication::setOverrideCursor( Qt::WaitCursor );


    try {
        m_pDocument = new PdfDocument( filename.latin1() );
    } catch( PdfError & e ) {
        QApplication::restoreOverrideCursor();
        podofoError( e );
        return;
    }
    
    m_filename = filename;
    setCaption( m_filename );

    loadObjects();

    QApplication::restoreOverrideCursor();
    statusBar()->message(  QString( tr("Loaded file %1 successfully") ).arg( filename ), 2000 );
}

bool PoDoFoBrowser::fileSave( const QString & filename )
{
    PdfError         eCode;

    if( !saveObject() ) 
    {
        if( m_lastItem ) 
        {
            listObjects->blockSignals( true );
            listObjects->setCurrentItem( m_lastItem );
            listObjects->blockSignals( false );
        }
        return false;
    }

    QApplication::setOverrideCursor( Qt::WaitCursor );
    loadAllObjects();

    try {
        m_pDocument->Write( filename.latin1() );
    } catch( PdfError & e ) {
        QApplication::restoreOverrideCursor();
        podofoError( e );
        return false;
    }

    QApplication::restoreOverrideCursor();
    statusBar()->message(  QString( tr("Wrote file %1 successfully") ).arg( filename ), 2000 );

    m_filename = filename;
    m_bChanged = false;
    setCaption( m_filename );

    return true;
}

void PoDoFoBrowser::objectChanged( QListViewItem* item )
{
    PdfError         eCode;
    std::string      str;
    int              i      = 0;
    TCIKeyMap        it;
    PdfObject*       object = static_cast<PdfListViewItem*>(item)->object();
    
    if( !saveObject() ) 
    {
        if( m_lastItem ) 
        {
            listObjects->blockSignals( true );
            listObjects->setCurrentItem( m_lastItem );
            listObjects->blockSignals( false );
        }
        return;
    }
    
    m_pCurObject     = object;
    m_bObjectChanged = false;

    dynamic_cast<PdfListViewItem*>(item)->init();

    if( object->IsDictionary() )
    {
        tableKeys->setNumRows( object->GetDictionary().GetKeys().size() );
        tableKeys->setNumCols( 2 );
        tableKeys->horizontalHeader()->setLabel( 0, tr( "Key" ) );
        tableKeys->horizontalHeader()->setLabel( 1, tr( "Value" ) );

        it = object->GetDictionary().GetKeys().begin();
        while( it != object->GetDictionary().GetKeys().end() )
        {
            if( !(*it).second->IsDictionary() )
            {
                try {
                    (*it).second->ToString( str );
                } catch( PdfError & e ) {
                    podofoError( e );
                    break;
                }
                
                tableKeys->setText( i, 0, QString( (*it).first.GetName() ) );
                tableKeys->setText( i, 1, QString( str ) );
            }

            ++i;
            ++it;
        }
        tableKeys->adjustColumn( 0 );
        tableKeys->adjustColumn( 1 );
    }
    else
    {
        try {
            object->ToString( str );
        } catch( PdfError & e ) {
            podofoError( e );
            return;
        }
        
        tableKeys->setNumRows( 1 );
        tableKeys->setNumCols( 1 );
        tableKeys->setText( 0, 0, QString( str ) );
        tableKeys->adjustColumn( 0 );
        tableKeys->horizontalHeader()->setLabel( 0, tr( "Value" ) );
    }

    streamChanged( object );

    m_lastItem = item;
}

void PoDoFoBrowser::streamChanged( PdfObject* object )
{
    bool             bErr   = false;
    char*            pBuf   = NULL;
    long             lLen   = 0;
    bool             bHas   = true;
    QByteArray       data;

    if( !object )
        return;

    try {
        bHas = object->HasStream();
    } catch( PdfError & e ) {
        podofoError( e );
    }

    if( bHas )
    {
        try {
            object->GetStream()->GetFilteredCopy( &pBuf, &lLen );
        } catch( PdfError & e ) {
            m_bEditableStream = false;
            statusBar()->message( tr("Cannot apply filters to this stream!"), 2000 );
            podofoError( e );
            bErr   = true;
            textStream->setText( "" );
        }

        if( !bErr )
        { 
            m_bEditableStream = (lLen == qstrlen( pBuf ));
            if( m_bEditableStream )
                statusBar()->message( tr("Stream contains binary data and is not shown completely!"), 2000 );
            
            for( int i=0; i<lLen; i++ )
                if( pBuf[i] == 0 ) 
                    pBuf[i] = 127;

            data.duplicate( pBuf, lLen );
            free( pBuf );
            textStream->setText( QString( data ) );
        }
        labelStream->setText( m_bEditableStream ? tr("This stream can be edited.") : tr("This stream is readonly because of binary data contained in it.") );

        buttonImport->setEnabled( true );
        buttonExport->setEnabled( true );
    }
    else
    {
        m_bEditableStream = false;
        textStream->setText( tr("This object does not have a stream!" ) );
        labelStream->setText( QString::null );

        buttonImport->setEnabled( false );
        buttonExport->setEnabled( false );
    }
    
    textStream->setReadOnly( !m_bEditableStream );
}

void PoDoFoBrowser::fileOpen()
{
   if( !trySave() ) 
       return;

    QString filename = QFileDialog::getOpenFileName( QString::null, tr("PDF File (*.pdf)"), this );
    if( !filename.isNull() )
        fileOpen( filename );
}

bool PoDoFoBrowser::fileSave()
{
    if( m_filename.isEmpty() )
        return fileSaveAs();
    else
        return fileSave( m_filename );
}

bool PoDoFoBrowser::fileSaveAs()
{
    QString filename = QFileDialog::getSaveFileName( QString::null, tr("PDF File (*.pdf)"), this );
    if( !filename.isNull() )
        return fileSave( filename );
    else
        return false;
}

void PoDoFoBrowser::fileExit()
{
   if( !trySave() ) 
       return;

    this->close();
}

void PoDoFoBrowser::podofoError( const PdfError & eCode ) 
{
    TCIDequeErrorInfo it = eCode.GetCallstack().begin();
    const char* pszMsg   = PdfError::ErrorMessage( eCode.GetError() );
    const char* pszName  = PdfError::ErrorName( eCode.GetError() );

    int         i        = 0;

    QString msg = QString( "PoDoFoBrowser encounter an error.\nError: %1 %2\n%3\n" ).arg( eCode.GetError() ).arg( pszName ? pszName : "" ).arg( pszMsg ? pszMsg : "" );

    if( eCode.GetCallstack().size() )
        msg += "Callstack:\n";

    while( it != eCode.GetCallstack().end() )
    {
        if( !(*it).GetFilename().empty() )
            msg += QString("\t#%1 Error Source: %2:%3\n").arg( i ).arg( (*it).GetFilename().c_str() ).arg( (*it).GetLine() );

        if( !(*it).GetInformation().empty() )
            msg += QString("\t\tInformation: %1\n").arg( (*it).GetInformation().c_str() );
        
        ++i;
        ++it;
    }
    
    QMessageBox::warning( this, tr("Error"), msg );
}

bool PoDoFoBrowser::saveObject()
{
    PdfError    eCode;
    PdfVariant  var;
    PdfName     name;
    int         i;
    const char* pszText;

    // Check if there is a current object to save
    if( !m_pCurObject || !m_bObjectChanged )
        return true;

    if( m_pCurObject->IsDictionary() )
    {
        // first check wether all keys are valid
        for( i=0;i<tableKeys->numRows();i++ )
        {
            pszText = tableKeys->text( i, 0 ).latin1();
            name    = PdfName( pszText );
            if( name.GetLength() == 0 )
            {
                QMessageBox::warning( this, tr("Error"), QString("Error: %1 is no valid name.").arg( pszText ) );
                return false;
            }


            pszText = tableKeys->text( i, 1 ).latin1();

            try {
                var.Parse( pszText );
            } catch( PdfError & e ) {
                QString msg = QString("\"%1\" is no valid PDF datatype.\n").arg( pszText );
                e.SetErrorInformation( msg.latin1() );
                podofoError( e );
                return false;
            }
        }

        // clear the key map
        m_pCurObject->GetDictionary().Clear();

        // first check wether all keys are valid
        for( i=0;i<tableKeys->numRows();i++ )
        {
            var.Parse( tableKeys->text( i, 1 ).latin1() );
            m_pCurObject->GetDictionary().AddKey( PdfName( tableKeys->text( i, 0 ).latin1() ), var );
        }
    }
    else
    {
        pszText = tableKeys->text( 0, 0 ).latin1();
        try {
            var.Parse( pszText );
        } catch ( PdfError & e ) {
            podofoError( eCode );
            return false;
        }
        
        *m_pCurObject = var;
    }

    if( m_bEditableStream && textStream->isModified() ) 
    {
        m_pCurObject->GetDictionary().RemoveKey( PdfName( "Filter" ) );
        m_pCurObject->GetStream()->Set( textStream->text().latin1() );
        m_pCurObject->FlateCompressStream();
        statusBar()->message( tr("Stream data saved"), 2000 );
    }

    m_bChanged       = true;
    m_bObjectChanged = false;

    return true;
}

void PoDoFoBrowser::slotImportStream()
{
    PdfError eCode;
    char*    pBuf     = NULL;
    long     lLen     = 0;
    QString  filename;
    QFile    file;

    if( !m_pCurObject )
        return; 

    filename = QFileDialog::getOpenFileName( QString::null, tr("File (*)"), this );
    if( filename.isNull() )
        return;
    
    file.setName( filename );
    if( !file.open( IO_ReadOnly ) )
    {
        QMessageBox::critical( this, tr("Error"), QString( tr("Cannot open file %1 for reading.") ).arg( filename ) );
        return;
    }

    pBuf = (char*)malloc( file.size() * sizeof(char) );
    if( !pBuf )
    {
        QMessageBox::critical( this, tr("Error"), tr("Not enough memory to import this stream.") );
        file.close();
        return;
    }

    lLen = file.readBlock( pBuf, file.size() );
    file.close();

    try {
        m_pCurObject->GetStream()->Set( pBuf, lLen );
    } catch( PdfError & e ) {
        podofoError( e );
        return;
    }

    m_bObjectChanged = true;;
    streamChanged( m_pCurObject );
    statusBar()->message( QString( tr("Stream imported from %1") ).arg( filename ), 2000 );    
}

void PoDoFoBrowser::slotExportStream()
{
    PdfError eCode;
    char*    pBuf     = NULL;
    long     lLen     = 0;
    QString  filename;
    QFile    file;

    if( !m_pCurObject )
        return; 

    filename = QFileDialog::getSaveFileName( QString::null, tr("File (*)"), this );
    if( filename.isNull() )
        return;

    try {
        m_pCurObject->GetStream()->GetFilteredCopy( &pBuf, &lLen );    
    } catch( PdfError & e ) {
        podofoError( e );
        return;
    }

    file.setName( filename );
    if( !file.open( IO_WriteOnly ) )
    {
        QMessageBox::critical( this, tr("Error"), QString( tr("Cannot open file %1 for writing.") ).arg( filename ) );
        return;
    }
    file.writeBlock( pBuf, lLen );
    file.close();

    statusBar()->message( QString( tr("Stream exported to %1") ).arg( filename ), 2000 );    
}

void PoDoFoBrowser::toolsToHex()
{
    PdfHexFilter   filter;

    char* pBuffer = NULL;
    long  lLen    = 0;
    QString text  = QInputDialog::getText( tr("PoDoFoBrowser"), tr("Please input a string:") );

    if( QString::null != text ) 
    {
        try {
            filter.Encode( text.latin1(), text.length(), &pBuffer, &lLen );
        } catch( PdfError & e ) {
            podofoError( e );
            return;
        }

        text.setLatin1( pBuffer, lLen );
        QMessageBox::information( this, tr("PoDoFoBrowser"), tr("The string converted to hex:<br>") + text );
    }
}

void PoDoFoBrowser::toolsFromHex()
{
    PdfHexFilter   filter;

    char* pBuffer = NULL;
    long  lLen    = 0;
    QString text  = QInputDialog::getText( tr("PoDoFoBrowser"), tr("Please input a hex string:") );

    if( QString::null != text ) 
    {
        try {
            filter.Decode( text.latin1(), text.length(), &pBuffer, &lLen );
        } catch( PdfError & e ) {
            podofoError( e );
            return;
        }

        text.setLatin1( pBuffer, lLen );
        QMessageBox::information( this, tr("PoDoFoBrowser"), tr("The string converted to ascii:<br>") + text );
    }
}

void PoDoFoBrowser::editInsertKey()
{
    if( m_pCurObject )
    {
        tableKeys->setNumRows( tableKeys->numRows() + 1 );
        tableKeys->setCurrentCell( tableKeys->numRows(), 0 );
        tableKeys->setFocus();

        m_bObjectChanged = true;
        m_bChanged       = true;
    }
}

void PoDoFoBrowser::editInsertObject()
{
    PdfObject* pObject;

    if( saveObject() )
    {
        pObject = m_pDocument->GetObjects().CreateObject();
        listObjects->setCurrentItem( new PdfListViewItem( listObjects, pObject ) );

        m_bObjectChanged = true;
        m_bChanged       = true;
    }
}

void PoDoFoBrowser::editDeleteKey()
{
    int cur = tableKeys->currentRow();

    if( !m_pCurObject )
        return;

    if( QMessageBox::question( this, tr("Delete"), QString( tr("Do you really want to delete the key '%1'?") ).arg( 
                               tableKeys->text( cur, 0 ) ), QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes ) 
    {
        tableKeys->removeRow( cur );
        m_bObjectChanged = true;
        m_bChanged       = true;
    }
}

void PoDoFoBrowser::editDeleteObject()
{
    QListViewItem* item;
    PdfObject*     pObj;
    
    item  = listObjects->currentItem();

    if( !m_pCurObject && item )
        return;

    if( listObjects->currentItem()->parent() )
    {
        QMessageBox::information( this, tr("Delete"), tr("Deleting child objects is not yet supported." ) );
        return;
    }

    if( QMessageBox::question( this, tr("Delete"), QString( tr("Do you really want to delete the object '%1'?") ).arg( 
                               m_pCurObject->Reference().ToString().c_str() ), QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes ) 
    {
        pObj = m_pDocument->GetObjects().RemoveObject( m_pCurObject->Reference() );
        if( pObj ) 
        {
            delete pObj;

            listObjects->takeItem( item );
            delete item;
        
            m_pCurObject     = NULL;

            objectChanged( listObjects->currentItem() );
        }

        m_bChanged       = true;
    }
}

void PoDoFoBrowser::loadObjects()
{
    TCIVecObjects       it;
    QProgressDialog     dlg( this );
    int                 i   = 0;

    dlg.setLabelText( QString( tr( "Reading %1 objects ...") ).arg( m_pDocument->GetObjects().size() ) );
    dlg.setTotalSteps( m_pDocument->GetObjects().size() );
    dlg.show();

    it = m_pDocument->GetObjects().begin();
    listObjects->setUpdatesEnabled( false );
    listObjects->viewport()->setUpdatesEnabled( false );
    listObjects->clear();
    while( it != m_pDocument->GetObjects().end() )
    {
        new PdfListViewItem( listObjects, *it );

        dlg.setProgress( i );
        qApp->processEvents();

        if( dlg.wasCancelled() )
        {
            break;
        }

        ++i;
        ++it;
    }
    listObjects->viewport()->setUpdatesEnabled( true );
    listObjects->setUpdatesEnabled( true );
    listObjects->viewport()->update();
    listObjects->update();

    if( listObjects->firstChild() )
        objectChanged( listObjects->firstChild() );
}

void PoDoFoBrowser::helpAbout()
{
    PodofoAboutDlg* dlg = new PodofoAboutDlg( this );
    dlg->show();
}

bool PoDoFoBrowser::trySave() 
{
   if( m_bChanged ) 
   {
       int m = QMessageBox::question( this, tr("File changed"), QString( tr("The file %1 was changed. Do you want to save it?") ).arg( m_filename ), 
                                  QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel );
       
       if( m == QMessageBox::Cancel )
           return false;
       else if( m == QMessageBox::Yes ) 
       {
           if( m_filename.isEmpty() )
               return fileSaveAs();
           else
               fileSave( m_filename );
       }
       else 
           return true;
   }

   return true;
}

void PoDoFoBrowser::slotTableChanged()
{
    m_bObjectChanged = true;
}

void PoDoFoBrowser::loadAllObjects()
{
    QListViewItemIterator it( listObjects );
    QProgressDialog       dlg( this );
    int                   i = 0;

    dlg.setLabelText( QString( tr( "Reading %1 objects ...") ).arg( listObjects->childCount() ) );
    dlg.setTotalSteps( listObjects->childCount() );
    dlg.show();

    while( it.current() ) 
    {
        if( dlg.wasCancelled() )
            return;

        dlg.setProgress( i );

        static_cast<PdfListViewItem*>(it.current())->init();    

        ++i;
        ++it;
    }
}
