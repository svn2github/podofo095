/***************************************************************************
 *   Copyright (C) 2006 by Dominik Seichter                                *
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


/*
 * Include the standard headers for cout to write
 * some output to the console.
 */
#include <iostream>

/*
 * Now include all podofo header files, to have access
 * to all functions of podofo and so that you do not have
 * to care about the order of includes.
 *
 * You should always use podofo.h and not try to include
 * the required headers on your own.
 */
#include <podofo.h>

/*
 * All podofo classes are member of the PoDoFo namespace.
 */
using namespace PoDoFo;

void PrintHelp()
{
    std::cout << "This is a example application for the PoDoFo PDF library." << std::endl
              << "It creates a small PDF file containing the text >Hello World!<" << std::endl
              << "Please see http://podofo.sf.net for more information" << std::endl << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  examplehelloworld [outputfile.pdf]" << std::endl << std::endl;
}

PdfError HelloWorld( const char* pszFilename ) 
{
    /*
     * Most PoDoFo functions and classes return a PdfError object.
     * It contains information about an error and where it has occurred.
     */
    PdfError        eCode;
    /*
     * PdfSimpleWriter is the class that can actually write a PDF file.
     */
    PdfSimpleWriter writer;
    /*
     * This pointer will hold the page object later. 
     * PdfSimpleWriter can write several PdfPage's to a PDF file.
     */
    PdfPage* pPage;
    /*
     * PdfPainter is the class which is able to draw text and graphics
     * directly on a PdfPage object.
     */
    PdfPainter painter;
    /*
     * A PdfFont object is required to draw text on a PdfPage using a PdfPainter.
     * PoDoFo will find the font using fontconfig on your system and embedd truetype
     * fonts automatically in the PDF file.
     */     
    PdfFont* pFont;

    /*
     * A PdfSimpleWriter object has to be initialized before it can be used.
     *
     * A PdfError object is returned. The SAFE_OP macro checks if an error has occurred.
     * If an error has occured it returns the PdfError object (which has to be called eCode)
     * from the current function. Otherwise the program continues as normal.
     */
    SAFE_OP( writer.Init() );

    /*
     * The PdfSimpleWriter object is initialized and can be used to create new PdfPage objects.
     * The PdfPage object is owned by the PdfSimpleWriter and will also be deleted automatically
     * by the PdfSimpleWriter object.
     *
     * You have to pass only one argument, i.e. the page size of the page to create.
     * There are predefined enums for some common page sizes.
     */
    pPage = writer.CreatePage( PdfPage::CreateStadardPageSize( ePdfPageSize_A4 ) );

    /*
     * If the page cannot be created because of an error (e.g. ePdfError_OutOfMemory )
     * a NULL pointer is returned.
     * We check for a NULL pointer here and return an error core using the RAISE_ERROR macro.
     * The raise error macro initializes the PdfError object with a given error code and
     * the location in the file in which the error ocurred.
     */
    if( !pPage ) 
    {
        RAISE_ERROR( ePdfError_InvalidHandle );
    }

    /*
     * Set the page as drawing target for the PdfPainter.
     * Before the painter can draw, a page has to be set first.
     */
    painter.SetPage( pPage );

    /*
     * Create a PdfFont object using the font "Arial".
     * The font is found on the system using fontconfig and embedded into the
     * PDF file. If Arial is not available, a default font will be used.
     *
     * The created PdfFont will be deleted by PdfSimpleWriter.
     */
    pFont = writer.CreateFont( "Arial" );
    
    /*
     * If the PdfFont object cannot be allocated return an error.
     */
    if( !pFont )
    {
        RAISE_ERROR( ePdfError_InvalidHandle );
    }

    /*
     * Set the font size
     */
    pFont->SetFontSize( 18.0 );

    /*
     * Set the font as default font for drawing.
     * A font has to be set before you can draw text on
     * a PdfPainter.
     */
    painter.SetFont( pFont );

    /*
     * You could set a different color than black to draw
     * the text.
     *
     * SAFE_OP( painter.SetColor( 1.0, 0.0, 0.0 ) );
     */

    /*
     * Actually draw the line "Hello World!" on to the PdfPage at
     * the position 2cm,2cm from the top left corner.
     * 
     * The position specifies the start of the baseline of the text.
     *
     * All coordinates in PoDoFo are in 1/1000th mm.
     */
    SAFE_OP( painter.DrawText( 20 * 1000, 20 * 1000, "Hello World!" ) );

    /*
     * Set some additional information on the PDF file.
     */
    writer.SetDocumentCreator ( PdfString("examplahelloworld - A PoDoFo test application") );
    writer.SetDocumentAuthor  ( PdfString("Dominik Seichter") );
    writer.SetDocumentTitle   ( PdfString("Hello World") );
    writer.SetDocumentSubject ( PdfString("Testing the PoDoFo PDF Library") );
    writer.SetDocumentKeywords( PdfString("Test;PDF;Hello World;") );

    /*
     * The last step is to write the PDF file from memory to the harddisk.
     */
    SAFE_OP( writer.Write( pszFilename ) );

    return eCode;
}

int main( int argc, char* argv[] )
{
    /*
     * Most PoDoFo functions and classes return a PdfError object.
     * It contains information about an error and where it has occurred.
     */
    PdfError        eCode;

    /*
     * Check if a filename was passed as commandline argument.
     * If more than 1 argument or no argument is passed,
     * a help message is displayed and the example application
     * will quit.
     */
    if( argc != 2 )
    {
        PrintHelp();
        return -1;
    }

    /*
     * Call the drawing routing which will create a PDF file
     * with the filename of the output file as argument.
     * 
     * The function returns a PdfError object. 
     */
    eCode = HelloWorld( argv[1] );

    /*
     * We have to check if an error has occurred.
     * If yes we return and print an error message
     * to the commandline.
     */
    if( eCode.IsError() )
    {
        eCode.PrintErrorMsg();
        return eCode.Error();
    }

    /*
     * The PDF was created sucessfully.
     */
    std::cout << std::endl
              << "Created a PDF file containing the line \"Hello World!\": " << argv[1] << std::endl << std::endl;
    
    return 0;
}
