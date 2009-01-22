#ifndef XID_DOM_DOCUMENT
#define XID_DOM_DOCUMENT

#include "include/XID_map.hpp"

#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/dom/DOMConfiguration.hpp"
//#include "xercesc/dom/DOMBuilder.hpp"
#include "xercesc/dom/DOMError.hpp"
#include "xercesc/dom/DOMErrorHandler.hpp"
#include "xercesc/dom/DOMLSParser.hpp"
//DLN#include "xercesc/dom/deprecated/DOMParser.hpp"
#include <iostream>

class DTDValidator;
//class DOMParser;

class XID_DOMDocument : public xercesc_3_0::DOMDocument {
	private:
		// not allowed, not implemented
		XID_DOMDocument(); 
		XID_DOMDocument(const XID_DOMDocument &);
		XID_DOMDocument & operator = (const XID_DOMDocument &);

	public:
		// Constructors
		
		static XID_DOMDocument* createDocument(void);
		static XID_DOMDocument* copy(const XID_DOMDocument *doc, bool withXID=true);
		XID_DOMDocument(xercesc_3_0::DOMDocument *doc, const char *xidmapStr=NULL, bool adoptDocument=false);
		XID_DOMDocument(const char* xmlfile, bool useXidMap=true, bool doValidation=false);
		
		//XID_DOMDocument();
		//XID_DOMDocument(const XID_DOMDocument& doc, bool withXID=true );
		//XID_DOMDocument & operator = (const XID_DOMDocument &other);

		void SaveAs(const char *xml_filename, bool saveXidMap=true);
		~XID_DOMDocument();

		// Methods
		
		void addXidMapFile(const char *xidmapFilename);
		int addXidMap(const char *theXidmap=NULL);
		void initEmptyXidmapStartingAt(XID_t firstAvailableXid);

		XID_map &getXidMap();
		xercesc_3_0::DOMDocument * getDOMDocumentOwnership();

		static int getSubtreeNodeCount(xercesc_3_0::DOMNode *node);
		int getDocumentNodeCount() ; //int getNodeCount();

		static bool isRealData(xercesc_3_0::DOMNode *node);
  
		xercesc_3_0::DOMNodeIterator * createNodeIterator(xercesc_3_0::DOMNode *root, long unsigned int whatToShow, xercesc_3_0::DOMNodeFilter *filter, bool entityReferenceExpansion);
		/*
		  {
			return xercesc_3_0::DOMDocumentTraversal::createNodeIterator 
			(root, whatToShow, filter, entityReferenceExpansion);
		  }
		*/
  
