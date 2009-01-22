#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/dom/DOMException.hpp"

#include "XyDiff/DeltaException.hpp"
#include "XyDiff/include/XID_DOMDocument.hpp"
#include "XyDiff/include/XID_map.hpp"
#include "XyDiff/Tools.hpp"
#include "XyDiff/include/XyLatinStr.hpp"

#include <stdio.h>
#include <fstream>
#include <string>

#include "xercesc/dom/DOMNode.hpp"
#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/util/XMLString.hpp"

#include <stdarg.h>

class CompareFailed {
	public:
		CompareFailed(std::string &_msg);
		std::string getMsg(void) const;
	private:
		std::string msg ;
	} ;

CompareFailed::CompareFailed(std::string &_msg) {
	msg = _msg;
	};
	
std::string CompareFailed::getMsg(void) const {
	return msg;
	}

class MyMessageEngine {
	public:
		MyMessageEngine(void);
		void setStr(const char *format, ...);
		char* getStr(void);
	private:
		char s[5000];
	};

MyMessageEngine::MyMessageEngine(void) {
	s[0]='\0';
	}

void MyMessageEngine::setStr(const char *format, ...) {
	va_list var_arg ;
	va_start (var_arg, format);
	vsnprintf(s, 4999, format, var_arg);
	va_end(var_arg);
	};
char* MyMessageEngine::getStr(void) {
	return s;
	};

#define FAILED(why) { \
	MyMessageEngine __macroLocalVariable__MyMessageEngine; \
	__macroLocalVariable__MyMessageEngine.setStr why ; \
	std::string __s=__macroLocalVariable__MyMessageEngine.getStr(); \
	throw CompareFailed(__s); \
	}
	
class XMLCompareEngine {
	public:
		enum XMLCompareOptions {
		          XMLCMP_IgnoreWhiteSpaces = 0x1,
		          XMLCMP_UnorderedTree     = 0x2,
							XMLCMP_IgnoreAttributes  = 0x4,
							XMLCMP_IgnoreComments    = 0x8 } ;

		XMLCompareEngine(int option=XMLCMP_IgnoreWhiteSpaces+XMLCMP_IgnoreComments);

		void compare(XID_DOMDocument *d1, XID_DOMDocument *d2);
		void compare(xercesc_2_2::DOMNode *n1, xercesc_2_2::DOMNode *n2);
		
		static void compareAttributes(xercesc_2_2::DOMNamedNodeMap *attributes1, xercesc_2_2::DOMNamedNodeMap *attributes2);
		static xercesc_2_2::DOMNode* skipWhiteSpacesStartingAt(xercesc_2_2::DOMNode *n);
		
	
	private:
		int flags ;
  };

XMLCompareEngine::XMLCompareEngine(int option) {
	if (option&XMLCMP_UnorderedTree) {
    THROW_AWAY(("unordered XML comparaison not implemented in this version"));
    }
	flags = option ;
  }

void XMLCompareEngine::compareAttributes(xercesc_2_2::DOMNamedNodeMap *attributes1, xercesc_2_2::DOMNamedNodeMap *attributes2) {
	if (attributes1->getLength()!=attributes2->getLength()) {
		FAILED(("both nodes don't have the same number of attributes: left=%d, right=%d",(int)attributes1->getLength(), (int)attributes2->getLength()));
		}
	for(unsigned int i=0; i<attributes1->getLength(); i++) {
		xercesc_2_2::DOMNode *att1 = attributes1->item(i);
		xercesc_2_2::DOMNode *att2 = attributes2->getNamedItem(att1->getNodeName());
		if (att2==NULL) {
			XyLatinStr att1str(att1->getNodeName());
			FAILED(("attribute '%s' not found in second document", att1str.localForm()));
			}
		if (!xercesc_2_2::XMLString::equals(att1->getNodeValue(), att2->getNodeValue())) {
			XyLatinStr att1str(att1->getNodeName());
			XyLatinStr v1(att1->getNodeValue());
			XyLatinStr v2(att2->getNodeValue());
			FAILED(("attribute '%s' has different values in both documents: <%s> vs. <%s>", att1str.localForm(), v1.localForm(), v2.localForm()));
			}
		}
	return;
	}

