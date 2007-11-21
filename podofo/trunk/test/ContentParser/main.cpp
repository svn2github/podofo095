#include "podofo.h"
#include "../PdfTest.h"

#include <iostream>
#include <stack>
#include <algorithm>
#include <string>
#include <iomanip>

using namespace std;
using namespace PoDoFo;

static bool print_output = false;

void parse_contents( PdfContentsTokenizer* pTokenizer ) 
{
    const char*      pszToken = NULL;
    PdfVariant       var;
    EPdfContentsType eType;
    std::string      str;

    int numKeywords = 0;
    int numVariants = 0;

    std::stack<PdfVariant> stack;

    while( pTokenizer->ReadNext( eType, pszToken, var ) )
    {
        if( eType == ePdfContentsType_Keyword )
        {
            ++numKeywords;
            if (print_output) std::cout << "Keyword: " << pszToken << std::endl;

            // support 'l' and 'm' tokens
            if( strcmp( pszToken, "l" ) == 0 ) 
            {
                double dPosY = stack.top().GetReal();
                stack.pop();
                double dPosX = stack.top().GetReal();
                stack.pop();

                if(print_output) std::cout << "LineTo: " << dPosX << " " << dPosY << std::endl;
            }
            else if( strcmp( pszToken, "m" ) == 0 ) 
            {
                double dPosY = stack.top().GetReal();
                stack.pop();
                double dPosX = stack.top().GetReal();
                stack.pop();

                if(print_output) std::cout << "MoveTo: " << dPosX << " " << dPosY << std::endl;
            }
        }
        else if ( eType == ePdfContentsType_Variant )
        {
            ++numVariants;
            var.ToString( str );
            if(print_output) std::cout << "Variant: " << str << std::endl;
            stack.push( var );
        }
        else
        {
            // Impossible; type must be keyword or variant
            PODOFO_RAISE_ERROR( ePdfError_InternalLogic );
        }
    }
    cout << ' ' << setw(8) << numKeywords << "keywords, " << setw(8) << numVariants << " variants" << endl;
}

#if defined(HAVE_BOOST)
using namespace boost;
#include <boost/graph/depth_first_search.hpp>

#define PODOFO_FAIL(s) PODOFO_RAISE_ERROR_INFO( ePdfError_TestFailed, s )

/**
 * This visitor is invoked on the variant associated with each node entry
 * (arriving=true) and exit (arriving=false) to check that whatever the
 * tokenizer reads from the input stream matches what we expected to get based
 * on our graph.
 */
class NodeCheckVisitor
    : public boost::static_visitor<void>
{
    PdfContentsTokenizer * m_tok;
    mutable const char* m_pszToken;
    mutable PdfVariant m_var;
    mutable EPdfContentsType m_eType;
    bool m_arriving;
    int* m_numKW;
    int* m_numVar;
public:
    NodeCheckVisitor(PdfContentsTokenizer* tok, bool arriving, int* numKW, int* numVar)
        : m_tok(tok), m_arriving(arriving), m_numKW(numKW), m_numVar(numVar) { }
    void operator()(PdfContentsGraph::KWPair kp) const
    {
        // This is an internal node representing an opening/closing
        // operator pair for a context. We should get a keyword from the
        // tokenizer.
        m_tok->ReadNext( m_eType, m_pszToken, m_var );
        if ( m_eType != ePdfContentsType_Keyword )
            PODOFO_FAIL( "KWPair node: expected keyword from tokenizer, got variant" );
        const char* gotKw;
        if (m_arriving)
            gotKw = PdfContentsGraph::findKwById(kp.first).kwText;
        else
            gotKw = PdfContentsGraph::findKwById(kp.second).kwText;
        if ( strcmp(gotKw, m_pszToken) != 0 )
            PODOFO_FAIL( "KWPair node: Got wrong token" );
        ++(*m_numKW);
    }
    void operator()(PdfContentStreamKeyword op) const
    {
        if ( m_arriving || op == KW_RootNode ) return;
        m_tok->ReadNext( m_eType, m_pszToken, m_var );
        if ( m_eType != ePdfContentsType_Keyword )
            PODOFO_FAIL( "Keyword id node: expected keyword from tokenizer, got variant" );
        if ( strcmp(PdfContentsGraph::findKwById(op).kwText, m_pszToken) != 0 )
            PODOFO_FAIL( "Keyword id node: Got wrong token" );
        ++(*m_numKW);
    }
    void operator()(const string& str) const
    {
        if ( m_arriving ) return;
        m_tok->ReadNext( m_eType, m_pszToken, m_var );
        if ( m_eType != ePdfContentsType_Keyword )
            PODOFO_FAIL( "Keyword str node: expected keyword from tokenizer, got variant" );
        if ( str != m_pszToken )
            PODOFO_FAIL( "Keyword str node: Got wrong token" );
        ++(*m_numKW);
    }
    void operator()(const PdfVariant& var) const
    {
        if ( m_arriving ) return;
        m_tok->ReadNext( m_eType, m_pszToken, m_var );
        if ( m_eType != ePdfContentsType_Variant )
            PODOFO_FAIL( "Value node: expected variant from tokenizer, got keyword" );
        if ( !(var == m_var) )
            PODOFO_FAIL( "Value node: Value mismatch" );
        ++(*m_numVar);
    }
};