		const XMLCh * getNodeName() const;
		const XMLCh * getNodeValue() const;
		xercesc_3_0::DOMNode::NodeType getNodeType() const;
		xercesc_3_0::DOMNode * getParentNode() const;
		xercesc_3_0::DOMNodeList * getChildNodes() const;
		xercesc_3_0::DOMNode * getFirstChild() const;
		xercesc_3_0::DOMNode * getLastChild() const;
		xercesc_3_0::DOMNode * getPreviousSibling() const;
		xercesc_3_0::DOMNode * getNextSibling() const;
		xercesc_3_0::DOMNamedNodeMap * getAttributes() const;
		xercesc_3_0::DOMDocument * getOwnerDocument() const;
		xercesc_3_0::DOMNode * cloneNode(bool deep) const;
		xercesc_3_0::DOMNode * insertBefore(xercesc_3_0::DOMNode *node1, xercesc_3_0::DOMNode *node2);
		xercesc_3_0::DOMNode * replaceChild(xercesc_3_0::DOMNode *node1, xercesc_3_0::DOMNode *node2);
		xercesc_3_0::DOMNode * removeChild(xercesc_3_0::DOMNode *node);
		xercesc_3_0::DOMNode * appendChild(xercesc_3_0::DOMNode *node);
		bool hasChildNodes() const;
		void setNodeValue(const XMLCh *nodeValue);
		void normalize();
		bool isSupported(const XMLCh *feature, const XMLCh *version) const;
		const XMLCh * getNamespaceURI() const;
		const XMLCh * getPrefix() const;
		const XMLCh * getLocalName() const;
		void setPrefix(const XMLCh *prefix);
		bool hasAttributes() const;
		bool isSameNode(const xercesc_3_0::DOMNode *node) const;
		bool isEqualNode(const xercesc_3_0::DOMNode *node) const;
		void * setUserData(const XMLCh *key, void *data, xercesc_3_0::DOMUserDataHandler *handler);
		void * getUserData(const XMLCh *key) const;
		const XMLCh * getBaseURI() const;
		// short int compareTreePosition(const xercesc_3_0::DOMNode *node) const;
		const XMLCh * getTextContent() const;
		void setTextContent(const XMLCh *textContent);
		// const XMLCh * lookupNamespacePrefix(const XMLCh *namespaceURI, bool useDefault) const;
		bool isDefaultNamespace(const XMLCh *namespaceURI) const;
		const XMLCh * lookupNamespaceURI(const XMLCh *prefix) const;
		// class xercesc_3_0::DOMNode * getInterface(const XMLCh *feature);
		void release();
		xercesc_3_0::DOMXPathExpression * createExpression(const XMLCh *xpath, const xercesc_3_0::DOMXPathNSResolver *resolver);
		xercesc_3_0::DOMXPathNSResolver * createNSResolver(const xercesc_3_0::DOMNode *node);
		xercesc_3_0::DOMXPathResult * evaluate(const XMLCh *xpath, const xercesc_3_0::DOMNode *node, const xercesc_3_0::DOMXPathNSResolver *resolver, xercesc_3_0::DOMXPathResult::ResultType resultType, xercesc_3_0::DOMXPathResult *result);
		short int compareDocumentPosition(const xercesc_3_0::DOMNode *node) const;
		const XMLCh* lookupPrefix(const XMLCh *prefix) const;
		void* getFeature(const XMLCh *param1, const XMLCh *param2) const;
		xercesc_3_0::DOMNode * importNode(const xercesc_3_0::DOMNode *node, bool trueOrFalse);
		const XMLCh * getInputEncoding() const;
		const XMLCh * getXmlEncoding() const;
		bool getXmlStandalone() const;
		void setXmlStandalone(bool isStandalone);
		const XMLCh * getXmlVersion() const;
		void setXmlVersion(const XMLCh *version);
		xercesc_3_0::DOMConfiguration * getDOMConfig() const;
		xercesc_3_0::DOMElement * createElementNS(const XMLCh *element, const XMLCh *xmlNameSpace, XMLFileLoc fileLoc1, XMLFileLoc fileLoc2);
		//xercesc_3_0::DOMNode * compareDocumentPosition(const xercesc_3_0::DOMNode *node);
		xercesc_3_0::DOMRange * createRange();
		xercesc_3_0::DOMElement * createElement(const XMLCh *tagName);
		xercesc_3_0::DOMDocumentFragment * createDocumentFragment();
		xercesc_3_0::DOMText * createTextNode(const XMLCh *data);
		xercesc_3_0::DOMComment * createComment(const XMLCh *data);
		xercesc_3_0::DOMCDATASection * createCDATASection(const XMLCh *data);
		xercesc_3_0::DOMProcessingInstruction * createProcessingInstruction(const XMLCh *target, const XMLCh *data);
		xercesc_3_0::DOMAttr * createAttribute(const XMLCh *name);
		xercesc_3_0::DOMEntityReference * createEntityReference(const XMLCh *name);
		xercesc_3_0::DOMDocumentType * getDoctype() const;
		xercesc_3_0::DOMImplementation *getImplementation() const;
		xercesc_3_0::DOMElement * getDocumentElement() const;
		xercesc_3_0::DOMNodeList * getElementsByTagName(const XMLCh *tagName) const;
		//xercesc_3_0::DOMNode * importNode(xercesc_3_0::DOMNode *importNode, bool deep);
		xercesc_3_0::DOMElement * createElementNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName);
		xercesc_3_0::DOMAttr * createAttributeNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName);
		xercesc_3_0::DOMNodeList * getElementsByTagNameNS(const XMLCh *namespaceURI, const XMLCh *localName) const;
		xercesc_3_0::DOMElement * getElementById(const XMLCh *elementId) const;
		// const XMLCh * getActualEncoding() const;
		// void setActualEncoding(const XMLCh *actualEncoding);
		// const XMLCh * getEncoding() const;
		// void setEncoding(const XMLCh *encoding);
//		bool getStandalone() const;
//		void setStandalone(bool standalone);
		// const XMLCh * getVersion() const;
		// void setVersion(const XMLCh *version);
		const XMLCh * getDocumentURI() const;
		void setDocumentURI(const XMLCh *documentURI);

		bool getStrictErrorChecking() const;
		void setStrictErrorChecking(bool strictErrorChecking);
		// xercesc_3_0::DOMErrorHandler * getErrorHandler() const;
		// void setErrorHandler(xercesc_3_0::DOMErrorHandler *handler);
		xercesc_3_0::DOMNode * renameNode(xercesc_3_0::DOMNode *n, const XMLCh *namespaceURI, const XMLCh *name);
		xercesc_3_0::DOMNode * adoptNode(xercesc_3_0::DOMNode *node);
		void normalizeDocument();
		// bool canSetNormalizationFeature(const XMLCh *name, bool state) const;
		// void setNormalizationFeature(const XMLCh *name, bool state);
		// bool getNormalizationFeature(const XMLCh *name) const;
		xercesc_3_0::DOMEntity * createEntity(const XMLCh *name);
		xercesc_3_0::DOMDocumentType * createDocumentType(const XMLCh *name);
		xercesc_3_0::DOMNotation * createNotation(const XMLCh *name);
		xercesc_3_0::DOMElement * createElementNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName, const XMLSSize_t lineNum, const XMLSSize_t columnNum);
  
		xercesc_3_0::DOMTreeWalker * createTreeWalker(xercesc_3_0::DOMNode *root, long unsigned int whatToShow, xercesc_3_0::DOMNodeFilter *filter, bool entityReferenceExpansion);


	private:		
		XID_map *xidmap;
		xercesc_3_0::DOMDocument* theDocument;
		xercesc_3_0::DOMLSParser*  theParser;
		bool                      doReleaseTheDocument;
		void parseDOM_Document(const char* xmlfile, bool doValidation);

} ;

class GlobalPrintContext_t {
	public:
		void SetModeDebugXID( XID_map &xidmap ) ;
		void ReleaseContext( void ) ;
} ;

extern class GlobalPrintContext_t globalPrintContext ;
extern bool PrintWithXID ;

namespace Restricted {
	void XidTagSubtree(XID_DOMDocument *doc, xercesc_3_0::DOMNode* node);
} ;
  
#endif
