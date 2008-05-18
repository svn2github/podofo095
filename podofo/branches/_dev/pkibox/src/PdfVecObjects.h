/***************************************************************************
 *   Copyright (C) 2005 by Dominik Seichter                                *
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

#ifndef _PDF_VEC_OBJECTS_H_
#define _PDF_VEC_OBJECTS_H_

#include "PdfDefines.h"
#include "PdfReference.h"

#include <list>

namespace PoDoFo {

class PdfDocument;
class PdfObject;
class PdfStream;
class PdfVariant;

// slist would be better, but it is not support by default gcc :-(
typedef std::list<PdfReference>                  TPdfReferenceList;
typedef TPdfReferenceList::iterator              TIPdfReferenceList;
typedef TPdfReferenceList::const_iterator        TCIPdfReferenceList;

typedef std::set<PdfReference>                   TPdfReferenceSet;
typedef TPdfReferenceSet::iterator               TIPdfReferenceSet;
typedef TPdfReferenceSet::const_iterator         TCIPdfReferenceSet;

typedef std::list<PdfReference*>                 TReferencePointerList;
typedef TReferencePointerList::iterator          TIReferencePointerList;
typedef TReferencePointerList::const_iterator    TCIReferencePointerList;

typedef std::vector<TReferencePointerList  >     TVecReferencePointerList;
typedef TVecReferencePointerList::iterator       TIVecReferencePointerList;
typedef TVecReferencePointerList::const_iterator TCIVecReferencePointerList;

/*
typedef std::vector<PdfObject*>      TVecObjects;
typedef TVecObjects::iterator        TIVecObjects;
typedef TVecObjects::const_iterator  TCIVecObjects;
*/

typedef std::vector<PdfObject*>      TVecObjects;
typedef TVecObjects::iterator        TIVecObjects;
typedef TVecObjects::const_iterator  TCIVecObjects;



/** A STL vector of PdfObjects. I.e. a list of PdfObject classes.
 *  The PdfParser will read the PdfFile into memory and create 
 *  a PdfVecObjects of all dictionaries found in the PDF file.
 * 
 *  The PdfWriter class contrary creates a PdfVecObjects internally
 *  and writes it to a PDF file later with an appropriate table of 
 *  contents.
 *
 *  These class contains also advanced funtions for searching of PdfObject's
 *  in a PdfVecObject. 
 */
class PODOFO_API PdfVecObjects {
    friend class PdfWriter;

 public:
    // An incomplete set of container typedefs, just enough to handle
    // the begin() and end() methods we wrap from the internal vector.
    // TODO: proper wrapper iterator class.
    typedef TVecObjects::iterator iterator;
    typedef TVecObjects::const_iterator const_iterator;

    /** Every observer of PdfVecObjects has to implement this interface.
     */
    class PODOFO_API Observer {
        friend class PdfVecObjects;

    public:
        virtual ~Observer()
            {
            }

        virtual void WriteObject( const PdfObject* pObject ) = 0;
            
        /**
         * This method is called when the observed PdfVecObjects is delted. 
         *
         * No more method may be called on the observable
         * after this method was called on the observer.
         */
        virtual void ParentDestructed() = 0;
        
        /** Called whenever appending to a stream is started.
         *  \param pStream the stream object the user currently writes to.
         */
        virtual void BeginAppendStream( const PdfStream* pStream ) = 0;

        /** Called whenever appending to a stream has ended.
         *  \param pStream the stream object the user currently writes to.
         */
        virtual void EndAppendStream( const PdfStream* pStream ) = 0;

        virtual void Finish() = 0;
    };

    /** This class is used to implement stream factories in PoDoFo.
     */
    class PODOFO_API StreamFactory {
    public:
        virtual ~StreamFactory()
            {
            }
        
        /** Creates a stream object
         *
         *  \param pParent parent object
         *
         *  \returns a new stream object 
         */
        virtual PdfStream* CreateStream( PdfObject* pParent ) = 0;
    };

 private:
    typedef std::vector<Observer*>        TVecObservers;
    typedef TVecObservers::iterator       TIVecObservers;
    typedef TVecObservers::const_iterator TCIVecObservers;

 public:
    /** Default constuctor 
     */
    PdfVecObjects();

    virtual ~PdfVecObjects();

    /** \returns a pointer to a PdfDocument that is the 
     *           parent of this vector.
     *           Might be NULL if the vector has no parent.
     */
    inline PdfDocument* GetParentDocument() const;

    /** Sets a parent document of this vector
     *  \param pDocument the parent of this vector
     */
    inline void SetParentDocument( PdfDocument* pDocument );

