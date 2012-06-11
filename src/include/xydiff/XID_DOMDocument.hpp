#ifndef XID_DOM_DOCUMENT
#define XID_DOM_DOCUMENT

#include "xydiff/XID_map.hpp"

#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/dom/DOMConfiguration.hpp"
#include "xercesc/dom/DOMError.hpp"
#include "xercesc/dom/DOMErrorHandler.hpp"
#include "xercesc/dom/DOMLSParser.hpp"
#include <iostream>
#include "xercesc/sax/ErrorHandler.hpp"
#include "xercesc/sax/SAXException.hpp"
#include "xercesc/sax/SAXParseException.hpp"

class DTDValidator;

class XID_DOMDocument : public xercesc::DOMDocument {
	private:
		// not allowed, not implemented
		XID_DOMDocument(); 
		XID_DOMDocument(const XID_DOMDocument &);
		XID_DOMDocument & operator = (const XID_DOMDocument &);

	public:
		// Constructors
		
		static XID_DOMDocument* createDocument(void);
		static XID_DOMDocument* createDocument(const XMLCh *namespaceURI, const XMLCh *qualifiedName);
		static XID_DOMDocument* copy(const XID_DOMDocument *doc, bool withXID=true);
		XID_DOMDocument(xercesc::DOMDocument *doc, const char *xidmapStr=NULL, bool adoptDocument=false, xercesc::DOMLSParser *domParser=NULL);
		XID_DOMDocument(const char* xmlfile, bool useXidMap=true, bool doValidation=false);
		
		void SaveAs(const char *xml_filename, bool saveXidMap=true);
		~XID_DOMDocument();

		// Methods
		
		void addXidMapFile(const char *xidmapFilename);
		int addXidMap(const char *theXidmap=NULL);
		void initEmptyXidmapStartingAt(XID_t firstAvailableXid);

		XID_map &getXidMap();
		xercesc::DOMDocument * getDOMDocumentOwnership();

		static int getSubtreeNodeCount(xercesc::DOMNode *node);
		int getDocumentNodeCount() ; //int getNodeCount();

		static bool isRealData(xercesc::DOMNode *node);
  
		xercesc::DOMNodeIterator * createNodeIterator(xercesc::DOMNode *root, long unsigned int whatToShow, xercesc::DOMNodeFilter *filter, bool entityReferenceExpansion);
  
