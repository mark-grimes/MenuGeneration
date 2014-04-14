#include "l1menu/tools/XMLFile.h"

#include <iostream>
#include <stdexcept>
#include <vector>
#include <sstream>
#include "l1menu/tools/XMLElement.h"
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>

namespace // Use the unnamed namespace for things only used in this file
{

	/** @brief Simple sentry class to call XMLPlatformUtils::Terminate() in an exception safe way
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
	 * @date 02/Oct/2013
	 */
	class XMLPlatformInitialise
	{
	public:
		XMLPlatformInitialise() { xercesc::XMLPlatformUtils::Initialize(); }
		~XMLPlatformInitialise() { xercesc::XMLPlatformUtils::Terminate(); }
	};

	/** @brief Used by the xerces framework to output xml. Dumps it to the std::ostream passed in the constructor.
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
	 * @date 02/Oct/2013
	 */
	class OStreamXMLFormatTarget : public xercesc::XMLFormatTarget
	{
	public:
		OStreamXMLFormatTarget( std::ostream& stream ) : stream_(stream) {}
		virtual void writeChars( const XMLByte* const toWrite, const unsigned int count, xercesc::XMLFormatter* const formatter )
		{
			stream_ << toWrite;
		}

		virtual void flush()
		{
		}
	protected:
		std::ostream& stream_;
	};

	/** @brief Class copied from a xerces example to convert a std::string into the format xerces uses.
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk) but copied from CreateDOMDocument.cpp in the xerces samples
	 * @date 02/Oct/2013
	 */
	class XercesString
	{
	public:
		XercesString( const char* cString );
		XercesString( const std::string& string );
		~XercesString();
		const XMLCh* get();
	protected:
		XMLCh*   pXMLString_;
	};

	/** @brief Corollary of the XercesString class to convert xerces format strings to native C strings.
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
	 * @date 03/Oct/2013
	 */
	class NativeString
	{
	public:
		NativeString( const XMLCh* xercesString );
		~NativeString();
		const char* get();
	protected:
		char*   pNativeString_;
	};

	XercesString::XercesString( const char* cString ) { pXMLString_=xercesc::XMLString::transcode(cString); }
	XercesString::XercesString( const std::string& string ) { pXMLString_=xercesc::XMLString::transcode(string.c_str()); }
	XercesString::~XercesString() { xercesc::XMLString::release(&pXMLString_); }
	const XMLCh* XercesString::get() { return pXMLString_; }

	NativeString::NativeString( const XMLCh* xercesString ) { pNativeString_=xercesc::XMLString::transcode(xercesString); }
	NativeString::~NativeString() { xercesc::XMLString::release(&pNativeString_); }
	const char* NativeString::get() { return pNativeString_; }

} // end of the unnamed namespace


//
// Pimple class has been implicitly declared but needs to be explicitly
// declared and defined.
//
namespace l1menu
{
	namespace tools
	{
		class XMLFilePrivateMembers
		{
		public:
			XMLFilePrivateMembers();
			xercesc::DOMImplementation* pDomImplementation_;
			xercesc::DOMImplementationLS* pDomImplementationLS_;
			xercesc::DOMDocument* pDocument_; ///< @brief Always points to the DOMDocument no matter who owns it.

			XMLPlatformInitialise initialisationSentry_; ///< @brief Makes sure xerces has been initialised and is closed down
			xercesc::XercesDOMParser parser_;
			/** @brief This looks after releasing the DOMDocument if it is not owned by parser_. This will be nullptr if
			 * the document is owned by parser_, so for general use one should use pDocument_ which always points to the
			 * document no matter who owns it. */
			std::unique_ptr<xercesc::DOMDocument,void(*)(xercesc::DOMDocument* p)> pOwnedDocument_;
		};

		XMLFilePrivateMembers::XMLFilePrivateMembers()
			// Set up the smart pointer so that it will use the xerces method to release the object (if it exists).
			// Don't actually call "delete" because it causes a crash.
			: pOwnedDocument_(nullptr,[](xercesc::DOMDocument* p){if(p!=nullptr){p->release();}})
		{
			pDomImplementation_=xercesc::DOMImplementationRegistry::getDOMImplementation( ::XercesString("Core LS").get() );
			if( pDomImplementation_==nullptr ) throw std::runtime_error( "Couldn't create the DOMImplementation" );
			pDomImplementationLS_=static_cast<xercesc::DOMImplementationLS*>( pDomImplementation_ );
		}

	} // end of namespace tools
} // end of namespace l1menu