template<typename EV, bool Arriving>
class CheckVertexVisitor
{
    PdfContentsTokenizer* m_tok;
    int* m_numKW;
    int* m_numVar;
    NodeCheckVisitor nodeVis;
public:
    CheckVertexVisitor( PdfContentsTokenizer& tok, int& numKW, int& numVar )
        : m_tok(&tok), m_numKW(&numKW), m_numVar(&numVar),
          nodeVis( NodeCheckVisitor(m_tok, Arriving, m_numKW, m_numVar) )
    { }
    typedef EV event_filter;
    void operator()(const PdfContentsGraph::Vertex & v,
                    const PdfContentsGraph::Graph & g)
    {
        boost::apply_visitor( nodeVis, g[v] );
    }
};

pair<int,int> CheckGraph(PdfContentsTokenizer& tok, PdfContentsGraph& g)
{
    int numKW = 0, numVar = 0;
    typedef pair<CheckVertexVisitor<on_discover_vertex,true>,CheckVertexVisitor<on_finish_vertex,false> > EVList;
    dfs_visitor<EVList> vis = make_dfs_visitor(
            EVList( CheckVertexVisitor<on_discover_vertex,true>(tok,numKW,numVar),
                    CheckVertexVisitor<on_finish_vertex,false>(tok,numKW,numVar) ) );
    depth_first_search( g.GetGraph(), visitor(vis));
    return pair<int,int>(numKW,numVar);
}

void parse_page_graph( PdfMemDocument*, PdfPage* pPage )
{
    PdfContentsTokenizer tokenizer( pPage );
    PdfContentsGraph g( tokenizer );

    // Using another instance of the tokenizer, traverse the graph and
    // compare what we find in the graph at each node to what we get from the
    // tokenizer. If the graph read, construction, and traverse are correct the
    // results should be identical.
    PdfContentsTokenizer checkTokenizer( pPage );
    pair<int,int> counts = CheckGraph(checkTokenizer, g);
    cout << ' ' << setw(8) << counts.first << " keywords, " << setw(8) << counts.second << " variants";
    // The above will either execute silently (a pass) or throw (a fail).

    // Write to stderr for test view
    if(print_output) g.WriteToStdErr();
}
#endif

void parse_page( PdfMemDocument*, PdfPage* pPage )
{
    PdfContentsTokenizer tokenizer( pPage );
    parse_contents( &tokenizer );
}

void usage()
{
    printf("Usage: ContentParser [-g] [-a] [-p] input_filename\n");
    printf("       -g   Use PdfContentsGraph\n");
    printf("       -a   Process all pages of input, not just first\n");
    printf("       -p   Print parsed content stream to stdout\n");
}

int main( int argc, char* argv[] ) 
{
    bool use_graph = false, all_pages = false;
    string inputFileName;
    ++argv;
    --argc;
    while (argc)
    {
        if( argv[0][0] == '-' )
        {
            // Single character flag
            switch( argv[0][1] )
            {
                case 'g':
                    // Use PdfContentsGraph
                    use_graph = true;
                    break;
                case 'a':
                    // Process all pages, not just first page
                    all_pages = true;
                    break;
                case 'p':
                    // Print output, rather than parsing & checking
                    // silently.
                    print_output = true;
                    break;
                default:
                    usage();
                    return 1;
            }
        }
        else
        {
            // Input filename
            if (inputFileName.empty())
            {
                inputFileName = argv[0];
            }
            else
            {
                usage();
                return 1;
            }
        }
        ++argv;
        --argc;
    }

    if (inputFileName.empty())
    {
        usage();
        return 1;
    }

    try
    {
        PdfMemDocument doc( inputFileName.c_str() );
        if( !doc.GetPageCount() )
        {
            std::cerr << "This document contains no page!" << std::endl;
            return 1;
        }

        int toPage = all_pages ? doc.GetPageCount() : 1 ;
        for ( int i = 0; i < toPage; ++i )
        {
            cout << "Processing page " << setw(6) << (i+1) << "...";
            PdfPage* page = doc.GetPage( i );
            PODOFO_RAISE_LOGIC_IF( !page, "Got null page pointer within valid page range" );

            if (!use_graph)
                parse_page( &doc, page );
            else
            {
#if defined(HAVE_BOOST)
                parse_page_graph( &doc, page );
#else
                std::cerr << "Can't use Boost::Graph output - not configured with Boost support" << std::endl;
                return 4;
#endif
            }
            cout << " - page ok" << endl;
        }
    }
    catch( const PdfError & e )
    {
        e.PrintErrorMsg();
        return e.GetError();
    }

    cout << endl;
    return 0;
}
