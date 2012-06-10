#include "infra/general/Log.hpp"

#include "xercesc/util/PlatformUtils.hpp"

#include "xercesc/framework/MemBufInputSource.hpp"
#include "xercesc/framework/LocalFileFormatTarget.hpp"
#include "xercesc/framework/LocalFileInputSource.hpp"
#include "xercesc/framework/Wrapper4InputSource.hpp"
#include "xercesc/dom/DOMImplementation.hpp"
#include "xercesc/dom/DOMImplementationLS.hpp"
#include "xercesc/dom/DOMImplementationRegistry.hpp"
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

#include "xydiff/XyLatinStr.hpp"
#include "xydiff/XID_DOMDocument.hpp"
#include "xydiff/XID_map.hpp"

#include "xydiff/DeltaException.hpp"
#include "DOMPrint.hpp"
#include "Tools.hpp"
#include <stdio.h>
#include <iostream>
#include <fstream>

XERCES_CPP_NAMESPACE_USE
using namespace std;

static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };

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
  DOMImplementation* impl =  DOMImplementationRegistry::getDOMImplementation(gLS);
  DOMDocument* doc = impl->createDocument();
  return new XID_DOMDocument(doc, NULL, true);
}

DOMDocument * XID_DOMDocument::getDOMDocumentOwnership() {
	DOMDocument * doc = theDocument;
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
	if (xidmap!=NULL) return 0;
	TRACE("getDocumentElement()");
	DOMElement* docRoot = getDocumentElement();
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

XID_DOMDocument::XID_DOMDocument(DOMDocument *doc, const char *xidmapStr, bool adoptDocument, DOMLSParser *domParser) : xidmap(NULL), theDocument(doc), theParser(NULL), doReleaseTheDocument(adoptDocument)
{
	removeIgnorableWhitespace((DOMNode*)theDocument);
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
	
	LocalFileFormatTarget *xmlfile = new LocalFileFormatTarget(xml_filename);
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

	DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(XMLString::transcode("LS"));
	DOMLSSerializer* theSerializer = ((DOMImplementationLS*)impl)->createLSSerializer();
	DOMLSOutput *theOutput = ((DOMImplementationLS*)impl)->createLSOutput();
	DOMConfiguration *configuration = theSerializer->getDomConfig();
	
	try {
		if (configuration->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true)) 
		    configuration->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);
		if (configuration->canSetParameter(XMLUni::fgDOMXMLDeclaration, true)) 
		    configuration->setParameter(XMLUni::fgDOMXMLDeclaration, true);
          // do the serialization through DOMLSSerializer::write();
		theOutput->setByteStream(xmlfile);
		theSerializer->write((DOMDocument*)this, theOutput);
      }
      catch (const XMLException& toCatch) {
          char* message = XMLString::transcode(toCatch.getMessage());
          std::cout << "Exception message is: \n"
               << message << "\n";
          XMLString::release(&message);
      }
      catch (const DOMException& toCatch) {
          char* message = XMLString::transcode(toCatch.msg);
          std::cout << "Exception message is: \n"
               << message << "\n";
          XMLString::release(&message);
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
	XID_DOMDocument* result = XID_DOMDocument::createDocument() ;
	DOMNode* resultRoot = result->importNode((DOMNode*)doc->getDocumentElement(), true);
	result->appendChild(resultRoot);

	if (withXID) {
		if ( doc->xidmap==NULL ) throw VersionManagerException("Program Error", "XID_DOMDocument::copy()", "Option 'withXID' used but source has NULL xidmap");
		result->xidmap = XID_map::addReference(new XID_map(doc->xidmap->String().c_str(), resultRoot));
	}
	return (result);	
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



bool xydeltaParseHandler::handleError(const DOMError& domError)
{
  DOMLocator* locator = domError.getLocation();
  cerr << "\n(GF) Error at (file " << XyLatinStr(locator->getURI()).localForm()
       << ", line " << (long) locator->getLineNumber()
       << ", char " << (long) locator->getColumnNumber()
       << "): " << XyLatinStr(domError.getMessage()).localForm() << endl;
  throw VersionManagerException("xydeltaParseHandler", "error", "-");
}

void xydeltaParseHandler::error(const SAXParseException& e) {
	cerr << "\n(GF) Error at (file " << XyLatinStr(e.getSystemId()).localForm()
	     << ", line " << (long) e.getLineNumber()
	     << ", char " << (long) e.getColumnNumber()
	     << "): " << XyLatinStr(e.getMessage()).localForm() << endl;
	throw VersionManagerException("xydeltaParseHandler", "error", "-");
	}
void xydeltaParseHandler::fatalError(const SAXParseException& e) {
	cerr << "\n(GF) Fatal Error at (file " << XyLatinStr(e.getSystemId()).localForm()
	     << ", line " << (long) e.getLineNumber()
	     << ", char " << (long) e.getColumnNumber()
	     << "): " << XyLatinStr(e.getMessage()).localForm() << endl;
	throw VersionManagerException("xydeltaParseHandler", "fatal error", "-");
	}
void xydeltaParseHandler::warning(const SAXParseException& e) {
	cerr << "\n(GF) Warning at (file " << XyLatinStr(e.getSystemId()).localForm()
	     << ", line " << (long) e.getLineNumber()
	     << ", char " << (long) e.getColumnNumber()
	     << "): " << XyLatinStr(e.getMessage()).localForm() << endl;
	}
  
void XID_DOMDocument::removeIgnorableWhitespace(DOMNode *node) {
	if ( node->hasChildNodes() ) {
		DOMNode* child = node->getFirstChild();
		DOMNode *nextChild;
		while(child!=NULL) {
			if (child->hasChildNodes()) {
				removeIgnorableWhitespace(child);
			}
			nextChild = child->getNextSibling();
			if ( child->getNodeType() == DOMNode::TEXT_NODE ) {
				XMLCh *childContent = XMLString::replicate(child->getNodeValue());
				XMLString::trim(childContent);
				if (XMLString::stringLen(childContent) == 0) {
					if (child->getNextSibling() != NULL || child->getPreviousSibling() != NULL) {
						node->removeChild(child);
					}
				}
				XMLString::release(&childContent);
			}
			child = nextChild;
		}
	}
}

void XID_DOMDocument::parseDOM_Document(const char* xmlfile, bool doValidation) {
	if (theDocument || theParser) {
		cerr << "current doc is not initialized, can not parse" << endl;
		throw VersionManagerException("XML Exception", "parseDOM_Document", "current doc is not initialized");
	}
	
	// Initialize the XML4C2 system
	try {
		XMLPlatformUtils::Initialize();
	}
	catch(const XMLException& e) {
		cerr << "Error during Xerces-c Initialization.\n"
		     << "  Exception message:" << XyLatinStr(e.getMessage()).localForm() << endl;
		ERRORMSG("Xerces::Initialize() FAILED");
		throw VersionManagerException("XML Exception", "parseDOM_Document", "Xerces-C++ Initialization failed");
	}

	//  Create our validator, then attach an error handler to the parser.
	//  The parser will call back to methods of the ErrorHandler if it
	//  discovers errors during the course of parsing the XML document.
	//  Then parse the XML file, catching any XML exceptions that might propogate out of it.
   
	// The parser owns the Validator, so we don't have to free it
  	// The parser also owns the DOMDocument
	
	static DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(gLS);
	theParser = ((DOMImplementationLS*)impl)->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
	
	bool errorsOccured = false;
  
	try {
		DOMErrorHandler * handler = new xydeltaParseHandler();
		if (theParser->getDomConfig()->canSetParameter(XMLUni::fgDOMValidate, doValidation))
			theParser->getDomConfig()->setParameter(XMLUni::fgDOMValidate, doValidation);
		if (theParser->getDomConfig()->canSetParameter(XMLUni::fgDOMElementContentWhitespace, false))
			theParser->getDomConfig()->setParameter(XMLUni::fgDOMElementContentWhitespace, false);
		theParser->getDomConfig()->setParameter(XMLUni::fgDOMErrorHandler, handler);
		LocalFileInputSource *fileIS = new LocalFileInputSource(XMLString::transcode(xmlfile));
		Wrapper4InputSource *wrapper = new Wrapper4InputSource(fileIS, false);
		theDocument = theParser->parse((DOMLSInput *)wrapper);
		// Remove text nodes within mixed content elements that are comprised
		// only of whitespace from DOMDocument
		removeIgnorableWhitespace((DOMNode*)theDocument);
	} catch (const XMLException& e) {
		cerr << "XMLException: An error occured during parsing\n   Message: "
		     << XyLatinStr(e.getMessage()).localForm() << endl;
		errorsOccured = true;
		throw VersionManagerException("XML Exception", "parseDOM_Document", "See previous exception messages for more details");
	} catch (const DOMException& toCatch) {
		char* message = XMLString::transcode(toCatch.msg);
		cout << "Exception message is: \n"
			<< message << "\n";
		XMLString::release(&message);
	}
	catch (...) {
       cout << "Unexpected Exception \n" ;
	}
}
 
//---------------------------------------------------------------------------
// get Document(Subtree)NodeCount
//---------------------------------------------------------------------------

int XID_DOMDocument::getSubtreeNodeCount(DOMNode *node) {
	int count = 0;
	if (node==NULL) {
		FATAL("unexpected NULL node");
		abort();
	}
        if(node->hasChildNodes()) {
		DOMNode* child = node->getFirstChild();
		while(child!=NULL) {
			count += getSubtreeNodeCount(child);
			child=child->getNextSibling();
			}
		}
	count++;
	return count;
	}

int XID_DOMDocument::getDocumentNodeCount() {
	DOMElement* docRoot = this->getDocumentElement() ;
	if (docRoot==NULL) {
		ERRORMSG("document has no Root Element");
		return 0;
	}
	return getSubtreeNodeCount( docRoot );
}

bool XID_DOMDocument::isRealData(DOMNode *node) {
	return true;
}


void Restricted::XidTagSubtree(XID_DOMDocument *doc, DOMNode* node) {
	
	// Tag Node
	
	XID_t myXid = doc->getXidMap().getXIDbyNode(node);
	if (node->getNodeType()==DOMNode::ELEMENT_NODE) {
		char xidStr[20];
		sprintf(xidStr, "%d", (int)myXid);
		DOMNamedNodeMap* attr = node->getAttributes();
		if (attr==NULL) throw VersionManagerException("Internal Error", "XidTagSubtree()", "Element node getAttributes() returns NULL");
		XMLCh tempStrA[100];
		XMLCh tempStrB[100];
		XMLString::transcode("urn:schemas-xydiff:xydelta", tempStrA, 99);
		XMLString::transcode("xy:xid", tempStrB, 99);
		DOMAttr* attrNode = doc->createAttributeNS(tempStrA, tempStrB);
		XMLString::transcode(xidStr, tempStrA, 99);
		attrNode->setValue(tempStrA);
		attr->setNamedItem(attrNode);
	} else if (node->getNodeType()==DOMNode::TEXT_NODE) {
		char xidStr[20];
		sprintf(xidStr, "%d", (int)myXid);
		DOMNamedNodeMap* attr = node->getParentNode()->getAttributes();
		if (attr==NULL) throw VersionManagerException("Internal Error", "XidTagSubtree()", "Element node getAttributes() returns NULL");
		XMLCh tempStrA[100];
		XMLCh tempStrB[100];
		XMLString::transcode("urn:schemas-xydiff:xydelta", tempStrA, 99);
		XMLString::transcode("xy:txid", tempStrB, 99);
		DOMAttr* attrNode = doc->createAttributeNS(tempStrA, tempStrB);
		XMLString::transcode(xidStr, tempStrA, 99);
		attrNode->setValue(tempStrA);
		attr->setNamedItem(attrNode);
	}
	
	
	// Apply Recursively
	
	if (node->hasChildNodes()) {
          DOMNode* child = node->getFirstChild();
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

DOMConfiguration * XID_DOMDocument::getDOMConfig() const
{
	return theDocument->getDOMConfig();
}

DOMElement * XID_DOMDocument::createElementNS(const XMLCh *element, const XMLCh *xmlNameSpace, const XMLFileLoc fileLoc1, const XMLFileLoc fileLoc2)
{
	return theDocument->createElementNS(element, xmlNameSpace, fileLoc1, fileLoc2);
}

DOMXPathResult * XID_DOMDocument::evaluate(const XMLCh *xpath, const DOMNode *node, const DOMXPathNSResolver *resolver, DOMXPathResult::ResultType resultType, DOMXPathResult *result)
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

DOMXPathNSResolver * XID_DOMDocument::createNSResolver(const DOMNode *node)
{
	return theDocument->createNSResolver(node);
}
DOMNode::NodeType XID_DOMDocument::getNodeType() const
{
  return theDocument->getNodeType();
}

DOMNode * XID_DOMDocument::getParentNode() const
{
  return theDocument->getParentNode();
}

DOMNodeList * XID_DOMDocument::getChildNodes() const
{
  return theDocument->getChildNodes();
}

DOMNode * XID_DOMDocument::getFirstChild() const
{
  return theDocument->getFirstChild();
}

DOMNode * XID_DOMDocument::getLastChild() const
{
  return theDocument->getLastChild();
}

DOMNode * XID_DOMDocument::getPreviousSibling() const
{
  return theDocument->getPreviousSibling();
}

DOMNode * XID_DOMDocument::getNextSibling() const
{
  return theDocument->getNextSibling();
}

DOMNamedNodeMap * XID_DOMDocument::getAttributes() const
{
  return theDocument->getAttributes();
}

DOMDocument * XID_DOMDocument::getOwnerDocument() const
{
  return theDocument->getOwnerDocument();
}

DOMNode * XID_DOMDocument::cloneNode(bool deep) const
{
  return theDocument->cloneNode(deep);
}

DOMNode * XID_DOMDocument::insertBefore(DOMNode * node1, DOMNode * node2)
{
  return theDocument->insertBefore(node1, node2);
}

DOMNode * XID_DOMDocument::replaceChild(DOMNode * node1, DOMNode *node2)
{
  return theDocument->replaceChild(node1, node2);
}

DOMNode * XID_DOMDocument::removeChild(DOMNode *node1)
{
  return theDocument->removeChild(node1);
}

DOMNode * XID_DOMDocument::appendChild(DOMNode *node1)
{
  return theDocument->appendChild(node1);
}

bool XID_DOMDocument::hasChildNodes() const
{
  return theDocument->hasChildNodes();
}

void XID_DOMDocument::setNodeValue(const XMLCh *nodeValue)
{
  theDocument->setNodeValue(nodeValue);
}

void XID_DOMDocument::normalize()
{
  theDocument->normalize();
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
  theDocument->setPrefix(prefix);
}

bool XID_DOMDocument::hasAttributes() const
{
  return theDocument->hasAttributes();
}

bool XID_DOMDocument::isSameNode(const DOMNode *node) const
{
  return theDocument->isSameNode(node);
}

bool XID_DOMDocument::isEqualNode(const DOMNode *node) const
{
  return theDocument->isEqualNode(node);
}

void * XID_DOMDocument::setUserData(const XMLCh *key, void *data, DOMUserDataHandler *handler)
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

const XMLCh * XID_DOMDocument::getTextContent() const
{
  return theDocument->getTextContent();
}

void XID_DOMDocument::setTextContent(const XMLCh *textContent)
{
  theDocument->setTextContent(textContent);
}

bool XID_DOMDocument::isDefaultNamespace(const XMLCh *namespaceURI) const
{
  return theDocument->isDefaultNamespace(namespaceURI);
}

const XMLCh * XID_DOMDocument::lookupNamespaceURI(const XMLCh *prefix) const
{
  return theDocument->lookupNamespaceURI(prefix);
}

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

DOMXPathExpression * XID_DOMDocument::createExpression(const XMLCh *xpath, const DOMXPathNSResolver *resolver)
{
	return theDocument->createExpression(xpath, resolver);
}

DOMRange * XID_DOMDocument::createRange()
{
  return theDocument->createRange();
}

DOMElement * XID_DOMDocument::createElement(const XMLCh *tagName)
{
  return theDocument->createElement(tagName);
}

DOMDocumentFragment * XID_DOMDocument::createDocumentFragment()
{
  return theDocument->createDocumentFragment();
}

DOMText * XID_DOMDocument::createTextNode(const XMLCh *data)
{
  return theDocument->createTextNode(data);
}

DOMComment * XID_DOMDocument::createComment(const XMLCh *data)
{
  return theDocument->createComment(data);
}

DOMCDATASection * XID_DOMDocument::createCDATASection(const XMLCh *data)
{
  return theDocument->createCDATASection(data);
}

DOMProcessingInstruction * XID_DOMDocument::createProcessingInstruction(const XMLCh *target, const XMLCh *data)
{
  return theDocument->createProcessingInstruction(target, data);
}

DOMAttr * XID_DOMDocument::createAttribute(const XMLCh *name)
{
  return theDocument->createAttribute(name);
}

DOMEntityReference * XID_DOMDocument::createEntityReference(const XMLCh *name)
{
  return theDocument->createEntityReference(name);
}

DOMDocumentType * XID_DOMDocument::getDoctype() const
{
  return theDocument->getDoctype();
}

DOMImplementation *XID_DOMDocument::getImplementation() const
{
  return theDocument->getImplementation();
}

DOMElement * XID_DOMDocument::getDocumentElement() const
{
  return theDocument->getDocumentElement();
}

DOMNodeList * XID_DOMDocument::getElementsByTagName(const XMLCh *tagName) const
{
  return theDocument->getElementsByTagName(tagName);
}

short int XID_DOMDocument::compareDocumentPosition(const DOMNode *node) const
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

DOMNode * XID_DOMDocument::importNode(const DOMNode *importNode, bool deep)
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

DOMElement * XID_DOMDocument::createElementNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName)
{
  return theDocument->createElementNS(namespaceURI, qualifiedName);
}

DOMAttr * XID_DOMDocument::createAttributeNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName)
{
  return theDocument->createAttributeNS(namespaceURI, qualifiedName);
}

DOMNodeList * XID_DOMDocument::getElementsByTagNameNS(const XMLCh *namespaceURI, const XMLCh *localName) const
{
  return theDocument->getElementsByTagNameNS(namespaceURI, localName);
}

DOMElement * XID_DOMDocument::getElementById(const XMLCh *elementId) const
{
  return theDocument->getElementById(elementId);
}

const XMLCh * XID_DOMDocument::getDocumentURI() const
{
  return theDocument->getDocumentURI();
}

void XID_DOMDocument::setDocumentURI(const XMLCh *documentURI)
{
  theDocument->setDocumentURI(documentURI);
}


bool XID_DOMDocument::getStrictErrorChecking() const
{
  return theDocument->getStrictErrorChecking();
}

void XID_DOMDocument::setStrictErrorChecking(bool strictErrorChecking)
{
  theDocument->setStrictErrorChecking(strictErrorChecking);
}

DOMNode * XID_DOMDocument::renameNode(DOMNode *n, const XMLCh *namespaceURI, const XMLCh *name)
{
  return theDocument->renameNode(n, namespaceURI, name);
}


DOMNode * XID_DOMDocument::adoptNode(DOMNode *node)
{
  return theDocument->adoptNode(node);
}


void XID_DOMDocument::normalizeDocument()
{
  theDocument->normalizeDocument();
}

DOMEntity * XID_DOMDocument::createEntity(const XMLCh *name)
{
  return theDocument->createEntity(name);
}


DOMDocumentType * XID_DOMDocument::createDocumentType(const XMLCh *name)
{
  return theDocument->createDocumentType(name);
}


DOMNotation * XID_DOMDocument::createNotation(const XMLCh *name)
{
  return theDocument->createNotation(name);
}


DOMElement * XID_DOMDocument::createElementNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName, const XMLSSize_t lineNum, const XMLSSize_t columnNum)
{

  return theDocument->createElementNS(namespaceURI, qualifiedName, lineNum, columnNum);
}

DOMTreeWalker * XID_DOMDocument::createTreeWalker(DOMNode *root, long unsigned int whatToShow, DOMNodeFilter *filter, bool entityReferenceExpansion){
  return theDocument->createTreeWalker(root, whatToShow, filter, entityReferenceExpansion);
}


DOMNodeIterator * XID_DOMDocument::createNodeIterator(DOMNode *root, long unsigned int whatToShow, DOMNodeFilter *filter, bool entityReferenceExpansion)
{
  return theDocument->createNodeIterator(root, whatToShow, filter, entityReferenceExpansion);
}
