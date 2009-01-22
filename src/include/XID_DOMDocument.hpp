#ifndef XID_DOM_DOCUMENT
#define XID_DOM_DOCUMENT

#include "XyDiff/include/XID_map.hpp"

#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/dom/DOMBuilder.hpp"
//DLN#include "xercesc/dom/deprecated/DOMParser.hpp"
#include <iostream>

class DTDValidator;
//class DOMParser;

class XID_DOMDocument : public xercesc_2_2::DOMDocument {
	private:
		// not allowed, not implemented
		XID_DOMDocument(); 
		XID_DOMDocument(const XID_DOMDocument &);
		XID_DOMDocument & operator = (const XID_DOMDocument &);

	public:
		// Constructors
		
		static XID_DOMDocument* createDocument(void);
		static XID_DOMDocument* copy(const XID_DOMDocument *doc, bool withXID=true);
		XID_DOMDocument(xercesc_2_2::DOMDocument *doc, const char *xidmapStr=NULL, bool adoptDocument=false);
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
		xercesc_2_2::DOMDocument * getDOMDocumentOwnership();

		static int getSubtreeNodeCount(xercesc_2_2::DOMNode *node);
		int getDocumentNodeCount() ; //int getNodeCount();

		static bool isRealData(xercesc_2_2::DOMNode *node);
  
		xercesc_2_2::DOMNodeIterator * createNodeIterator(xercesc_2_2::DOMNode *root, long unsigned int whatToShow, xercesc_2_2::DOMNodeFilter *filter, bool entityReferenceExpansion);
		/*
		  {
			return xercesc_2_2::DOMDocumentTraversal::createNodeIterator 
			(root, whatToShow, filter, entityReferenceExpansion);
		  }
		*/
  
