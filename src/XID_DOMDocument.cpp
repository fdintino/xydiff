#include "infra/general/Log.hpp"

#include "xercesc/util/PlatformUtils.hpp"
//#include "xercesc/dom/deprecated/DOM_NamedNodeMap.hpp"
//#include "xercesc/dom/deprecated/DOMParser.hpp"


#include "xercesc/framework/MemBufInputSource.hpp"
#include "xercesc/framework/LocalFileFormatTarget.hpp"
#include "xercesc/dom/DOMImplementation.hpp"
#include "xercesc/dom/DOMImplementationLS.hpp"
#include "xercesc/dom/DOMImplementationRegistry.hpp"
// #include "xercesc/dom/DOMBuilder.hpp"
#include "xercesc/dom/DOMException.hpp"
#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/dom/DOMLSOutput.hpp"
#include "xercesc/dom/DOMLSSerializer.hpp"
#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/dom/DOMNodeList.hpp"

#include "xercesc/dom/DOMLocator.hpp"
#include "xercesc/dom/DOMAttr.hpp"
#include "xercesc/util/XMLUniDefs.hpp"
#include "xercesc/sax/ErrorHandler.hpp"
#include "xercesc/sax/SAXException.hpp"
#include "xercesc/sax/SAXParseException.hpp"

#include "include/XyLatinStr.hpp"
#include "include/XID_DOMDocument.hpp"
#include "include/XID_map.hpp"

#include "DeltaException.hpp"
#include "DOMPrint.hpp"
#include "Tools.hpp"
#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace std;

static const XMLCh gLS[] = { xercesc_3_0::chLatin_L, xercesc_3_0::chLatin_S, xercesc_3_0::chNull };

//
// Opens an XML file and if possible its associated XID-mapping file
// Apply all XIDs on the Xerces Nodes and create XID_map hash_map index to help 'getNodeWithXID()'
//

bool PrintWithXID = false ;
XID_map *PrintXidMap = NULL ;

void GlobalPrintContext_t::SetModeDebugXID( XID_map &xidmap ) { PrintXidMap = &xidmap; PrintWithXID=true; }
void GlobalPrintContext_t::ReleaseContext( void )             { PrintWithXID=false; } ;

class GlobalPrintContext_t globalPrintContext ;

// Create new XID_DOMDocument

XID_DOMDocument* XID_DOMDocument::createDocument(void) {
  xercesc_3_0::DOMImplementation* impl =  xercesc_3_0::DOMImplementationRegistry::getDOMImplementation(gLS);
  xercesc_3_0::DOMDocument* doc = impl->createDocument();
  return new XID_DOMDocument(doc, NULL, true);
}

xercesc_3_0::DOMDocument * XID_DOMDocument::getDOMDocumentOwnership() {
	xercesc_3_0::DOMDocument * doc = theDocument;
	theDocument = NULL;
	return doc;
}

/* Given an XML File, construct an XID_DOMDocument
 * There are 3 possibilities concerning XIDs for the Document:
 *
 * (1) The XID-file exists, it is used
 * (2) There is no XID-file, default XIDs (1-n|n+1) are assigned
 * (3) There is no XID-file, no XIDs are assigned for the moment
 *
 */

XID_map &XID_DOMDocument::getXidMap(void) {
	return *xidmap;
	}

void XID_DOMDocument::addXidMapFile(const char *xidmapFilename) {
	if (xidmapFilename==NULL) THROW_AWAY(("empty file name"));
	std::fstream XIDmap_file(xidmapFilename, std::fstream::in);
	if (!XIDmap_file) {
		printf("Warning: no .xidmap description for XML file <%s>, using defaults Xids (1-n|n+1)\n",xidmapFilename);
		char xidstring[100];
		int nodecount = addXidMap(NULL)-1;
		if (nodecount>1) sprintf(xidstring, "(1-%d|%d)", nodecount, nodecount+1 );
		else sprintf(xidstring, "(1|2)");
		std::string mapString = xidstring ;
		XIDmap_file.close();
		std::fstream writeXidmapFile(xidmapFilename, std::fstream::out);
		if (!writeXidmapFile) {
			fprintf(stderr, "Could not attach xid description to file - write access to %s denied\n",xidmapFilename);
			THROW_AWAY(("Can not write to Xidmap file"));
			}
		else {
			writeXidmapFile << mapString << endl ;
			printf("XIDmap attached to xml file (.xidmap file created)\n");
			}
		}
	else {
		std::string mapstr;
		XIDmap_file >> mapstr ;
		addXidMap(mapstr.c_str());
		}
	return;	
	}

