
#include "DeltaManager.hpp"

#include "Tools.hpp"
#include "include/XyLatinStr.hpp"
#include "include/XyUTF8Str.hpp"
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

static const XMLCh gLS[] = { xercesc_3_0::chLatin_L, xercesc_3_0::chLatin_S, xercesc_3_0::chNull };

using namespace std;

DeltaHeader::DeltaHeader(xercesc_3_0::DOMNode *deltaElement) {
	if (deltaElement->getNodeType() != xercesc_3_0::DOMNode::ELEMENT_NODE) {
		throw VersionManagerException("Data Error", "DeltaHeader -> DeltaHeader()", "the argument (deltaElement) is not a DOM_Element");
	}

        //xercesc_3_0::DOMString domstr;
	xercesc_3_0::DOMNode* node;
	 
	node = deltaElement->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("fromVersionId"));
	if (node == NULL)
		throw VersionManagerException("Data Error", "DeltaHeader -> DeltaHeader()", "the element node doesn't have the attribute <fromVersionId>");

        fromVersionId = watoi(node->getNodeValue());
	
	node = deltaElement->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("toVersionId"));

	if (node == NULL) 
		throw VersionManagerException("Data Error", "DeltaHeader -> DeltaHeader()", "the element node doesn't have the attribute <toVersionId>");

        toVersionId = watoi(node->getNodeValue());

	node = deltaElement->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("fromDate"));
	
	if (node == NULL) 
		throw VersionManagerException("Data Error", "DeltaHeader -> DeltaHeader()", "the element node doesn't have the attribute <fromDate>");

        fromDate = XyLatinStr(node->getNodeValue());
    
	node = deltaElement->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("toDate"));
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
	
	xercesc_3_0::DOMElement* deltaRoot = deltaDocument->getDocumentElement() ;
    if (deltaRoot==NULL) 
		throw VersionManagerException("Data Error", "DeltaManager", "deltaRoot is NULL");
	if (!deltaRoot->hasChildNodes())
		throw VersionManagerException("Data Error", "DeltaManager", "deltaRoot has no child nodes");
	
	// verify if the child nodes are element nodes
	xercesc_3_0::DOMNode* node = deltaRoot->getFirstChild();
	//cout << XyLatinStr(node.getNodeName())<< " " <<  XyLatinStr(node.getNodeValue()) << endl;
	if ((node->getNodeType() != xercesc_3_0::DOMNode::ELEMENT_NODE) || (node->getNodeName() != xercesc_3_0::XMLString::transcode("info")) ) 
		throw VersionManagerException("Data error", "DeltaManager", "incorrect input delta document: info tag missing");

	for ( node = node->getNextSibling(); node != NULL; node = node->getNextSibling()) {
		// cout << XyLatinStr(node->getNodeName()) << " " <<  XyLatinStr(node->getNodeValue()) << endl;
		if ((node->getNodeType() != xercesc_3_0::DOMNode::ELEMENT_NODE) || (!xercesc_3_0::XMLString::equals(node->getNodeName(),xercesc_3_0::XMLString::transcode("t")))) 
			throw VersionManagerException("Data error", "DeltaManager", "incorrect input delta document: tags");
		headerList.insert(headerList.begin(), DeltaHeader(node));
	}
	
	node = deltaRoot->getFirstChild()->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("firstAvailableVersionId")); 
	if (node == NULL) {
		throw VersionManagerException("Data error", "DeltaManager", "incorrect input delta document: firstAvailableVersionId missing");
	}
        
        firstAvailableVersionId = watoi(node->getNodeValue());

	cout << "firstAvailableVersionId = " << firstAvailableVersionId << endl;

	node = deltaRoot->getFirstChild()->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("currentVersionFileName")); 
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

        xercesc_3_0::DOMImplementation* impl =  xercesc_3_0::DOMImplementationRegistry::getDOMImplementation(gLS);
        
	deltaDocument = new XID_DOMDocument(impl->createDocument(
			0,                    // root element namespace URI.
			xercesc_3_0::XMLString::transcode("unit_delta"),            // root element name
			0));  // document type object (DTD)

	xercesc_3_0::DOMElement* deltaRoot = deltaDocument->getDocumentElement();
	xercesc_3_0::DOMElement*  infoElem = deltaDocument->createElement(xercesc_3_0::XMLString::transcode("info"));
    deltaRoot->appendChild(infoElem);

	infoElem->setAttribute(xercesc_3_0::XMLString::transcode("firstAvailableVersionId"), xercesc_3_0::XMLString::transcode("1"));
	infoElem->setAttribute(xercesc_3_0::XMLString::transcode("currentVersionFileName"), XyLatinStr(newVersionFileName));

	firstAvailableVersionId = 1;
	currentVersionFileName = newVersionFileName;

	cout << deltaDocument << endl;
}


