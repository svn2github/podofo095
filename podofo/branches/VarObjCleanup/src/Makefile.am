
AM_CPPFLAGS = -I/usr/include/fontconfig/ $(FREETYPE_CFLAGS)

#
# Build a shared library

lib_LTLIBRARIES = libpodofo.la
libpodofo_la_LIBADD = -lz $(FREETYPE_LIBS) -lfontconfig -ljpeg
libpodofo_la_LDFLAGS = -version-number @LT_VERSION_NUMBER@
#libpodofo_la_CXXFLAGS = -pg -O2 -g

includedir = $(prefix)/include/podofo/

include_HEADERS = podofo.h podofoapi.h PdfDefines.h PdfObject.h PdfParser.h \
	 PdfParserObject.h PdfWriter.h PdfVariant.h \
	PdfPage.h PdfPainter.h PdfFont.h PdfFontMetrics.h PdfImage.h PdfStream.h \
	PdfVecObjects.h PdfDate.h PdfAnnotation.h PdfRect.h PdfXObject.h \
	PdfCanvas.h PdfOutputDevice.h PdfAction.h PdfError.h PdfString.h PdfName.h \
	PdfFilter.h PdfReference.h PdfDocument.h PdfArray.h PdfDictionary.h \
	PdfDataType.h PdfElement.h PdfPainterMM.h PdfPagesTree.h \
	PdfRefCountedBuffer.h PdfOutlines.h PdfDestination.h PdfContents.h \
	PdfEncrypt.h PdfNamesTree.h PdfInputDevice.h PdfHintStream.h \
	PdfRefCountedInputDevice.h PdfInfo.h PdfFileSpec.h PdfExtGState.h \
	PdfTokenizer.h PdfFiltersPrivate.h PdfInputStream.h PdfOutputStream.h \
	PdfFontCache.h PdfImmediateWriter.h PdfXRef.h PdfXRefStream.h PdfData.h \
	PdfMemStream.h PdfFileStream.h PdfStreamedDocument.h PdfLocale.h

# PdfSimpleWriter.cpp # PdfSimpleWriter.h
libpodofo_la_SOURCES = PdfObject.cpp PdfParser.cpp PdfParserObject.cpp \
	PdfWriter.cpp PdfVariant.cpp \
	PdfPage.cpp PdfPainter.cpp PdfFont.cpp PdfFontMetrics.cpp PdfImage.cpp PdfStream.cpp \
	PdfVecObjects.cpp PdfDate.cpp PdfAnnotation.cpp PdfRect.cpp PdfXObject.cpp \
	PdfCanvas.cpp PdfOutputDevice.cpp PdfAction.cpp PdfError.cpp PdfString.cpp PdfName.cpp \
	PdfFilter.cpp PdfReference.cpp PdfDocument.cpp PdfArray.cpp PdfDictionary.cpp \
	PdfDataType.cpp PdfElement.cpp PdfPainterMM.cpp PdfPagesTree.cpp \
	PdfRefCountedBuffer.cpp PdfOutlines.cpp PdfDestination.cpp PdfContents.cpp \
	PdfEncrypt.cpp PdfNamesTree.cpp PdfInputDevice.cpp PdfHintStream.cpp \
	PdfRefCountedInputDevice.cpp PdfInfo.cpp PdfFileSpec.cpp PdfExtGState.cpp \
	PdfTokenizer.cpp PdfFiltersPrivate.cpp PdfInputStream.cpp PdfOutputStream.cpp \
	PdfFontCache.cpp PdfImmediateWriter.cpp PdfXRef.cpp PdfXRefStream.cpp PdfData.cpp \
	PdfMemStream.cpp PdfFileStream.cpp PdfStreamedDocument.cpp PdfLocale.cpp

# 
# Build a static library
# 

# noinst_LIBRARIES = libsigpdf.a
# #libsigpdf_a_LDFLAGS = -pg
# #libsigpdf_a_LIBADD = 
# libsigpdf_a_CXXFLAGS = -pg -O2 -g

# noinst_HEADERS = PdfDefines.h PdfObject.h PdfParser.h PdfParserObject.h \
# 	PdfParserBase.h PdfWriter.h PdfVariant.h PdfAlgorithm.h PdfSimpleWriter.h \
# 	PdfPage.h PdfPainter.h PdfFont.h PdfFontMetrics.h PdfImage.h PdfStream.h

# libsigpdf_a_SOURCES = PdfObject.cpp PdfParser.cpp PdfParserObject.cpp \
# 	PdfParserBase.cpp PdfWriter.cpp PdfVariant.cpp PdfAlgorithm.cpp PdfSimpleWriter.cpp \
# 	PdfPage.cpp PdfPainter.cpp PdfFont.cpp PdfFontMetrics.cpp PdfImage.cpp PdfStream.cpp