int XID_DOMDocument::addXidMap(const char *theXidmap) {
	if (xidmap!=NULL) THROW_AWAY(("can't attach XidMap, slot not empty"));
	TRACE("getDocumentElement()");
	xercesc_3_0::DOMElement* docRoot = getDocumentElement();
	if (theXidmap==NULL) {
		int nodecount = getDocumentNodeCount() ;
		char xidstring[100];
		if (nodecount>1) sprintf(xidstring, "(1-%d|%d)", nodecount, nodecount+1 );
		else sprintf(xidstring, "(1|2)");
		TRACE("define default XidMap : " <<xidstring);
		xidmap = XID_map::addReference( new XID_map( xidstring, docRoot ) );
		return (nodecount+1);
		}
	xidmap = XID_map::addReference(new XID_map(theXidmap, docRoot));
	return -1;
	}

void XID_DOMDocument::initEmptyXidmapStartingAt(XID_t firstAvailableXid) {
	if (xidmap!=NULL) THROW_AWAY(("can't attach XidMap, slot not empty"));
	xidmap = XID_map::addReference( new XID_map() );
	if (firstAvailableXid>0) xidmap->initFirstAvailableXID(firstAvailableXid);
	}

// Construct XID_DOMDocument from a DOM_Document

XID_DOMDocument::XID_DOMDocument(xercesc_3_0::DOMDocument *doc, const char *xidmapStr, bool adoptDocument) : xidmap(NULL), theDocument(doc), theParser(NULL), doReleaseTheDocument(adoptDocument)
{
	if (xidmapStr) {
		(void)addXidMap(xidmapStr);
	}
}

XID_DOMDocument::XID_DOMDocument(const char* xmlfile, bool useXidMap, bool doValidation) : xidmap(NULL), theDocument(NULL), theParser(NULL), doReleaseTheDocument(false) {
	parseDOM_Document(xmlfile, doValidation);
	
	xidmap=NULL;
	if (xmlfile==NULL) THROW_AWAY(("bad argument for xmlfile"));
	std::string xidmap_filename = std::string( xmlfile )+std::string(".xidmap") ;
  
	if (!useXidMap) {
		xidmap = XID_map::addReference( new XID_map() );
		}
	else {
		addXidMapFile(xidmap_filename.c_str());
		}
		
	}

//
// Save the XID_DOMDocument by saving the document in one file, and the XIDString in another
//

void XID_DOMDocument::SaveAs(const char *xml_filename, bool saveXidMap) {
	
	// Saves XML file
	
	//xercesc_3_0::LocalFileFormatTarget xmlfile(xml_filename);
	xercesc_3_0::LocalFileFormatTarget *xmlfile = new xercesc_3_0::LocalFileFormatTarget(xml_filename);
	if (!xmlfile) {
		cerr << "Error: could not open <" << xml_filename << "> for output" << endl ;
		return ;
	}
	
	if (saveXidMap) {
		if (xidmap==NULL) throw VersionManagerException("Runtime Error", "XID_DOMDocument::SaveAs", "Cannot save XidMap as there is no XidMap");
		
		// Saves XID mapping
		std::string xidmap_filename = xml_filename ;
		xidmap_filename += ".xidmap" ;
	
		std::ofstream xidmapfile(xidmap_filename.c_str());
		if (!xidmapfile) {
			cerr << "Error: could not open <" << xidmap_filename << "> for output" << endl ;
			return ;
			}
		
		std::string xidString = xidmap->String() ;
		std::cout << "Version: SaveAs/xidmap->String()=" << xidString << endl ;	
		xidmapfile << xidString ;
	}

	XMLCh tempStr[100];
	xercesc_3_0::XMLString::transcode("LS", tempStr, 99);
	xercesc_3_0::DOMImplementation *impl = xercesc_3_0::DOMImplementationRegistry::getDOMImplementation(tempStr);
	xercesc_3_0::DOMLSSerializer* theSerializer = ((xercesc_3_0::DOMImplementationLS*)impl)->createLSSerializer();
	xercesc_3_0::DOMLSOutput *theOutput = ((xercesc_3_0::DOMImplementationLS*)impl)->createLSOutput();
	theOutput->setByteStream(xmlfile);
	try {
          // do the serialization through DOMLSSerializer::writeToString();
		theSerializer->write((xercesc_3_0::DOMDocument*)this, theOutput);
		//XMLCh * serializedString = theSerializer->writeToString((xercesc_3_0::DOMNode*)this);
		//char *xmlstring = xercesc_3_0::XMLString::transcode(serializedString);
		//xmlfile << xmlstring;
		//xercesc_3_0::XMLString::release(&xmlstring);
      }
      catch (const xercesc_3_0::XMLException& toCatch) {
          char* message = xercesc_3_0::XMLString::transcode(toCatch.getMessage());
          std::cout << "Exception message is: \n"
               << message << "\n";
          xercesc_3_0::XMLString::release(&message);
      }
      catch (const xercesc_3_0::DOMException& toCatch) {
          char* message = xercesc_3_0::XMLString::transcode(toCatch.msg);
          std::cout << "Exception message is: \n"
               << message << "\n";
          xercesc_3_0::XMLString::release(&message);
      }
      catch (...) {
          std::cout << "Unexpected Exception \n" ;
      }
	theOutput->release();
	theSerializer->release();

	}
	
