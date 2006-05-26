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

#include <qapplication.h>
#include <qcursor.h>
#include <qfiledialog.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qsettings.h>
#include <qsplitter.h>
#include <qstatusbar.h>
#include <qtable.h>
#include <qtextedit.h>

using namespace PoDoFo;

class ObjectsComperator { 
public:
    ObjectsComperator( long lObject, long lGeneration )
        {
            m_lObject     = lObject;
            m_lGeneration = lGeneration;
        }
    
    bool operator()(const PdfObject* p1) { 
        return (p1->ObjectNumber() == m_lObject && p1->GenerationNumber() == m_lGeneration );
    }
private:
    long m_lObject;
    long m_lGeneration;
};

PoDoFoBrowser::PoDoFoBrowser()
   : PoDoFoBrowserBase( 0, "PoDoFoBrowser", WDestructiveClose )
{
    m_writer     = new PdfWriter();
    m_parser     = new PdfParser();

    clear();

    connect( listObjects, SIGNAL( selectionChanged( QListViewItem* ) ), this, SLOT( objectChanged( QListViewItem* ) ) );
    connect( buttonImport, SIGNAL( clicked() ), this, SLOT( slotImportStream() ) );
    connect( buttonExport, SIGNAL( clicked() ), this, SLOT( slotExportStream() ) );

    show();
    statusBar()->message( tr("Ready"), 2000 );

    loadConfig();
    parseCmdLineArgs();
}

PoDoFoBrowser::~PoDoFoBrowser()
{
    saveConfig();

    delete m_parser;
    delete m_writer;
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

    m_pCurObject      = NULL;
    m_lastItem        = NULL;
    m_bEditableStream = false;

    listObjects->clear();
    tableKeys->setNumRows( 0 );
    textStream->clear();
}

void PoDoFoBrowser::fileNew()
{
   PdfError         eCode;

   this->clear();
   m_writer->Init( false );
   loadObjects();

}

void PoDoFoBrowser::fileOpen( const QString & filename )
{
    PdfError         eCode;
    QStringList      lst;

    clear();

    QApplication::setOverrideCursor( Qt::WaitCursor );

    eCode = m_parser->Init( filename.latin1() );

    if( eCode.IsError() )
    {
        QApplication::restoreOverrideCursor();
        podofoError( eCode );
        return;
    }
    
    m_filename = filename;
    setCaption( m_filename );

    m_writer->Init( m_parser );
    loadObjects();

    QApplication::restoreOverrideCursor();
    statusBar()->message(  QString( tr("Loaded file %1 successfully") ).arg( filename ), 2000 );
}

