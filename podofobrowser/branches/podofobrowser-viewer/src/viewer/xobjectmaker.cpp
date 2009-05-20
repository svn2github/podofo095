/***************************************************************************
 *   Copyright (C) 2009 by Pierre Marchand   *
 *   pierremarc@oep-h.com   *
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

#include "xobjectmaker.h"
#include "viewer/dispatcher.h"

XObjectMaker::XObjectMaker()
{
    supportedOps << QString::fromAscii("Do");
}

bool XObjectMaker::support(const QString& op) const
{
    return supportedOps.contains(op);
}

PdfContentIterator XObjectMaker::item(PdfContentIterator csIterator,  GraphicState& gState)
{
    // first find the XObject!
    if( !csIterator->operands.first().IsName() )
    {
        qDebug()<<"Error - Do called with no name";
        return ++csIterator;
    }

    PoDoFo::PdfName xobject("XObject");
    PoDoFo::PdfName xoName( csIterator->operands.first().GetName() );
    PoDoFo::PdfObject * candidate(gState.getResource(xoName , xobject));


    // at this point candidate is an XObject dictionary
    if(!candidate)
    {
        qDebug()<<"Error - Cannot get xobject" << xoName.GetEscapedName().c_str();
        return ++csIterator;
    }
    if(!candidate->IsDictionary())
    {
        qDebug()<<"Error - XObject not a dictionary" << xoName.GetEscapedName().c_str();
        return ++csIterator;
    }

    xoObj.append( candidate );
    PoDoFo::PdfName formName("Form");
    PoDoFo::PdfName imageName("Image");

    PoDoFo::PdfName subtype( xoObj.last()->GetIndirectKey(PoDoFo::PdfName("Subtype"))->GetName() );
    if(subtype == formName)
    {
        PdfContentIterator retIt = form(csIterator, gState);
        xoObj.removeLast();
        return retIt;
    }
    else if(subtype == imageName)
    {
        PdfContentIterator retIt = image(csIterator, gState);
        xoObj.removeLast();
        return retIt;
    }
    else
    {
        qDebug()<<"Subtype not supported"<<subtype.GetEscapedName().c_str();
        xoObj.removeLast();
        return ++csIterator;
    }

    return ++csIterator;

}

PdfContentIterator XObjectMaker::form(PdfContentIterator csIterator,  GraphicState& gState)
{
//static int deepCount = 0;
//++deepCount;

    QUuid gsUid( GraphicStateStack::getInstance()->lock() );
GraphicState storeState(gState);
    if(xoObj.last()->HasStream())
    {

        GraphicState xState;
        xState.scene = gState.scene;
        xState.itemList = gState.itemList;
        xState.document = gState.document;
        xState.page = gState.page;
        xState.resource = xoObj.last()->GetIndirectKey(PoDoFo::PdfName("Resources"));

        PoDoFo::PdfObject * matrixObj(xoObj.last()->GetIndirectKey(PoDoFo::PdfName("Matrix")));
        if(matrixObj && matrixObj->IsArray() && (matrixObj->GetArray().GetSize() == 6))
        {
            PoDoFo::PdfArray& mar(matrixObj->GetArray());
            double a(mar[0].GetReal());
            double b(mar[1].GetReal());
            double c(mar[2].GetReal());
            double d(mar[3].GetReal());
            double e(mar[4].GetReal());
            double f(mar[5].GetReal());

            xState.xm = QMatrix(a,b,c,d,e,f) * gState.cm * gState.xm;
        }
        else
            xState.xm = gState.cm * gState.xm;

//        QString dString(deepCount,QChar(43));
//        qDebug()<<dString<<"Xm"<<xState.xm<<gState.cm;

        PoDoFo::PdfMemoryOutputStream bufferStream ( 1 );


        const PoDoFo::PdfStream * const stream = xoObj.last()->GetStream();
        try
        {
            stream->GetFilteredCopy ( &bufferStream );
        }
        catch ( PoDoFo::PdfError & e )
        {
            qDebug()<<"AAAAAaaaaaarrrrrrrrrrgh";
            return ++csIterator;
        }

        bufferStream.Close();
        int bLen(bufferStream.GetLength());
        PoDoFo::PdfContentsTokenizer * tokenizer = new PoDoFo::PdfContentsTokenizer ( bufferStream.TakeBuffer(), bLen);

        const char*      pszToken = NULL;
        PoDoFo::PdfVariant       var;
        PoDoFo::EPdfContentsType eType;
        std::string      str;

        PdfContentStream contentStream;

        QList<PoDoFo::PdfVariant> stack;

        while ( tokenizer->ReadNext ( eType, pszToken, var ) )
        {
            if ( eType == PoDoFo::ePdfContentsType_Keyword )
            {
                contentStream << PdfOperation ( QString::fromAscii ( pszToken ), stack );
                stack.clear();
            }
            else if ( eType == PoDoFo::ePdfContentsType_Variant )
            {
                stack << var;
            }
            else
            {
                qWarning ( "Impossible; type must be keyword or variant" );
            }
        }
        delete tokenizer;
        PdfContentIterator it ( contentStream.begin() );
        Dispatcher *d(Dispatcher::getInstance());
        while ( it != contentStream.end() )
        {
            it = d->run(it, xState);
        }
    }
//    --deepCount;
    GraphicStateStack::getInstance()->unlock(gsUid);
    return ++csIterator;
}

PdfContentIterator XObjectMaker::image(PdfContentIterator csIterator,  GraphicState& gState)
{
        qDebug()<<"Images not supported yet";
        return ++csIterator;
}