//
// Create a copy of a document
//

XID_DOMDocument* XID_DOMDocument::copy(const XID_DOMDocument *doc, bool withXID) {
	std::cout << "XID_DOMDocument::copy" << std::endl;
	XID_DOMDocument* result = XID_DOMDocument::createDocument() ;
	std::cout << "create document ok" << std::endl;
	xercesc_3_0::DOMNode* resultRoot = result->importNode((xercesc_3_0::DOMNode*)doc->getDocumentElement(), true);
	std::cout << "node import ok" << std::endl;
	result->appendChild(resultRoot);
	std::cout << "append child ok" << std::endl;

	if (withXID) {
		if ( doc->xidmap==NULL ) throw VersionManagerException("Program Error", "XID_DOMDocument::copy()", "Option 'withXID' used but source has NULL xidmap");
		//result.xidmap = XID_map::addReference( new XID_map( resultRoot, doc.xidmap->getFirstAvailableXID() ) );
		result->xidmap = XID_map::addReference(new XID_map(doc->xidmap->String().c_str(), resultRoot));
		std::cout << "copy xidmap ok" << std::endl;
	}
	return (result) ;	
}

XID_DOMDocument::~XID_DOMDocument() {
	this->release();
}

//---------------------------------------------------------------------------
//  Initialisation of document from File
//---------------------------------------------------------------------------

/************************************************************************************************************************************
 ***                                                    ***
 ***                PARSING XML DOCUMENT                ***
 ***                                                    ***
 ************************************************************************************************************************************/

class xydeltaParseHandler : public xercesc_3_0::DOMErrorHandler {
public:
  void warning(const xercesc_3_0::SAXParseException& e);
  void error(const xercesc_3_0::SAXParseException& e);
  void fatalError(const xercesc_3_0::SAXParseException& e);
  void resetErrors() {};
  bool handleError(const xercesc_3_0::DOMError& domError);
} ;

bool xydeltaParseHandler::handleError(const xercesc_3_0::DOMError& domError)
{
  xercesc_3_0::DOMLocator* locator = domError.getLocation();
  cerr << "\n(GF) Error at (file " << XyLatinStr(locator->getURI()).localForm()
       << ", line " << locator->getLineNumber()
       << ", char " << locator->getColumnNumber()
       << "): " << XyLatinStr(domError.getMessage()).localForm() << endl;
  throw VersionManagerException("xydeltaParseHandler", "error", "-");
}

void xydeltaParseHandler::error(const xercesc_3_0::SAXParseException& e) {
	cerr << "\n(GF) Error at (file " << XyLatinStr(e.getSystemId()).localForm()
	     << ", line " << e.getLineNumber()
	     << ", char " << e.getColumnNumber()
	     << "): " << XyLatinStr(e.getMessage()).localForm() << endl;
	throw VersionManagerException("xydeltaParseHandler", "error", "-");
	}
void xydeltaParseHandler::fatalError(const xercesc_3_0::SAXParseException& e) {
	cerr << "\n(GF) Fatal Error at (file " << XyLatinStr(e.getSystemId()).localForm()
	     << ", line " << e.getLineNumber()
	     << ", char " << e.getColumnNumber()
	     << "): " << XyLatinStr(e.getMessage()).localForm() << endl;
	throw VersionManagerException("xydeltaParseHandler", "fatal error", "-");
	}
void xydeltaParseHandler::warning(const xercesc_3_0::SAXParseException& e) {
	cerr << "\n(GF) Warning at (file " << XyLatinStr(e.getSystemId()).localForm()
	     << ", line " << e.getLineNumber()
	     << ", char " << e.getColumnNumber()
	     << "): " << XyLatinStr(e.getMessage()).localForm() << endl;
	}
  