xercesc_2_2::DOMNode* XMLCompareEngine::skipWhiteSpacesStartingAt(xercesc_2_2::DOMNode *n) {
	xercesc_2_2::DOMNode* t = n ;
	while((t!=NULL)&&(!XID_DOMDocument::isRealData(t))) {
		t = t->getNextSibling();
		}
	return t;
	}

void XMLCompareEngine::compare(XID_DOMDocument *d1, XID_DOMDocument *d2) {
	xercesc_2_2::DOMNode* r1 = d1->getDocumentElement();
	xercesc_2_2::DOMNode* r2 = d2->getDocumentElement();
	try {
		compare(r1, r2);
		}
	catch(const CompareFailed &e) {
		FAILED(("Documents are different: %s", e.getMsg().c_str()));
		}
	return;
	}

void XMLCompareEngine::compare(xercesc_2_2::DOMNode* n1, xercesc_2_2::DOMNode *n2) {
	if ((n1==NULL)&&(n2==NULL)) return;
	if (n1==NULL) FAILED(("first document has no more nodes here"));
	if (n2==NULL) FAILED(("second document has no more nodes here"));
	
	if (n1->getNodeType()!=n2->getNodeType()) FAILED(("nodes don't have the same type here left=%d, right=%d", (int)n1->getNodeType(), (int)n2->getNodeType()));
	
	switch(n1->getNodeType()) {
		
		/* -- Element Node -- */
		
		case xercesc_2_2::DOMNode::ELEMENT_NODE:
		
			if (!xercesc_2_2::XMLString::equals(n1->getNodeName(), n2->getNodeName())) {
				XyLatinStr name1(n1->getNodeName());
				FAILED(("<%s>: second document element has different name", name1.localForm() ));
				}
		
			if (!(flags&XMLCMP_IgnoreAttributes)) {
				xercesc_2_2::DOMNamedNodeMap* attributes1 = n1->getAttributes();
				xercesc_2_2::DOMNamedNodeMap* attributes2 = n2->getAttributes();
				if ((attributes1->getLength()>0)||(attributes2->getLength()>0)) {
					try {
						XMLCompareEngine::compareAttributes(attributes1, attributes2);
						}
					catch (const CompareFailed &e){
						XyLatinStr name1(n1->getNodeName());
						FAILED(("<%s>: %s", name1.localForm(), e.getMsg().c_str()));
						}
					}
				}
			
			if ((!n1->hasChildNodes())&&(!n2->hasChildNodes())) return;
			else if (!n1->hasChildNodes()) {
				XyLatinStr name1(n1->getNodeName());
				FAILED(("<%s>: second document has child node(s)", name1.localForm() ));
				}
			else if (!n2->hasChildNodes()) {
				XyLatinStr name1(n1->getNodeName());
				FAILED(("<%s>: second document has no child node", name1.localForm() ));
				}
			else {
				int childCount=1;
				xercesc_2_2::DOMNode* child1 = n1->getFirstChild();
				xercesc_2_2::DOMNode* child2 = n2->getFirstChild();
				if (flags&XMLCMP_IgnoreWhiteSpaces) {
					child1 = skipWhiteSpacesStartingAt(child1);
					child2 = skipWhiteSpacesStartingAt(child2);
					}
				while(child1!=NULL) {
					if (child2==NULL) {
						XyLatinStr name1(n1->getNodeName());
						FAILED(("<%s>: second document has not %d child nodes", name1.localForm(), (int)childCount ));
						}
					
					try {
						compare(child1, child2);
						}
					catch(const CompareFailed &e) {
						XyLatinStr name1(n1->getNodeName());
						const char *elName = name1.localForm();
						printf("elName=%s\n", elName);
						const char *msg = e.getMsg().c_str();
						printf("{msg=%s}\n", msg);
						FAILED(("/%s[position=%d]%s", elName, (int)childCount, msg ));
						}
					
					child1 = child1->getNextSibling();
					child2 = child2->getNextSibling();
					if (flags&XMLCMP_IgnoreWhiteSpaces) {
						child1 = skipWhiteSpacesStartingAt(child1);
						child2 = skipWhiteSpacesStartingAt(child2);
						}
					childCount++;
					}
				if (child2!=NULL) {
					XyLatinStr name1(n1->getNodeName());
					FAILED(("<%s>: second document has more than %d child nodes", name1.localForm(), childCount ));
					}
				else return;
			}
			THROW_AWAY(("internal error"));
			break;
			
		/* -- Text Node -- */
		
		case xercesc_2_2::DOMNode::TEXT_NODE:
		case xercesc_2_2::DOMNode::CDATA_SECTION_NODE:
			if ( (!XID_DOMDocument::isRealData(n1))
			   &&(!XID_DOMDocument::isRealData(n2))
				 &&(flags & XMLCMP_IgnoreWhiteSpaces) ) return;
			if (!xercesc_2_2::XMLString::equals(n1->getNodeValue(), n2->getNodeValue())) {
				XyLatinStr v1(n1->getNodeValue());
				XyLatinStr v2(n2->getNodeValue());
				FAILED(("text value is different: <%s> vs. <%s>", v1.localForm(), v2.localForm()));
				}
			return;

		case xercesc_2_2::DOMNode::COMMENT_NODE:
			if (flags&XMLCMP_IgnoreComments) return;
			else if (!xercesc_2_2::XMLString::equals(n1->getNodeValue(), n2->getNodeValue())) {
				XyLatinStr v1(n1->getNodeValue());
				XyLatinStr v2(n2->getNodeValue());
				FAILED(("comment text is different: <%s> vs. <%s>", v1.localForm(), v2.localForm()));
				}
			return;

		/* -- Other Types -- */

		case xercesc_2_2::DOMNode::ENTITY_REFERENCE_NODE:
			THROW_AWAY(("Unsupported node type Entity - can't compare"));
			break;

		default:
			THROW_AWAY(("Unknown node type %d - can't compare", (int)n1->getNodeType() ));
			break;
		
		}
	}