    /** Enable/disable auto deletion.
     *  By default auto deletion is disabled.
     *
     *  \param bAutoDelete if true all objects will be deleted when the PdfVecObjects is 
     *         deleted.
     */
    inline void SetAutoDelete( bool bAutoDelete );

    /** 
     *  \returns if autodeletion is enabled and all objects will be deleted when the PdfVecObjects is 
     *           deleted.
     */
    inline bool AutoDelete() const;

    /** Removes all objects from the vector
     *  and resets it to the default state.
     *
     *  If SetAutoDelete is true all objects are deleted.
     *  All observers are removed from the vector.
     *
     *  \see SetAutoDelete
     *  \see AutoDelete
     */
    void Clear();

    /** 
     *  \returns the size of the internal vector
     */
    inline size_t GetSize() const;

    /**
     *  \returns the highest object number in the vector 
     */
    size_t GetObjectCount() const { return m_nObjectCount; }

    /** Finds the object with the given reference in m_vecOffsets 
     *  and returns a pointer to it if it is found.
     *  \param ref the object to be found
     *  \returns the found object or NULL if no object was found.
     */
    PdfObject* GetObject( const PdfReference & ref ) const;

    /** Finds the object with the given reference in m_vecOffsets 
     *  and returns the index to it.
     *  \param ref the object to be found
     *  \returns the found object or NULL if no object was found.
     */
    unsigned int GetIndex( const PdfReference & ref ) const;

    /** Remove the object with the given object and generation number from the list
     *  of objects.
     *  The object is returned if it was found. Otherwise NULL is returned.
     *  The caller has to delete the object by hisself.
     *
     *  \param ref the object to be found
     *  \param bMarkAsFree if true the removed object reference is marked as free object
     *                     you will always want to have this true
     *                     as invalid PDF files can be generated otherwise
     *  \returns The removed object.
     */
    PdfObject* RemoveObject( const PdfReference & ref, bool bMarkAsFree = true );

    /** Remove the object with the iterator it from the vector and return it
     *  \param it the object to remove
     *  \returns the removed object
     */
    PdfObject* RemoveObject( const TIVecObjects & it );

    /** Creates a new object and inserts it into the vector.
     *  This function assigns the next free object number to the PdfObject.
     *
     *  \param pszType optionall value of the /Type key of the object
     *  \returns PdfObject pointer to the new PdfObject
     */
    PdfObject* CreateObject( const char* pszType = NULL );

    /** Creates a new object (of type rVariants) and inserts it into the vector.
     *  This function assigns the next free object number to the PdfObject.
     *
     *  \param rVariant value of the PdfObject
     *  \returns PdfObject pointer to the new PdfObject
     */
    PdfObject* CreateObject( const PdfVariant & rVariant );

    /** Mark a reference as unused so that it can be reused for new objects.
     *  \param rReference the reference to reuse
     */
    void AddFreeObject( const PdfReference & rReference );

    /** \returns a list of free references in this vector
     */
    inline const TPdfReferenceList & GetFreeObjects() const;

    /** 
     *  Renumbers all objects according to there current position in the vector.
     *  All references remain intact.
     *  Warning! This function is _very_ calculation intensive.
     *
     *  \param pTrailer the trailer object
     *  \param pNotDelete a list of object which must not be deleted
     */
    void RenumberObjects( PdfObject* pTrailer, TPdfReferenceSet* pNotDelete = NULL );

    /** Insert a object into this vector.
     *  Overwritten from std::vector so that 
     *  m_bObjectCount can be increased for each object.
     * 
     *  \param pObj pointer to the object you want to insert
     */
    void push_back( PdfObject* pObj );

    /** 
     * Sort the objects in the vector based on their object and generation numbers
     */
    void Sort();

    /** 
     * Causes the internal vector to reserve space for size elements.
     * \param size reserve space for that much elements in the internal vector
     */
    inline void Reserve( size_t size );

    /** Get a set with all references of objects that the passed object
     *  depends on.
     *  \param pObj the object to calculate all dependencies for
     *  \param pList write the list of dependencies to this list
     *     
     */
    void GetObjectDependencies( const PdfObject* pObj, TPdfReferenceList* pList ) const;


    /** Attach a new observer
     *  \param pObserver to attach
     */
    inline void Attach( Observer* pObserver );
    
    /** Detach an observer.
     *
     *  \param pObserver observer to detach
     */
    void Detach( Observer* pObserver );

    /** Sets a StreamFactory which is used whenever CreateStream is called.
     *  
     *  \param pFactory a stream factory or NULL to reset to the default factory
     */
    inline void SetStreamFactory( StreamFactory* pFactory );

    /** Creates a stream object
     *  This method is a factory for PdfStream objects.
     *
     *  \param pParent parent object
     *
     *  \returns a new stream object 
     */
    PdfStream* CreateStream( PdfObject* pParent );