void XID_DOMDocument::parseDOM_Document(const char* xmlfile, bool doValidation) {
	if (theDocument || theParser) {
		cerr << "current doc is not initialized, can not parse" << endl;
		throw VersionManagerException("XML Exception", "parseDOM_Document", "current doc is not initialized");
	}
	
	// Initialize the XML4C2 system
	try {
		xercesc_3_0::XMLPlatformUtils::Initialize();
	}
	catch(const xercesc_3_0::XMLException& e) {
		cerr << "Error during Xerces-c Initialization.\n"
		     << "  Exception message:" << XyLatinStr(e.getMessage()).localForm() << endl;
		ERROR("Xerces::Initialize() FAILED");
		throw VersionManagerException("XML Exception", "parseDOM_Document", "Xerces-C++ Initialization failed");
	}

	//  Create our validator, then attach an error handler to the parser.
	//  The parser will call back to methods of the ErrorHandler if it
	//  discovers errors during the course of parsing the XML document.
	//  Then parse the XML file, catching any XML exceptions that might propogate out of it.
   
        // The parser owns the Validator, so we don't have to free it
  	// The parser also owns the DOMDocument
	
	static xercesc_3_0::DOMImplementation *impl = xercesc_3_0::DOMImplementationRegistry::getDOMImplementation(gLS);
	theParser = ((xercesc_3_0::DOMImplementationLS*)impl)->createLSParser(xercesc_3_0::DOMImplementationLS::MODE_SYNCHRONOUS, 0);
	//bool ignoresWhitespace = theParser->getIncludeIgnorableWhitespace();
	
	bool errorsOccured = false;
  
	try {
		// theParser->setFeature(xercesc_3_0::XMLUni::fgDOMValidation, doValidation);
		//theParser->setDoValidation(doValidation);
		xercesc_3_0::DOMErrorHandler * handler = new xydeltaParseHandler();
		if (theParser->getDomConfig()->canSetParameter(xercesc_3_0::XMLUni::fgDOMValidate, doValidation))
			theParser->getDomConfig()->setParameter(xercesc_3_0::XMLUni::fgDOMValidate, doValidation);
		if (theParser->getDomConfig()->canSetParameter(xercesc_3_0::XMLUni::fgDOMElementContentWhitespace, false))
			theParser->getDomConfig()->setParameter(xercesc_3_0::XMLUni::fgDOMElementContentWhitespace, false);
		theParser->getDomConfig()->setParameter(xercesc_3_0::XMLUni::fgDOMErrorHandler, handler);
	//	theParser->setErrorHandler(handler);
                theDocument = theParser->parseURI(xmlfile);
	} catch (const xercesc_3_0::XMLException& e) {
		cerr << "XMLException: An error occured during parsing\n   Message: "
		     << XyLatinStr(e.getMessage()).localForm() << endl;
		errorsOccured = true;
		throw VersionManagerException("XML Exception", "parseDOM_Document", "See previous exception messages for more details");
	} catch (const xercesc_3_0::DOMException& toCatch) {
		char* message = xercesc_3_0::XMLString::transcode(toCatch.msg);
		cout << "Exception message is: \n"
			<< message << "\n";
		xercesc_3_0::XMLString::release(&message);
	}
	catch (...) {
       cout << "Unexpected Exception \n" ;
	}
}
 
//---------------------------------------------------------------------------
// get Document(Subtree)NodeCount
//---------------------------------------------------------------------------

int XID_DOMDocument::getSubtreeNodeCount(xercesc_3_0::DOMNode *node) {
	int count = 0;
	if (node==NULL) {
		FATAL("unexpected NULL node");
		abort();
	}
        if(node->hasChildNodes()) {
		xercesc_3_0::DOMNode* child = node->getFirstChild();
		while(child!=NULL) {
			count += getSubtreeNodeCount(child);
			child=child->getNextSibling();
			}
		}
	count++;
	return count;
	}

int XID_DOMDocument::getDocumentNodeCount() {
	xercesc_3_0::DOMElement* docRoot = this->getDocumentElement() ;
	if (docRoot==NULL) {
		ERROR("document has no Root Element");
		return 0;
	}
	return getSubtreeNodeCount( docRoot );
}