// add a delta element to a delta document. The version ID's are found inside the delta. Return 0 if ok, negative code if not.
int DeltaManager::addDeltaElement(xercesc_3_0::DOMNode *deltaElement, const char *versionFile) {
	xercesc_3_0::DOMNode* deltaroot = deltaDocument->getDocumentElement(); // unit_delta
    
	//  verify if the argument is an element node
	if (deltaElement->getNodeType()!=xercesc_3_0::DOMNode::ELEMENT_NODE) { 
		cout << "Error: wrong node type" << endl;
		return(-1);
	}

    
	// adds attributes fromVersionId and toVersionId
	// and also fromDate and toDate
    xercesc_3_0::DOMElement* element = (xercesc_3_0::DOMElement*) deltaElement; 
    char buf[32];
	sprintf(buf, "%d", firstAvailableVersionId++); 
	element->setAttribute(xercesc_3_0::XMLString::transcode("fromVersionId"), XyLatinStr(buf) );
	sprintf(buf, "%d", firstAvailableVersionId); 
    element->setAttribute(xercesc_3_0::XMLString::transcode("toVersionId"), XyLatinStr(buf) );

	// **************** provizoriu 
	element->setAttribute(xercesc_3_0::XMLString::transcode("fromDate"), xercesc_3_0::XMLString::transcode(" "));
	element->setAttribute(xercesc_3_0::XMLString::transcode("toDate"), xercesc_3_0::XMLString::transcode(" "));


	// adds the delta element to the delta document
	xercesc_3_0::DOMNode* node = deltaDocument->importNode(element,true);
	deltaroot->appendChild(node);
	
	headerList.insert(headerList.begin(), DeltaHeader(node));

	// update currentVersionFileName and firstAvailableVersionId into the delta document
	
	currentVersionFileName = versionFile;
	node = deltaroot->getFirstChild();

		//DOM_Node* pNode = &node;
		//DOM_Element *pElem = (DOM_Element*) pNode;
		//DOM_Element elem = *pElem;
		// sau asa:
    xercesc_3_0::DOMElement* elem = (xercesc_3_0::DOMElement*)node ;

	char id[32];
	sprintf(id, "%d", firstAvailableVersionId);
	elem->setAttribute(xercesc_3_0::XMLString::transcode("firstAvailableVersionId"), XyLatinStr(id));
	elem->setAttribute(xercesc_3_0::XMLString::transcode("currentVersionFileName"), XyLatinStr(versionFile));

	return(0);
}


// returns NULL if that delta element does not exist
xercesc_3_0::DOMNode* DeltaManager::getDeltaElement(DeltaHeader::VersionId_t fromVersionId, DeltaHeader::VersionId_t toVersionId) {
	xercesc_3_0::DOMNode* deltaroot = deltaDocument->getDocumentElement();
	if (toVersionId == -1) {
		toVersionId = fromVersionId + 1;
	}
	xercesc_3_0::DOMNode* node ;
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


xercesc_3_0::DOMDocument* DeltaManager::getDeltaDocument() {
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
	xercesc_3_0::DOMNode* deltaroot = deltaDocument->getDocumentElement();
    xercesc_3_0::DOMNode* node ;
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