		const XMLCh * getNodeName() const;
		const XMLCh * getNodeValue() const;
		short int getNodeType() const;
		xercesc_2_2::DOMNode * getParentNode() const;
		xercesc_2_2::DOMNodeList * getChildNodes() const;
		xercesc_2_2::DOMNode * getFirstChild() const;
		xercesc_2_2::DOMNode * getLastChild() const;
		xercesc_2_2::DOMNode * getPreviousSibling() const;
		xercesc_2_2::DOMNode * getNextSibling() const;
		xercesc_2_2::DOMNamedNodeMap * getAttributes() const;
		xercesc_2_2::DOMDocument * getOwnerDocument() const;
		xercesc_2_2::DOMNode * cloneNode(bool deep) const;
		xercesc_2_2::DOMNode * insertBefore(xercesc_2_2::DOMNode *node1, xercesc_2_2::DOMNode *node2);
		xercesc_2_2::DOMNode * replaceChild(xercesc_2_2::DOMNode *node1, xercesc_2_2::DOMNode *node2);
		xercesc_2_2::DOMNode * removeChild(xercesc_2_2::DOMNode *node);
		xercesc_2_2::DOMNode * appendChild(xercesc_2_2::DOMNode *node);
		bool hasChildNodes() const;
		void setNodeValue(const XMLCh *nodeValue);
		void normalize();
		bool isSupported(const XMLCh *feature, const XMLCh *version) const;
		const XMLCh * getNamespaceURI() const;
		const XMLCh * getPrefix() const;
		const XMLCh * getLocalName() const;
		void setPrefix(const XMLCh *prefix);
		bool hasAttributes() const;
		bool isSameNode(const xercesc_2_2::DOMNode *node) const;
		bool isEqualNode(const xercesc_2_2::DOMNode *node) const;
		void * setUserData(const XMLCh *key, void *data, xercesc_2_2::DOMUserDataHandler *handler);
		void * getUserData(const XMLCh *key) const;
		const XMLCh * getBaseURI() const;
		short int compareTreePosition(const xercesc_2_2::DOMNode *node) const;
		const XMLCh * getTextContent() const;
		void setTextContent(const XMLCh *textContent);
		const XMLCh * lookupNamespacePrefix(const XMLCh *namespaceURI, bool useDefault) const;
		bool isDefaultNamespace(const XMLCh *namespaceURI) const;
		const XMLCh * lookupNamespaceURI(const XMLCh *prefix) const;
		class xercesc_2_2::DOMNode * getInterface(const XMLCh *feature);
		void release();
		xercesc_2_2::DOMRange * createRange();
		xercesc_2_2::DOMElement * createElement(const XMLCh *tagName);
		xercesc_2_2::DOMDocumentFragment * createDocumentFragment();
		xercesc_2_2::DOMText * createTextNode(const XMLCh *data);
		xercesc_2_2::DOMComment * createComment(const XMLCh *data);
		xercesc_2_2::DOMCDATASection * createCDATASection(const XMLCh *data);
		xercesc_2_2::DOMProcessingInstruction * createProcessingInstruction(const XMLCh *target, const XMLCh *data);
		xercesc_2_2::DOMAttr * createAttribute(const XMLCh *name);
		xercesc_2_2::DOMEntityReference * createEntityReference(const XMLCh *name);
		xercesc_2_2::DOMDocumentType * getDoctype() const;
		xercesc_2_2::DOMImplementation *getImplementation() const;
		xercesc_2_2::DOMElement * getDocumentElement() const;
		xercesc_2_2::DOMNodeList * getElementsByTagName(const XMLCh *tagName) const;
		xercesc_2_2::DOMNode * importNode(xercesc_2_2::DOMNode *importNode, bool deep);
		xercesc_2_2::DOMElement * createElementNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName);
		xercesc_2_2::DOMAttr * createAttributeNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName);
		xercesc_2_2::DOMNodeList * getElementsByTagNameNS(const XMLCh *namespaceURI, const XMLCh *localName) const;
		xercesc_2_2::DOMElement * getElementById(const XMLCh *elementId) const;
		const XMLCh * getActualEncoding() const;
		void setActualEncoding(const XMLCh *actualEncoding);
		const XMLCh * getEncoding() const;
		void setEncoding(const XMLCh *encoding);
		bool getStandalone() const;
		void setStandalone(bool standalone);
		const XMLCh * getVersion() const;
		void setVersion(const XMLCh *version);
		const XMLCh * getDocumentURI() const;
		void setDocumentURI(const XMLCh *documentURI);

		bool getStrictErrorChecking() const;
		void setStrictErrorChecking(bool strictErrorChecking);
		xercesc_2_2::DOMErrorHandler * getErrorHandler() const;
		void setErrorHandler(xercesc_2_2::DOMErrorHandler *handler);
		xercesc_2_2::DOMNode * renameNode(xercesc_2_2::DOMNode *n, const XMLCh *namespaceURI, const XMLCh *name);
		xercesc_2_2::DOMNode * adoptNode(xercesc_2_2::DOMNode *node);
		void normalizeDocument();
		bool canSetNormalizationFeature(const XMLCh *name, bool state) const;
		void setNormalizationFeature(const XMLCh *name, bool state);
		bool getNormalizationFeature(const XMLCh *name) const;
		xercesc_2_2::DOMEntity * createEntity(const XMLCh *name);
		xercesc_2_2::DOMDocumentType * createDocumentType(const XMLCh *name);
		xercesc_2_2::DOMNotation * createNotation(const XMLCh *name);
		xercesc_2_2::DOMElement * createElementNS(const XMLCh *namespaceURI, const XMLCh *qualifiedName, const XMLSSize_t lineNum, const XMLSSize_t columnNum);
  
		xercesc_2_2::DOMTreeWalker * createTreeWalker(xercesc_2_2::DOMNode *root, long unsigned int whatToShow, xercesc_2_2::DOMNodeFilter *filter, bool entityReferenceExpansion);


	private:		
		XID_map *xidmap;
		xercesc_2_2::DOMDocument* theDocument;
		xercesc_2_2::DOMBuilder*  theParser;
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
	void XidTagSubtree(XID_DOMDocument *doc, xercesc_2_2::DOMNode* node);
} ;
  
#endif