bool XID_DOMDocument::isRealData(xercesc_3_0::DOMNode *node) {
	switch(node->getNodeType()) {
		
/*
 * this part needs to be optimized as it uses XyLatinStr (XML transcode)
 */ 

		case xercesc_3_0::DOMNode::TEXT_NODE: {
			XyLatinStr theText( node->getNodeValue() );
			unsigned int l = strlen(theText.localForm()) ;
			for(unsigned int i=0;i<l;i++) {
				if (theText.localForm()[i]>32) return true;
				}
			return false;
			}
			
		default:
			return true;
		}
	}


void Restricted::XidTagSubtree(XID_DOMDocument *doc, xercesc_3_0::DOMNode* node) {
	
	// Tag Node
	
	XID_t myXid = doc->getXidMap().getXIDbyNode(node);
	if (node->getNodeType()==xercesc_3_0::DOMNode::ELEMENT_NODE) {
		char xidStr[20];
		sprintf(xidStr, "%d", (int)myXid);
		xercesc_3_0::DOMNamedNodeMap* attr = node->getAttributes();
		if (attr==NULL) throw VersionManagerException("Internal Error", "XidTagSubtree()", "Element node getAttributes() returns NULL");
		// If we are dealing with the root node
		//if (doc->getDocumentNode()->isEqualNode(node)) {
		if (node->isEqualNode((xercesc_3_0::DOMNode*)doc->getDocumentElement())) {
			xercesc_3_0::DOMAttr* attrNodeNS = doc->createAttributeNS(
				xercesc_3_0::XMLString::transcode("http://www.w3.org/2000/xmlns/"),
				xercesc_3_0::XMLString::transcode("xmlns:xyd"));
				attrNodeNS->setValue(xercesc_3_0::XMLString::transcode("urn:schemas-xydiff:xydelta"));
				attr->setNamedItem(attrNodeNS);
		}
		xercesc_3_0::DOMAttr* attrNode = doc->createAttributeNS(
			xercesc_3_0::XMLString::transcode("urn:schemas-xydiff:xydelta"),
			xercesc_3_0::XMLString::transcode("xyd:XyXid"));
		attrNode->setValue(XyLatinStr(xidStr).wideForm());
		attr->setNamedItem(attrNode);
		}
	
	
	// Apply Recursively
	
	if (node->hasChildNodes()) {
          xercesc_3_0::DOMNode* child = node->getFirstChild();
		while(child!=NULL) {
			XidTagSubtree(doc, child);
			child=child->getNextSibling();
			}
		}
	}

const XMLCh * XID_DOMDocument::getNodeName() const
{
  return theDocument->getNodeName();
}

const XMLCh * XID_DOMDocument::getNodeValue() const
{
  return theDocument->getNodeValue();
}

const XMLCh * XID_DOMDocument::getXmlVersion() const
{
	return theDocument->getXmlVersion();
}

void XID_DOMDocument::setXmlVersion(const XMLCh *version)
{
	theDocument->setXmlVersion(version);
}

xercesc_3_0::DOMConfiguration * XID_DOMDocument::getDOMConfig() const
{
	return theDocument->getDOMConfig();
}

xercesc_3_0::DOMElement * XID_DOMDocument::createElementNS(const XMLCh *element, const XMLCh *xmlNameSpace, XMLFileLoc fileLoc1, XMLFileLoc fileLoc2)
{
	return theDocument->createElementNS(element, xmlNameSpace, fileLoc1, fileLoc2);
}

xercesc_3_0::DOMXPathResult * XID_DOMDocument::evaluate(const XMLCh *xpath, const xercesc_3_0::DOMNode *node, const xercesc_3_0::DOMXPathNSResolver *resolver, xercesc_3_0::DOMXPathResult::ResultType resultType, xercesc_3_0::DOMXPathResult *result)
{
	return theDocument->evaluate(xpath, node, resolver, resultType, result);
}

bool XID_DOMDocument::getXmlStandalone() const
{
	return theDocument->getXmlStandalone();
}

void XID_DOMDocument::setXmlStandalone(bool isStandalone)
{
	theDocument->setXmlStandalone(isStandalone);
}

xercesc_3_0::DOMXPathNSResolver * XID_DOMDocument::createNSResolver(const xercesc_3_0::DOMNode *node)
{
	return theDocument->createNSResolver(node);
}
xercesc_3_0::DOMNode::NodeType XID_DOMDocument::getNodeType() const
{
  return theDocument->getNodeType();
}

xercesc_3_0::DOMNode * XID_DOMDocument::getParentNode() const
{
  return theDocument->getParentNode();
}

xercesc_3_0::DOMNodeList * XID_DOMDocument::getChildNodes() const
{
  return theDocument->getChildNodes();
}