    /** Creates a stream object by copying an existing stream
     *
     *  \param rhs copy this stream
     *
     *  \returns a new stream object 
     */
    PdfStream* CreateStream( const PdfStream & rhs );

    /** Can be called to force objects to be written to disk.
     * 
     *  \param pObject a PdfObject that should be written to disk.
     */
    void WriteObject( PdfObject* pObject );

    /** Call whenever a document is finished
     */
    void Finish();

    /** Every stream implementation has to call this in BeginAppend
     *  \param pStream the stream object that is calling
     */
    void BeginAppendStream( const PdfStream* pStream );
    
    /** Every stream implementation has to call this in EndAppend
     *  \param pStream the stream object that is calling
     */
    void EndAppendStream( const PdfStream* pStream );

    /** Iterator pointing at the begining of the vector
     *  \returns beginning iterator
     */
    inline TIVecObjects begin();

    /** Iterator pointing at the begining of the vector
     *  \returns beginning iterator
     */
    inline TCIVecObjects begin() const;

    /** Iterator pointing at the end of the vector
     *  \returns ending iterator
     */
    inline TIVecObjects end();

    /** Iterator pointing at the end of the vector
     *  \returns ending iterator
     */
    inline TCIVecObjects end() const;

    inline PdfObject*& operator[](int index);

    /** Get the last object in the vector
     *  \returns the last object in the vector or NULL 
     *           if the vector is emtpy.
     */
    inline PdfObject* GetBack();
    
 private:    
    /** 
     * \returns the next free object reference
     */
    PdfReference GetNextFreeObject();

    /** 
     * Create a list of all references that point to the object
     * for each object in this vector.
     * \param pList write all references to this list
     */
    void BuildReferenceCountVector( TVecReferencePointerList* pList );
    void InsertReferencesIntoVector( const PdfObject* pObj, TVecReferencePointerList* pList );

    /** Assumes that the PdfVecObjects is sorted
     */
    void InsertOneReferenceIntoVector( const PdfObject* pObj, TVecReferencePointerList* pList );

    /** Delete all objects from the vector which do not have references to them selves
     *  \param pList must be a list created by BuildReferenceCountVector
     *  \param pTrailer must be the trailer object so that it is not deleted
     *  \param pNotDelete a list of object which must not be deleted
     *  \see BuildReferenceCountVector
     */
    void GarbageCollection( TVecReferencePointerList* pList, PdfObject* pTrailer, TPdfReferenceSet* pNotDelete = NULL );

 private:
    bool                m_bAutoDelete;
    size_t              m_nObjectCount;
    bool                m_bSorted;
    TVecObjects         m_vector;


    TVecObservers       m_vecObservers;
    TPdfReferenceList   m_lstFreeObjects;

    PdfDocument*        m_pDocument;

    StreamFactory*      m_pStreamFactory;
};


// -----------------------------------------------------
// 
// -----------------------------------------------------
inline size_t PdfVecObjects::GetSize() const
{
    return m_vector.size();
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline void PdfVecObjects::Reserve( size_t size )
{
    m_vector.reserve( size );
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline PdfDocument* PdfVecObjects::GetParentDocument() const
{
    return m_pDocument;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline void PdfVecObjects::SetParentDocument( PdfDocument* pDocument )
{
    m_pDocument = pDocument;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline void PdfVecObjects::SetAutoDelete( bool bAutoDelete ) 
{
    m_bAutoDelete = bAutoDelete;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline bool PdfVecObjects::AutoDelete() const
{
    return m_bAutoDelete;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline const TPdfReferenceList & PdfVecObjects::GetFreeObjects() const
{
    return m_lstFreeObjects;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline void PdfVecObjects::Attach( Observer* pObserver )
{
    m_vecObservers.push_back( pObserver );
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline void PdfVecObjects::SetStreamFactory( StreamFactory* pFactory )
{
    m_pStreamFactory = pFactory;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline TIVecObjects PdfVecObjects::begin()
{
    return m_vector.begin();
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline TCIVecObjects PdfVecObjects::begin() const
{
    return m_vector.begin();
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline TIVecObjects PdfVecObjects::end()
{
    return m_vector.end();
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline TCIVecObjects PdfVecObjects::end() const
{
    return m_vector.end();
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline PdfObject* PdfVecObjects::GetBack() 
{ 
    return m_vector.back(); 
}

inline PdfObject*& PdfVecObjects::operator[](int index) { return m_vector[index]; }

//inline PdfObject const * & PdfVecObjects::operator[](int index) const { return m_vector[index]; }



};

#endif // _PDF_VEC_OBJECTS_H_
