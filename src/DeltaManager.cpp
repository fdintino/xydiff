
#include "DeltaManager.hpp"

#include "Tools.hpp"
#include "xydiff/XyLatinStr.hpp"
#include "xydiff/XyUTF8Str.hpp"
#include "DOMPrint.hpp"

//#include <util/PlatformUtils.hpp>
#include "xercesc/dom/DOMException.hpp"
#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/dom/DOMImplementationRegistry.hpp"
#include "xercesc/dom/DOMImplementation.hpp"
#include "xercesc/util/XMLUniDefs.hpp"

#include <stdio.h>
#include <set>

#include "xydiff/XyDiffNS.hpp"
using namespace XyDiff;

XERCES_CPP_NAMESPACE_USE

static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
using namespace std;

DeltaHeader::DeltaHeader(DOMNode *deltaElement) {
	if (deltaElement->getNodeType() != DOMNode::ELEMENT_NODE) {
		throw VersionManagerException("Data Error", "DeltaHeader -> DeltaHeader()", "the argument (deltaElement) is not a DOM_Element");
	}

        //DOMString domstr;
	DOMNode* node;
	 
	node = deltaElement->getAttributes()->getNamedItem(XMLString::transcode("fromVersionId"));
	if (node == NULL)
		throw VersionManagerException("Data Error", "DeltaHeader -> DeltaHeader()", "the element node doesn't have the attribute <fromVersionId>");

        fromVersionId = watoi(node->getNodeValue());
	
	node = deltaElement->getAttributes()->getNamedItem(XMLString::transcode("toVersionId"));

	if (node == NULL) 
		throw VersionManagerException("Data Error", "DeltaHeader -> DeltaHeader()", "the element node doesn't have the attribute <toVersionId>");

        toVersionId = watoi(node->getNodeValue());

	node = deltaElement->getAttributes()->getNamedItem(XMLString::transcode("fromDate"));
	
	if (node == NULL) 
		throw VersionManagerException("Data Error", "DeltaHeader -> DeltaHeader()", "the element node doesn't have the attribute <fromDate>");

        fromDate = XyLatinStr(node->getNodeValue());
    
	node = deltaElement->getAttributes()->getNamedItem(XMLString::transcode("toDate"));
	if (node == NULL) 
		throw VersionManagerException("Data Error", "DeltaHeader -> DeltaHeader()", "the element node doesn't have the attribute <toDate>");

        toDate = XyLatinStr(node->getNodeValue());

	cout << "fromVersionId=" << fromVersionId << " toVersionId=" << toVersionId << endl;
	cout << "fromDate=" << fromDate << " toDate=" << toDate << endl;
}


DeltaHeader::VersionId_t DeltaHeader::getFromVersionId() {
	return fromVersionId;
}


DeltaHeader::VersionId_t DeltaHeader::getToVersionId() {
	return toVersionId;
}

std::string DeltaHeader::getFromDate() {
	return fromDate;
}

std::string DeltaHeader::getToDate() {
	return toDate;
}




// loads the delta XID_DOMDocument from the delta file 
// in file should be a well formed delta document 
// should be a delta root <delta_unit>, a tag <info> with 2 attributes and and 0 or more delta elements <t>
DeltaManager::DeltaManager(const char *fileName) {
	
    if (!existsFile(fileName)) 
		throw VersionManagerException("Data error", "DeltaManager constructor", "the delta file doesn't exists");
	
	setFileName(fileName);
	deltaDocument = new XID_DOMDocument(fileName, false) ;
	
	DOMElement* deltaRoot = deltaDocument->getDocumentElement() ;
    if (deltaRoot==NULL) 
		throw VersionManagerException("Data Error", "DeltaManager", "deltaRoot is NULL");
	if (!deltaRoot->hasChildNodes())
		throw VersionManagerException("Data Error", "DeltaManager", "deltaRoot has no child nodes");
	
	// verify if the child nodes are element nodes
	DOMNode* node = deltaRoot->getFirstChild();
	//cout << XyLatinStr(node.getNodeName())<< " " <<  XyLatinStr(node.getNodeValue()) << endl;
	if ((node->getNodeType() != DOMNode::ELEMENT_NODE) || (node->getNodeName() != XMLString::transcode("info")) ) 
		throw VersionManagerException("Data error", "DeltaManager", "incorrect input delta document: info tag missing");

	for ( node = node->getNextSibling(); node != NULL; node = node->getNextSibling()) {
		// cout << XyLatinStr(node->getNodeName()) << " " <<  XyLatinStr(node->getNodeValue()) << endl;
		if ((node->getNodeType() != DOMNode::ELEMENT_NODE) || (!XMLString::equals(node->getLocalName(),XMLString::transcode("t")))) 
			throw VersionManagerException("Data error", "DeltaManager", "incorrect input delta document: tags");
		headerList.insert(headerList.begin(), DeltaHeader(node));
	}
	
	node = deltaRoot->getFirstChild()->getAttributes()->getNamedItem(XMLString::transcode("firstAvailableVersionId")); 
	if (node == NULL) {
		throw VersionManagerException("Data error", "DeltaManager", "incorrect input delta document: firstAvailableVersionId missing");
	}
        
        firstAvailableVersionId = watoi(node->getNodeValue());

	cout << "firstAvailableVersionId = " << firstAvailableVersionId << endl;

	node = deltaRoot->getFirstChild()->getAttributes()->getNamedItem(XMLString::transcode("currentVersionFileName")); 
	if (node == NULL) {
		throw VersionManagerException("Data error", "DeltaManager", "incorrect input delta document: currentVersionFileName missing");
	}
        currentVersionFileName = std::string(XyLatinStr(node->getNodeValue()));
	cout << "currentVersionFileName = " << currentVersionFileName << endl;
   
}