xercesc_3_0::DOMNode * XID_DOMDocument::getFirstChild() const
{
  return theDocument->getFirstChild();
}

xercesc_3_0::DOMNode * XID_DOMDocument::getLastChild() const
{
  return theDocument->getLastChild();
}

xercesc_3_0::DOMNode * XID_DOMDocument::getPreviousSibling() const
{
  return theDocument->getPreviousSibling();
}

xercesc_3_0::DOMNode * XID_DOMDocument::getNextSibling() const
{
  return theDocument->getNextSibling();
}

xercesc_3_0::DOMNamedNodeMap * XID_DOMDocument::getAttributes() const
{
  return theDocument->getAttributes();
}

xercesc_3_0::DOMDocument * XID_DOMDocument::getOwnerDocument() const
{
  return theDocument->getOwnerDocument();
}

xercesc_3_0::DOMNode * XID_DOMDocument::cloneNode(bool deep) const
{
  return theDocument->cloneNode(deep);
}

xercesc_3_0::DOMNode * XID_DOMDocument::insertBefore(xercesc_3_0::DOMNode * node1, xercesc_3_0::DOMNode * node2)
{
  return theDocument->insertBefore(node1, node2);
}

xercesc_3_0::DOMNode * XID_DOMDocument::replaceChild(xercesc_3_0::DOMNode * node1, xercesc_3_0::DOMNode *node2)
{
  return theDocument->replaceChild(node1, node2);
}

xercesc_3_0::DOMNode * XID_DOMDocument::removeChild(xercesc_3_0::DOMNode *node1)
{
  return theDocument->removeChild(node1);
}

xercesc_3_0::DOMNode * XID_DOMDocument::appendChild(xercesc_3_0::DOMNode *node1)
{
  return theDocument->appendChild(node1);
}

bool XID_DOMDocument::hasChildNodes() const
{
  return theDocument->hasChildNodes();
}

void XID_DOMDocument::setNodeValue(const XMLCh *nodeValue)
{
  return theDocument->setNodeValue(nodeValue);
}

void XID_DOMDocument::normalize()
{
  return theDocument->normalize();
}

bool XID_DOMDocument::isSupported(const XMLCh *feature, const XMLCh *version) const
{
  return theDocument->isSupported(feature, version);
}

const XMLCh * XID_DOMDocument::getNamespaceURI() const
{
  return theDocument->getNamespaceURI();
}

const XMLCh * XID_DOMDocument::getPrefix() const
{
  return theDocument->getPrefix();
}

const XMLCh * XID_DOMDocument::getLocalName() const
{
  return theDocument->getLocalName();
}

void XID_DOMDocument::setPrefix(const XMLCh *prefix)
{
  return theDocument->setPrefix(prefix);
}

bool XID_DOMDocument::hasAttributes() const
{
  return theDocument->hasAttributes();
}

bool XID_DOMDocument::isSameNode(const xercesc_3_0::DOMNode *node) const
{
  return theDocument->isSameNode(node);
}

bool XID_DOMDocument::isEqualNode(const xercesc_3_0::DOMNode *node) const
{
  return theDocument->isEqualNode(node);
}

void * XID_DOMDocument::setUserData(const XMLCh *key, void *data, xercesc_3_0::DOMUserDataHandler *handler)
{
  return theDocument->setUserData(key, data, handler);
}

void * XID_DOMDocument::getUserData(const XMLCh *key) const
{
  return theDocument->getUserData(key);
}

const XMLCh * XID_DOMDocument::getBaseURI() const
{
  return theDocument->getBaseURI();
}

// short int XID_DOMDocument::compareTreePosition(const xercesc_3_0::DOMNode *node) const
// {
//   return theDocument->compareTreePosition(node);
// }

const XMLCh * XID_DOMDocument::getTextContent() const
{
  return theDocument->getTextContent();
}

void XID_DOMDocument::setTextContent(const XMLCh *textContent)
{
  return theDocument->setTextContent(textContent);
}

// const XMLCh * XID_DOMDocument::lookupNamespacePrefix(const XMLCh *namespaceURI, bool useDefault) const
// {
//   return theDocument->lookupNamespacePrefix(namespaceURI, useDefault);
// }

bool XID_DOMDocument::isDefaultNamespace(const XMLCh *namespaceURI) const
{
  return theDocument->isDefaultNamespace(namespaceURI);
}