l1menu::tools::XMLFile::XMLFile() : pImple( new l1menu::tools::XMLFilePrivateMembers )
{
	//using l1menu::tools::XMLFile::XercesString;

	// First put the new document in a smart pointer so that it will automatically be deleted, since I own it.
	pImple->pOwnedDocument_.reset( pImple->pDomImplementation_->createDocument( nullptr, XercesString("l1menu").get(), nullptr ) );
	// Then copy the raw pointer so that I have a consistent way of referring to the document
	// whether I own it or parser_ owns it.
	pImple->pDocument_=pImple->pOwnedDocument_.get();

	//
	// Create some default entries to describe what program the file is for
	//
	xercesc::DOMElement* pDescriptionElement=pImple->pDocument_->createElement( XercesString( "description" ).get() );
	pImple->pDocument_->getDocumentElement()->appendChild( pDescriptionElement );

	xercesc::DOMText* pDescriptionValue=pImple->pDocument_->createTextNode( XercesString( "File for use with the L1Trigger/MenuGeneration CMSSW package" ).get() );
	pDescriptionElement->appendChild( pDescriptionValue );

//	xercesc::DOMElement* rootElem=pImple->pDocument_->getDocumentElement();
//	xercesc::DOMElement* prodElem=pImple->pDocument_->createElement( XercesString( "product" ).get() );
//	rootElem->appendChild( prodElem );
//
//	xercesc::DOMText* prodDataVal=pImple->pDocument_->createTextNode( XercesString( "Xerces-C" ).get() );
//	prodElem->appendChild( prodDataVal );
//
//	xercesc::DOMElement* catElem=pImple->pDocument_->createElement( XercesString( "category" ).get() );
//	rootElem->appendChild( catElem );
//
//	catElem->setAttribute( XercesString( "idea" ).get(), XercesString( "great" ).get() );
//
//	xercesc::DOMText* catDataVal=pImple->pDocument_->createTextNode( XercesString( "XML Parsing Tools" ).get() );
//	catElem->appendChild( catDataVal );
//
//	xercesc::DOMElement* devByElem=pImple->pDocument_->createElement( XercesString( "developedBy" ).get() );
//	rootElem->appendChild( devByElem );
//
//	xercesc::DOMText* devByDataVal=pImple->pDocument_->createTextNode( XercesString( "Apache Software Foundation" ).get() );
//	devByElem->appendChild( devByDataVal );

}

l1menu::tools::XMLFile::XMLFile( const std::string& filename ) : pImple( new l1menu::tools::XMLFilePrivateMembers )
{
	parseFromFile( filename );
}

l1menu::tools::XMLFile::~XMLFile()
{

}

void l1menu::tools::XMLFile::parseFromFile( const std::string& filename )
{
	pImple->parser_.parse( filename.c_str() );
	// Copy a raw pointer to the document so that I have a consistent way of referring to it
	// even when the parser wasn't used (i.e. when creating a document in memory).
	pImple->pDocument_=pImple->parser_.getDocument();

	if( pImple->pDocument_==nullptr )
	{
		// I want to throw an exception to say the file couldn't be opened. However, I want to have the
		// file in a valid state so that it's possible to catch the exception and carry on anyway. So
		// I'll do the steps I'd normally do if no filename was supplied and only then throw the exception.
		pImple->pOwnedDocument_.reset( pImple->pDomImplementation_->createDocument( nullptr, XercesString("l1menu").get(), nullptr ) );
		pImple->pDocument_=pImple->pOwnedDocument_.get();
		xercesc::DOMElement* pDescriptionElement=pImple->pDocument_->createElement( XercesString( "description" ).get() );
		pImple->pDocument_->getDocumentElement()->appendChild( pDescriptionElement );
		xercesc::DOMText* pDescriptionValue=pImple->pDocument_->createTextNode( XercesString( "File for use with the L1Trigger/MenuGeneration CMSSW package" ).get() );
		pDescriptionElement->appendChild( pDescriptionValue );

		throw std::runtime_error( "Couldn't open the file "+filename );
	}
	else if( pImple->pDocument_->getDocumentElement()==nullptr ) throw std::runtime_error( filename+" doesn't appear to be an xml file" );


}

l1menu::tools::XMLElement l1menu::tools::XMLFile::rootElement()
{
	return l1menu::tools::XMLElement( pImple->pDocument_->getDocumentElement(), pImple->pDocument_ );
}

const l1menu::tools::XMLElement l1menu::tools::XMLFile::rootElement() const
{
	return l1menu::tools::XMLElement( pImple->pDocument_->getDocumentElement(), pImple->pDocument_ );
}

void l1menu::tools::XMLFile::outputToStream( std::ostream& outputStream )
{
	xercesc::DOMWriter* pSerializer = pImple->pDomImplementationLS_->createDOMWriter();
	if( pSerializer->canSetFeature(xercesc::XMLUni::fgDOMWRTFormatPrettyPrint,true) ) pSerializer->setFeature(xercesc::XMLUni::fgDOMWRTFormatPrettyPrint,true);
	OStreamXMLFormatTarget myFormatter( outputStream );

	pSerializer->writeNode( &myFormatter, *pImple->pDocument_ );
}