// creates a new delta document
DeltaManager::DeltaManager(const char *deltaFileName, const char *newVersionFileName ) {
	
	if (existsFile(deltaFileName)) 
			throw VersionManagerException("Data error", "DeltaManager constructor", "the delta file already exists");

	setFileName(deltaFileName);

        DOMImplementation* impl =  DOMImplementationRegistry::getDOMImplementation(gLS);
        
	deltaDocument = new XID_DOMDocument(impl->createDocument(
			XYDIFF_XYDELTA_NS,                     // root element namespace URI.
			XMLString::transcode("xy:unit_delta"), // root element name
			0));  // document type object (DTD)

	DOMElement* deltaRoot = deltaDocument->getDocumentElement();
	DOMElement*  infoElem = deltaDocument->createElement(XMLString::transcode("info"));
    deltaRoot->appendChild(infoElem);

	infoElem->setAttribute(XMLString::transcode("firstAvailableVersionId"), XMLString::transcode("1"));
	infoElem->setAttribute(XMLString::transcode("currentVersionFileName"), XyLatinStr(newVersionFileName));

	firstAvailableVersionId = 1;
	currentVersionFileName = newVersionFileName;

	cout << deltaDocument << endl;
}


// add a delta element to a delta document. The version ID's are found inside the delta. Return 0 if ok, negative code if not.
int DeltaManager::addDeltaElement(DOMNode *deltaElement, const char *versionFile) {
	DOMNode* deltaroot = deltaDocument->getDocumentElement(); // unit_delta
    
	//  verify if the argument is an element node
	if (deltaElement->getNodeType()!=DOMNode::ELEMENT_NODE) { 
		cout << "Error: wrong node type" << endl;
		return(-1);
	}

    
	// adds attributes fromVersionId and toVersionId
	// and also fromDate and toDate
    DOMElement* element = (DOMElement*) deltaElement; 
    char buf[32];
	sprintf(buf, "%d", firstAvailableVersionId++); 
	element->setAttribute(XMLString::transcode("fromVersionId"), XyLatinStr(buf) );
	sprintf(buf, "%d", firstAvailableVersionId); 
    element->setAttribute(XMLString::transcode("toVersionId"), XyLatinStr(buf) );

	// **************** provizoriu 
	element->setAttribute(XMLString::transcode("fromDate"), XMLString::transcode(" "));
	element->setAttribute(XMLString::transcode("toDate"), XMLString::transcode(" "));


	// adds the delta element to the delta document
	DOMNode* node = deltaDocument->importNode(element,true);
	deltaroot->appendChild(node);
	
	headerList.insert(headerList.begin(), DeltaHeader(node));

	// update currentVersionFileName and firstAvailableVersionId into the delta document
	
	currentVersionFileName = versionFile;
	node = deltaroot->getFirstChild();

		//DOM_Node* pNode = &node;
		//DOM_Element *pElem = (DOM_Element*) pNode;
		//DOM_Element elem = *pElem;
		// sau asa:
    DOMElement* elem = (DOMElement*)node ;

	char id[32];
	sprintf(id, "%d", firstAvailableVersionId);
	elem->setAttribute(XMLString::transcode("firstAvailableVersionId"), XyLatinStr(id));
	elem->setAttribute(XMLString::transcode("currentVersionFileName"), XyLatinStr(versionFile));

	return(0);
}


// returns NULL if that delta element does not exist
DOMNode* DeltaManager::getDeltaElement(DeltaHeader::VersionId_t fromVersionId, DeltaHeader::VersionId_t toVersionId) {
	DOMNode* deltaroot = deltaDocument->getDocumentElement();
	if (toVersionId == -1) {
		toVersionId = fromVersionId + 1;
	}
	DOMNode* node ;
	for (node = deltaroot->getFirstChild(); node != NULL; node = node->getNextSibling()) {
		DeltaHeader dh = DeltaHeader(node);
		if ( (dh.getFromVersionId() == fromVersionId) && 
			 (dh.getToVersionId() == toVersionId) ) {
			return node;
		}
	}
	return NULL;
}



const std::vector<DeltaHeader>& DeltaManager::getDeltaList(){	
	return headerList;
}


DOMDocument* DeltaManager::getDeltaDocument() {
	return deltaDocument;
}

std::string DeltaManager::getCurrentVersionFileName() {
	return currentVersionFileName;
}


void DeltaManager::listAllDocumentVersions() {
	std::set< int, std::less<int> > ids;
	std::vector<DeltaHeader>::iterator p; 
	for ( p = headerList.begin(); p != headerList.end(); p++ ) {
    	ids.insert(p->getFromVersionId());
		ids.insert(p->getToVersionId());
	}

	cout << "Document versions: "; 
	std::set<int>::iterator ps;
	for ( ps = ids.begin(); ps != ids.end(); ps++) {
		cout << *ps << " "; 
	}
	cout << endl;
}

		
void DeltaManager::listAllDeltas() {
	DOMNode* deltaroot = deltaDocument->getDocumentElement();
    DOMNode* node ;
	for (node = deltaroot->getFirstChild(); node != NULL; node = node->getNextSibling()) {
        cout << node << endl;
	}
}

void DeltaManager::setFileName(const char *filename) {
	if ( (filename ==  NULL) || (!strcmp(filename,"")) ) {
        throw VersionManagerException("Data Error", "DeltaManager", "invalid delta filename");
	}
	else {
		std::string tmpstr(filename);
		deltaFileName = tmpstr;
	}
}

void DeltaManager::SaveToDisk() {
	if (deltaFileName == "") {
		throw VersionManagerException("Data Error", "DeltaManager", "invalid delta filename");
	}
	deltaDocument->SaveAs(deltaFileName.c_str(),false);
}