const XMLCh * XID_DOMDocument::lookupNamespaceURI(const XMLCh *prefix) const
{
  return theDocument->lookupNamespaceURI(prefix);
}
// 
// xercesc_3_0::DOMNode * XID_DOMDocument::getInterface(const XMLCh *feature)
// {
//   return theDocument->getInterface(feature);
// }

void XID_DOMDocument::release() {
	if (theParser) {
		NOTE("delete parser owning the document");
		delete theParser;
		theParser=NULL;
		theDocument=NULL;
	} else if (doReleaseTheDocument) {
		if (theDocument) {
			NOTE("delete the document");
			delete theDocument;
			theDocument=NULL;
		}
	}
	theDocument=NULL;		
	if (xidmap) {
		NOTE("delete the xidmap");
		XID_map::removeReference(xidmap);
	}
}

xercesc_3_0::DOMXPathExpression * XID_DOMDocument::createExpression(const XMLCh *xpath, const xercesc_3_0::DOMXPathNSResolver *resolver)
{
	return theDocument->createExpression(xpath, resolver);
}

xercesc_3_0::DOMRange * XID_DOMDocument::createRange()
{
  return theDocument->createRange();
}

xercesc_3_0::DOMElement * XID_DOMDocument::createElement(const XMLCh *tagName)
{
  return theDocument->createElement(tagName);
}

xercesc_3_0::DOMDocumentFragment * XID_DOMDocument::createDocumentFragment()
{
  return theDocument->createDocumentFragment();
}

xercesc_3_0::DOMText * XID_DOMDocument::createTextNode(const XMLCh *data)
{
  return theDocument->createTextNode(data);
}

xercesc_3_0::DOMComment * XID_DOMDocument::createComment(const XMLCh *data)
{
  return theDocument->createComment(data);
}

xercesc_3_0::DOMCDATASection * XID_DOMDocument::createCDATASection(const XMLCh *data)
{
  return theDocument->createCDATASection(data);
}

xercesc_3_0::DOMProcessingInstruction * XID_DOMDocument::createProcessingInstruction(const XMLCh *target, const XMLCh *data)
{
  return theDocument->createProcessingInstruction(target, data);
}

xercesc_3_0::DOMAttr * XID_DOMDocument::createAttribute(const XMLCh *name)
{
  return theDocument->createAttribute(name);
}

xercesc_3_0::DOMEntityReference * XID_DOMDocument::createEntityReference(const XMLCh *name)
{
  return theDocument->createEntityReference(name);
}

xercesc_3_0::DOMDocumentType * XID_DOMDocument::getDoctype() const
{
  return theDocument->getDoctype();
}

xercesc_3_0::DOMImplementation *XID_DOMDocument::getImplementation() const
{
  return theDocument->getImplementation();
}

xercesc_3_0::DOMElement * XID_DOMDocument::getDocumentElement() const
{
  return theDocument->getDocumentElement();
}

xercesc_3_0::DOMNodeList * XID_DOMDocument::getElementsByTagName(const XMLCh *tagName) const
{
  return theDocument->getElementsByTagName(tagName);
}

short int XID_DOMDocument::compareDocumentPosition(const xercesc_3_0::DOMNode *node) const
{
	return theDocument->compareDocumentPosition(node);
}

const XMLCh* XID_DOMDocument::lookupPrefix(const XMLCh *prefix) const
{
	return theDocument->lookupPrefix(prefix);
}

void * XID_DOMDocument::getFeature(const XMLCh *param1, const XMLCh *param2) const
{
	return theDocument->getFeature(param1, param2);
}

xercesc_3_0::DOMNode * XID_DOMDocument::importNode(const xercesc_3_0::DOMNode *importNode, bool deep)
{
  return theDocument->importNode(importNode, deep);
}

const XMLCh * XID_DOMDocument::getInputEncoding() const
{
	return theDocument->getInputEncoding();
}

const XMLCh * XID_DOMDocument::getXmlEncoding() const
{
	return theDocument->getXmlEncoding();
}

xercesc_3_0::DOMElement * XID_DOMDocument::createElementNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName)
{
  return theDocument->createElementNS(namespaceURI, qualifiedName);
}

xercesc_3_0::DOMAttr * XID_DOMDocument::createAttributeNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName)
{
  return theDocument->createAttributeNS(namespaceURI, qualifiedName);
}

xercesc_3_0::DOMNodeList * XID_DOMDocument::getElementsByTagNameNS(const XMLCh *namespaceURI, const XMLCh *localName) const
{
  return theDocument->getElementsByTagNameNS(namespaceURI, localName);
}