		const XMLCh * getNodeName() const;
		const XMLCh * getNodeValue() const;
		xercesc::DOMNode::NodeType getNodeType() const;
		xercesc::DOMNode * getParentNode() const;
		xercesc::DOMNodeList * getChildNodes() const;
		xercesc::DOMNode * getFirstChild() const;
		xercesc::DOMNode * getLastChild() const;
		xercesc::DOMNode * getPreviousSibling() const;
		xercesc::DOMNode * getNextSibling() const;
		xercesc::DOMNamedNodeMap * getAttributes() const;
		xercesc::DOMDocument * getOwnerDocument() const;
		xercesc::DOMNode * cloneNode(bool deep) const;
		xercesc::DOMNode * insertBefore(xercesc::DOMNode *node1, xercesc::DOMNode *node2);
		xercesc::DOMNode * replaceChild(xercesc::DOMNode *node1, xercesc::DOMNode *node2);
		xercesc::DOMNode * removeChild(xercesc::DOMNode *node);
		xercesc::DOMNode * appendChild(xercesc::DOMNode *node);
		bool hasChildNodes() const;
		void setNodeValue(const XMLCh *nodeValue);
		void normalize();
		bool isSupported(const XMLCh *feature, const XMLCh *version) const;
		const XMLCh * getNamespaceURI() const;
		const XMLCh * getPrefix() const;
		const XMLCh * getLocalName() const;
		void setPrefix(const XMLCh *prefix);
		bool hasAttributes() const;
		bool isSameNode(const xercesc::DOMNode *node) const;
		bool isEqualNode(const xercesc::DOMNode *node) const;
		void * setUserData(const XMLCh *key, void *data, xercesc::DOMUserDataHandler *handler);
		void * getUserData(const XMLCh *key) const;
		const XMLCh * getBaseURI() const;
		const XMLCh * getTextContent() const;
		void setTextContent(const XMLCh *textContent);
		bool isDefaultNamespace(const XMLCh *namespaceURI) const;
		const XMLCh * lookupNamespaceURI(const XMLCh *prefix) const;
		void release();
		xercesc::DOMXPathExpression * createExpression(const XMLCh *xpath, const xercesc::DOMXPathNSResolver *resolver);
		xercesc::DOMXPathNSResolver * createNSResolver(const xercesc::DOMNode *node);
		xercesc::DOMXPathResult * evaluate(const XMLCh *xpath, const xercesc::DOMNode *node, const xercesc::DOMXPathNSResolver *resolver, xercesc::DOMXPathResult::ResultType resultType, xercesc::DOMXPathResult *result);
		short int compareDocumentPosition(const xercesc::DOMNode *node) const;
		const XMLCh* lookupPrefix(const XMLCh *prefix) const;
		void* getFeature(const XMLCh *param1, const XMLCh *param2) const;
		xercesc::DOMNode * importNode(const xercesc::DOMNode *node, bool trueOrFalse);
		const XMLCh * getInputEncoding() const;
		const XMLCh * getXmlEncoding() const;
		bool getXmlStandalone() const;
		void setXmlStandalone(bool isStandalone);
		const XMLCh * getXmlVersion() const;
		void setXmlVersion(const XMLCh *version);
		xercesc::DOMConfiguration * getDOMConfig() const;
		xercesc::DOMElement * createElementNS(const XMLCh *element, const XMLCh *xmlNameSpace, const XMLFileLoc fileLoc1, const XMLFileLoc fileLoc2);
		xercesc::DOMRange * createRange();
		xercesc::DOMElement * createElement(const XMLCh *tagName);
		xercesc::DOMDocumentFragment * createDocumentFragment();
		xercesc::DOMText * createTextNode(const XMLCh *data);
		xercesc::DOMComment * createComment(const XMLCh *data);
		xercesc::DOMCDATASection * createCDATASection(const XMLCh *data);
		xercesc::DOMProcessingInstruction * createProcessingInstruction(const XMLCh *target, const XMLCh *data);
		xercesc::DOMAttr * createAttribute(const XMLCh *name);
		xercesc::DOMEntityReference * createEntityReference(const XMLCh *name);
		xercesc::DOMDocumentType * getDoctype() const;
		xercesc::DOMImplementation *getImplementation() const;
		xercesc::DOMElement * getDocumentElement() const;
		xercesc::DOMNodeList * getElementsByTagName(const XMLCh *tagName) const;
		xercesc::DOMElement * createElementNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName);
		xercesc::DOMAttr * createAttributeNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName);
		xercesc::DOMNodeList * getElementsByTagNameNS(const XMLCh *namespaceURI, const XMLCh *localName) const;
		xercesc::DOMElement * getElementById(const XMLCh *elementId) const;

		const XMLCh * getDocumentURI() const;
		void setDocumentURI(const XMLCh *documentURI);

		bool getStrictErrorChecking() const;
		void setStrictErrorChecking(bool strictErrorChecking);
		xercesc::DOMNode * renameNode(xercesc::DOMNode *n, const XMLCh *namespaceURI, const XMLCh *name);
		xercesc::DOMNode * adoptNode(xercesc::DOMNode *node);
		void normalizeDocument();
		xercesc::DOMEntity * createEntity(const XMLCh *name);
		xercesc::DOMDocumentType * createDocumentType(const XMLCh *name);
		xercesc::DOMNotation * createNotation(const XMLCh *name);
		xercesc::DOMElement * createElementNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName, const XMLSSize_t lineNum, const XMLSSize_t columnNum);
  
		xercesc::DOMTreeWalker * createTreeWalker(xercesc::DOMNode *root, long unsigned int whatToShow, xercesc::DOMNodeFilter *filter, bool entityReferenceExpansion);


	private:		
		XID_map *xidmap;
		xercesc::DOMDocument* theDocument;
		xercesc::DOMLSParser*  theParser;
		bool                      doReleaseTheDocument;
		void parseDOM_Document(const char* xmlfile, bool doValidation);
		void removeIgnorableWhitespace(xercesc::DOMNode *node);

} ;

class GlobalPrintContext_t {
	public:
		void SetModeDebugXID( XID_map &xidmap ) ;
		void ReleaseContext( void ) ;
} ;

extern class GlobalPrintContext_t globalPrintContext ;
extern bool PrintWithXID ;

namespace Restricted {
	void XidTagSubtree(XID_DOMDocument *doc, xercesc::DOMNode* node);
} ;
 

class xydeltaParseHandler : public xercesc::DOMErrorHandler {
public:
	void warning(const xercesc::SAXParseException& e);
  void error(const xercesc::SAXParseException& e);
  void fatalError(const xercesc::SAXParseException& e);
  void resetErrors() {};
  bool handleError(const xercesc::DOMError& domError);
} ;

#endif