int main(int argc, char **argv) {
  
	if (argc<3) {
		std::cerr << "usage: exec file1.xml file2.xml" << std::endl ;
		exit(-1);
	}

	try {
		xercesc_2_2::XMLPlatformUtils::Initialize();
	}
	catch(const xercesc_2_2::XMLException& toCatch) {
		std::cerr << "Error during Xerces-c Initialization.\n"
		          << "  Exception message:" << XyLatinStr(toCatch.getMessage()).localForm() << std::endl;
		exit(-1);
	}
	
	try {
		printf("Opening file <%s>\n", argv[1]);
		XID_DOMDocument* d1 = new XID_DOMDocument(argv[1], false);
    
		printf("Opening file <%s>\n", argv[2]);
		XID_DOMDocument* d2 = new XID_DOMDocument(argv[2], false);
		
		XMLCompareEngine cmp ;
		try {
			cmp.compare(d1, d2);
			printf("Documents <%s> and <%s> are identical\n", argv[1], argv[2]);
			exit(0);
		}
		catch(const CompareFailed &e) {
			printf("Compare FAILED: %s\n", e.getMsg().c_str());
			exit(-1);
		}
	}
	catch(const VersionManagerException &e ) {
		std::cerr << e << std::endl ;
		exit(-1);
	}
	catch(const xercesc_2_2::DOMException &e ) {
		std::cerr << "DOMException, code=" << e.code << std::endl ;
		std::cerr << "DOMException, message=" << e.msg << std::endl ;
		exit(-1);
	}	
	std::cout << "Terminated." << std::endl ;
	return 0;
}