xercesc_3_0::DOMElement * XID_DOMDocument::getElementById(const XMLCh *elementId) const
{
  return theDocument->getElementById(elementId);
}

// const XMLCh * XID_DOMDocument::getActualEncoding() const
// {
//   return theDocument->getActualEncoding();
// }
// 
// void XID_DOMDocument::setActualEncoding(const XMLCh *actualEncoding)
// {
//   return theDocument->setActualEncoding(actualEncoding);
// }

// const XMLCh * XID_DOMDocument::getEncoding() const
// {
//   return theDocument->getEncoding();
// }
// 
// void XID_DOMDocument::setEncoding(const XMLCh *encoding)
// {
//   return theDocument->setEncoding(encoding);
// }

// bool XID_DOMDocument::getStandalone() const
// {
//   return theDocument->getStandalone();
// }

// void XID_DOMDocument::setStandalone(bool standalone)
// {
//   return theDocument->setStandalone(standalone);
// }

// const XMLCh * XID_DOMDocument::getVersion() const
// {
//   return theDocument->getVersion();
// }
// 
// void XID_DOMDocument::setVersion(const XMLCh *version)
// {
//   return theDocument->setVersion(version);
// }

const XMLCh * XID_DOMDocument::getDocumentURI() const
{
  return theDocument->getDocumentURI();
}

void XID_DOMDocument::setDocumentURI(const XMLCh *documentURI)
{
  return theDocument->setDocumentURI(documentURI);
}





bool XID_DOMDocument::getStrictErrorChecking() const
{
  return theDocument->getStrictErrorChecking();
}

void XID_DOMDocument::setStrictErrorChecking(bool strictErrorChecking)
{
  return theDocument->setStrictErrorChecking(strictErrorChecking);
}

// 
// xercesc_3_0::DOMErrorHandler * XID_DOMDocument::getErrorHandler() const
// {
//   return theDocument->getErrorHandler();
// }
// 
// 
// void XID_DOMDocument::setErrorHandler(xercesc_3_0::DOMErrorHandler *handler)
// {
//   return theDocument->setErrorHandler(handler);
// }


xercesc_3_0::DOMNode * XID_DOMDocument::renameNode(xercesc_3_0::DOMNode *n, const XMLCh *namespaceURI, const XMLCh *name)
{
  return theDocument->renameNode(n, namespaceURI, name);
}


xercesc_3_0::DOMNode * XID_DOMDocument::adoptNode(xercesc_3_0::DOMNode *node)
{
  return theDocument->adoptNode(node);
}


void XID_DOMDocument::normalizeDocument()
{
  return theDocument->normalizeDocument();
}


// bool XID_DOMDocument::canSetNormalizationFeature(const XMLCh *name, bool state) const
// {
//   return theDocument->canSetNormalizationFeature(name, state);
// }
// 
// 
// void XID_DOMDocument::setNormalizationFeature(const XMLCh *name, bool state)
// {
//   return theDocument->setNormalizationFeature(name, state);
// }
// 
// 
// bool XID_DOMDocument::getNormalizationFeature(const XMLCh *name) const
// {
//   return theDocument->getNormalizationFeature(name);
// }


xercesc_3_0::DOMEntity * XID_DOMDocument::createEntity(const XMLCh *name)
{
  return theDocument->createEntity(name);
}


xercesc_3_0::DOMDocumentType * XID_DOMDocument::createDocumentType(const XMLCh *name)
{
  return theDocument->createDocumentType(name);
}


xercesc_3_0::DOMNotation * XID_DOMDocument::createNotation(const XMLCh *name)
{
  return theDocument->createNotation(name);
}


xercesc_3_0::DOMElement * XID_DOMDocument::createElementNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName, const XMLSSize_t lineNum, const XMLSSize_t columnNum)
{

  return theDocument->createElementNS(namespaceURI, qualifiedName, lineNum, columnNum);
}

xercesc_3_0::DOMTreeWalker * XID_DOMDocument::createTreeWalker(xercesc_3_0::DOMNode *root, long unsigned int whatToShow, xercesc_3_0::DOMNodeFilter *filter, bool entityReferenceExpansion){
  return theDocument->createTreeWalker(root, whatToShow, filter, entityReferenceExpansion);
}


xercesc_3_0::DOMNodeIterator * XID_DOMDocument::createNodeIterator(xercesc_3_0::DOMNode *root, long unsigned int whatToShow, xercesc_3_0::DOMNodeFilter *filter, bool entityReferenceExpansion)
{
  return theDocument->createNodeIterator(root, whatToShow, filter, entityReferenceExpansion);
}