void PoDoFoBrowser::fileSave( const QString & filename )
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
        return;
    }

    QApplication::setOverrideCursor( Qt::WaitCursor );

    eCode = m_writer->Write( filename.latin1() );
    if( eCode.IsError() ) 
    {
        QApplication::restoreOverrideCursor();
        podofoError( eCode );
        return;
    }

    QApplication::restoreOverrideCursor();
    statusBar()->message(  QString( tr("Wrote file %1 successfully") ).arg( filename ), 2000 );

    m_filename = filename;
    setCaption( m_filename );
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
    
    m_pCurObject = object;
    if( !object->HasSingleValue() )
    {
        tableKeys->setNumRows( object->GetKeys().size() );
        tableKeys->setNumCols( 2 );
        tableKeys->horizontalHeader()->setLabel( 0, tr( "Key" ) );
        tableKeys->horizontalHeader()->setLabel( 1, tr( "Value" ) );

        it = object->GetKeys().begin();
        while( it != object->GetKeys().end() )
        {
            eCode = (*it).second.ToString( str );
            if( eCode.IsError() ) 
            {
                podofoError( eCode );
                break;
            }
            
            tableKeys->setText( i, 0, QString( (*it).first.Name() ) );
            tableKeys->setText( i, 1, QString( str ) );
            
            ++i;
            ++it;
        }
        tableKeys->adjustColumn( 0 );
        tableKeys->adjustColumn( 1 );
    }
    else
    {
        eCode = object->GetSingleValueVariant().ToString( str );
        if( eCode.IsError() ) 
        {
            podofoError( eCode );
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
    PdfError         eCode;
    char*            pBuf   = NULL;
    long             lLen   = 0;
    QByteArray       data;

    if( !object )
        return;

    if( object->HasStream() )
    {
        eCode = object->Stream()->GetFilteredCopy( &pBuf, &lLen );
        qDebug("Reading: %p %li", pBuf, lLen );
        if( eCode.IsError() )
        {
            m_bEditableStream = false;
            statusBar()->message( tr("Cannot apply filters to this stream!"), 2000 );
            podofoError( eCode );
        }
        else
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
    QString filename = QFileDialog::getOpenFileName( QString::null, tr("PDF File (*.pdf)"), this );
    if( !filename.isNull() )
        fileOpen( filename );
}

void PoDoFoBrowser::fileSave()
{
    if( m_filename.isEmpty() )
        fileSaveAs();
    else
        fileSave( m_filename );
}

void PoDoFoBrowser::fileSaveAs()
{
    QString filename = QFileDialog::getSaveFileName( QString::null, tr("PDF File (*.pdf)"), this );
    if( !filename.isNull() )
        fileSave( filename );
}

void PoDoFoBrowser::fileExit()
{
    this->close();
}

void PoDoFoBrowser::podofoError( const PdfError & eCode ) 
{
    const char* pszMsg  = PdfError::ErrorMessage( eCode.Error() );
    const char* pszName = PdfError::ErrorName( eCode.Error() );

    QString msg = QString( "PoDoFoBrowser encounter an error.\nError: %1 %2\nError Description: %3\nError Source: %4:%5\n" 
        ).arg( eCode.Error() ).arg( pszName ? pszName : "" ).arg( pszMsg ).arg( PdfError::Filename() ).arg( PdfError::Line() );

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
    if( !m_pCurObject )
        return true;

    if( !m_pCurObject->HasSingleValue() )
    {
        // first check wether all keys are valid
        for( i=0;i<tableKeys->numRows();i++ )
        {
            pszText = tableKeys->text( i, 0 ).latin1();
            name    = PdfName( pszText );
            if( name.Length() == 0 )
            {
                QMessageBox::warning( this, tr("Error"), QString("Error: %1 is no valid name.").arg( pszText ) );
                return false;
            }


            pszText = tableKeys->text( i, 1 ).latin1();
            eCode = var.Init( pszText );

            if( eCode.IsError() )
            {
                podofoError( eCode );
                return false;
            }
        }

        // clear the key map
        m_pCurObject->ClearKeys();

        // first check wether all keys are valid
        for( i=0;i<tableKeys->numRows();i++ )
        {
            eCode = var.Init( tableKeys->text( i, 1 ).latin1() );
            m_pCurObject->AddKey( PdfName( tableKeys->text( i, 0 ) ), var );
        }
    }
    else
    {
        pszText = tableKeys->text( 0, 0 ).latin1();
        eCode = var.Init( pszText );
        
        if( eCode.IsError() )
        {
            podofoError( eCode );
            return false;
        }
        
        m_pCurObject->SetSingleValue( var );
    }

    if( m_bEditableStream && textStream->isModified() ) 
    {
        m_pCurObject->RemoveKey( PdfName( "Filter" ) );
        m_pCurObject->Stream()->Set( textStream->text().latin1() );
        m_pCurObject->FlateDecodeStream();
        statusBar()->message( tr("Stream data saved"), 2000 );
    }

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

    eCode = m_pCurObject->Stream()->Set( pBuf, lLen );
    if( eCode.IsError() )
    {
        podofoError( eCode );
        return;
    }

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

    eCode = m_pCurObject->Stream()->GetFilteredCopy( &pBuf, &lLen );    
    if( eCode.IsError() )
    {
        podofoError( eCode );
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
    PdfError       eCode;
    PdfHexFilter   filter;

    char* pBuffer = NULL;
    long  lLen    = 0;
    QString text  = QInputDialog::getText( tr("PoDoFoBrowser"), tr("Please input a string:") );

    if( QString::null != text ) 
    {
        eCode = filter.Encode( text.latin1(), text.length(), &pBuffer, &lLen );
        if( eCode.IsError() )
        {
            podofoError( eCode );
            return;
        }

        text.setLatin1( pBuffer, lLen );
        QMessageBox::information( this, tr("PoDoFoBrowser"), tr("The string converted to hex:<br>") + text );
    }
}

void PoDoFoBrowser::toolsFromHex()
{
    PdfError       eCode;
    PdfHexFilter   filter;

    char* pBuffer = NULL;
    long  lLen    = 0;
    QString text  = QInputDialog::getText( tr("PoDoFoBrowser"), tr("Please input a hex string:") );

    if( QString::null != text ) 
    {
        eCode = filter.Decode( text.latin1(), text.length(), &pBuffer, &lLen );
        if( eCode.IsError() )
        {
            podofoError( eCode );
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
    }
}

void PoDoFoBrowser::editInsertObject()
{
    PdfObject* pObject;

    if( saveObject() )
    {
        pObject = m_writer->CreateObject();
        listObjects->setCurrentItem( new PdfListViewItem( listObjects, pObject ) );
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
                               m_pCurObject->Reference().c_str() ), QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes ) 
    {
        pObj = m_writer->RemoveObject( m_pCurObject->ObjectNumber(), m_pCurObject->GenerationNumber() );
        if( pObj ) 
        {
            delete pObj;

            listObjects->takeItem( item );
            delete item;
        
            m_pCurObject = NULL;

            objectChanged( listObjects->currentItem() );
        }
    }
}

void PoDoFoBrowser::loadObjects()
{
    TCIVecObjects    it;

    it = m_writer->GetObjects().begin();
    listObjects->setUpdatesEnabled( false );
    while( it != m_writer->GetObjects().end() )
    {
        new PdfListViewItem( listObjects, *it );

        ++it;
    }
    listObjects->setUpdatesEnabled( true );

    if( listObjects->firstChild() )
        objectChanged( listObjects->firstChild() );
}

    /*

void PoDoFoBrowser::slotFindObject()
{
    PdfError      eCode;
    PdfVariant    var;
    long          lObj  = 0;
    long          lGen  = 0;
    TCIVecObjects it;


    // ignore empty texts
    if( m_find->currentText().isEmpty() )
        return;

    eCode = var.Init( m_find->currentText().latin1() );
    if( !eCode.IsError() )
    {
        if( var.GetDataType() == ePdfDataType_Reference )
            var.GetReference( &lObj, &lGen );
        else if( var.GetDataType() == ePdfDataType_Number )
            var.GetNumber( &lObj );

        it = std::find_if( m_parser->GetObjects().begin(), m_parser->GetObjects().end(), ObjectsComperator( lObj, lGen ) );
    }
    else
    {
        podofoError( eCode );
        return;
    }

    if( it == m_parser->GetObjects().end() )
    {
        m_pCurObject = NULL;
        QMessageBox::critical( this, tr("Object not found"), QString( tr("No object %1 was found!") ).arg( m_find->currentText() ) );
    }
    else
    {
        if( m_pCurObject == NULL || applyChangesToObject() )
        {
            m_pCurObject = (*it);
            updateCurrentItem();
        }
    }
}
    */

#if 0

void PoDoFoBrowser::updateCurrentItem()
{
    TCIVecObjects it;

    m_list->clear();

    if( m_pCurObject )
    {
        m_list->blockSignals( true );
        new PdfListViewItem( m_list, m_pCurObject );
        m_list->blockSignals( false );
        m_list->setSelected( m_list->firstChild(), true );

        /*
        m_prev->setEnabled( m_cur_it != m_parser->GetObjects().begin() );
        m_next->setEnabled( m_cur_it != it );
        */

        m_find->setCurrentText( QString( m_pCurObject->Reference() ) );
    }

    m_last_obj = NULL;
}

void PoDoFoBrowser::objectChanged( QListViewItem* item )
{
/*    
    PdfError         eCode;
    const PdfObject* object;
    TCIKeyMap        it;
    int              i     = 0;
    std::string      str;
    char*            pBuf;
    long             lLen;
    QByteArray       data;

    if( m_last_obj )
    {
        if( !applyChangesToObject( static_cast<PdfListViewItem*>(m_last_obj)->object() ) )
        {
            m_list->blockSignals( true );
            m_list->setCurrentItem( m_last_obj );
            m_list->blockSignals( false );
            return;
        }
    }

    m_table->setNumRows( 0 );
    m_edit->setText( QString::null );

    if( item && static_cast<PdfListViewItem*>(item) && static_cast<PdfListViewItem*>(item)->object() )
    {
        object = static_cast<PdfListViewItem*>(item)->object();
        it     = object->GetKeys().begin();
            
        m_table->setNumRows( object->GetKeys().size() );

        if( !object->HasSingleValue() )
        {
            m_stack->raiseWidget( m_table );

            while( it != object->GetKeys().end() )
            {
                eCode = (*it).second.ToString( str );
                if( eCode.IsError() ) 
                {
                    podofoError( eCode );
                    break;
                }
                
                m_table->setText( i, 0, QString( (*it).first ) );
                m_table->setText( i, 1, QString( str ) );
                
                ++i;
                ++it;
            }
        }
        else
        {
            m_stack->raiseWidget( m_sbox );

            m_single->setText( object->GetSingleValueString() );
        }

        if( object->HasStream() )
        {
            eCode = object->Stream()->GetFilteredCopy( &pBuf, &lLen );
            if( eCode.IsError() )
            {
                statusBar()->message( tr("Cannot apply filters to this stream!"), 2000 );
                podofoError( eCode );
            }
            else
            { 
                if( lLen != qstrlen( pBuf ) )
                    statusBar()->message( tr("Stream contains binary data and is not shown completely!"), 2000 );

                data.assign( pBuf, lLen );
                m_edit->setText( QString( data ) );
            }
        }
        else
            m_edit->setText( tr("This object does not have a stream!" ) );
    }
    
    m_last_obj = item;
*/
}

void PoDoFoBrowser::slotNext()
{
    /*
    if( applyChangesToObject() )
    {
        //++m_cur_it;
        updateCurrentItem();
    }
    */
}

void PoDoFoBrowser::slotPrev()
{
    /*
    if( applyChangesToObject() )
    {
        //--m_cur_it;
        updateCurrentItem();
    }
    */
}

bool PoDoFoBrowser::applyChangesToObject()
{
    /*
    if( m_filename.isEmpty() )
        return true;
    
    if( m_pCurObject == NULL )
        return false;
    else
        return applyChangesToObject( m_pCurObject );
    */
}

bool PoDoFoBrowser::applyChangesToObject( PdfObject* object )
{
    /*
    PdfError      eCode;
    PdfVariant    var;
    int           i;
    const char*   pszText;

    if( !object )
        return true;

    if( object->HasSingleValue() && m_single->isModified() )
    {
        eCode = var.Init( m_single->text().latin1() );
        if( !eCode.IsError() )
            object->SetSingleValue( m_single->text().latin1() );
        else
        {        
            podofoError( eCode );
            return false;
        }
    }
    else
    {
        // first check wether all keys are valid
        for( i=0;i<m_table->numRows();i++ )
        {
            pszText = m_table->text( i, 0 ).latin1();
            eCode = var.Init( pszText );

            if( !eCode.IsError() )
            {
                pszText = m_table->text( i, 1 ).latin1();
                eCode = var.Init( pszText );
            }
            else
            {
                podofoError( eCode );
                return false;
            }
        }

        // clear the key map
        object->ClearKeys();

        // first check wether all keys are valid
        for( i=0;i<m_table->numRows();i++ )
        {
            eCode = var.Init( m_table->text( i, 1 ).latin1() );
            object->AddKey( QString( m_table->text( i, 0 ) ), var );
        }
    }

    return true;
    */
}
#endif // 0
