#include "include/XyDelta_DOMInterface.hpp"

#include "include/XyLatinStr.hpp"
#include "include/XyUTF8Str.hpp"
#include "include/XID_map.hpp"
#include "include/XID_DOMDocument.hpp"

#include "DeltaApply.hpp"
#include "DeltaReverse.hpp"
#include "DOMPrint.hpp"
#include "CommonSubSequenceAlgorithms.hpp"
#include "Tools.hpp"
#include "Diff_NodesManager.hpp"
#include "Diff_DeltaConstructor.hpp"
#include "Diff_UniqueIdHandler.hpp"
#include "DeltaException.hpp"
#include "DeltaManager.hpp"

#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/dom/DOMImplementationRegistry.hpp"
#include "xercesc/dom/DOMImplementation.hpp"
#include "xercesc/validators/DTD/DTDValidator.hpp"

#include "infra/general/Log.hpp"

static const XMLCh gLS[] = { xercesc_3_0::chLatin_L, xercesc_3_0::chLatin_S, xercesc_3_0::chNull };

xercesc_3_0::DOMNode* XyDelta::ReverseDelta(xercesc_3_0::DOMDocument *reversedDeltaDoc, xercesc_3_0::DOMNode *deltaElement) {
	try {
		xercesc_3_0::DOMNode* reversed = DeltaReverse(deltaElement, reversedDeltaDoc);
		return reversed ;
	}
	catch(...) {
          return NULL;
	}
}


XID_DOMDocument* XidXyDiff(XID_DOMDocument* v0XML, const char *doc1name, XID_DOMDocument* v1XML, const char *doc2name, bool ignoreSpacesFlag=false, bool verbose=false, xercesc_3_0::DTDValidator *dtdValidator=NULL);

// called from computeDelta()   in file VersionManager.cpp
xercesc_3_0::DOMDocument* XyDelta::XyDiff(xercesc_3_0::DOMDocument* doc1, const char *doc1name, 
                   xercesc_3_0::DOMDocument* doc2, const char *doc2name, 
                   const char *doc1xidmap) {

	// doc1xidmap can be NULL => the default XidMap (1-n|n+1) will be used
	
	if (doc1==NULL) {
		ERROR("doc1 is NULL");
		return NULL;
	}
	if (doc1name==NULL) {
		ERROR("doc1name is NULL");
		return NULL;
	}
	if (doc2==NULL) {
		ERROR("doc2 is NULL");
		return NULL;
	}
	if (doc2name==NULL) {
		ERROR("doc2 is NULL");
		return NULL;
	}
	
	TRACE("Create XID-DOMDocument version 1");
	
	XID_DOMDocument* v0XML = new XID_DOMDocument(doc1);

	TRACE("Add XidMap to version 1");

	v0XML->addXidMap(doc1xidmap);

	TRACE("Create XID-DOMDocument version 2");

	XID_DOMDocument* v1XML = new XID_DOMDocument(doc2);
	v1XML->initEmptyXidmapStartingAt(v0XML->getXidMap().getFirstAvailableXID());
	
	TRACE("Go XidXyDiff");

	XID_DOMDocument* delta = XidXyDiff(v0XML, doc1name, v1XML, doc2name, false, false, NULL);

        delete v0XML;
	v0XML = NULL;
        delete v1XML;
	v1XML = NULL;

	return (xercesc_3_0::DOMDocument*)delta;
}

void XyDelta::SaveDomDocument(xercesc_3_0::DOMDocument* d, const char *filename) {
	XID_DOMDocument* xd = new XID_DOMDocument(d);
	xd->SaveAs(filename, false);
        delete xd;
}

xercesc_3_0::DOMDocument* XyDelta::ApplyDelta(xercesc_3_0::DOMDocument* doc, xercesc_3_0::DOMNode* deltaElement, bool CreateNewDocumentResult) {
	XID_DOMDocument* xdoc = new XID_DOMDocument(doc);
	XyLatinStr fromXidmap(deltaElement->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("fromXidMap"))->getNodeValue());

	try {
		xdoc->addXidMap(fromXidmap);

		DeltaApplyEngine appliqueDoc(xdoc);
		appliqueDoc.ApplyDeltaElement(deltaElement) ;
	
		doc = xdoc->getDOMDocumentOwnership();
		delete xdoc;
		xdoc=NULL;

		// TO-DO: testXidMap value ?	

                if (CreateNewDocumentResult) {
			xercesc_3_0::DOMImplementation* impl =  xercesc_3_0::DOMImplementationRegistry::getDOMImplementation(gLS);
                	xercesc_3_0::DOMDocument* resultDoc = impl->createDocument();
			xercesc_3_0::DOMNode* resultRoot = resultDoc->importNode(appliqueDoc.getResultDocument()->getDocumentElement(), true);
			resultDoc->appendChild(resultRoot);
			return resultDoc;
		} else {
			return doc;
		}			
	}
	catch(...) {
		ERROR("Unknown Exception catched");
		if (xdoc) {
			doc = xdoc->getDOMDocumentOwnership();
			delete xdoc;
			xdoc=NULL;
		}
		return NULL;
	}
}

unsigned int XyDelta::estimateDocumentSize(xercesc_3_0::DOMDocument *doc) {
	xercesc_3_0::DOMNode* root = doc->getDocumentElement();
	return (XyDelta::estimateSubtreeSize(root)+strlen("<?xml version='1.0' encoding='ISO-8859-1' ?>\n\n"));
	}

unsigned int XyDelta::estimateSubtreeSize(xercesc_3_0::DOMNode *node) {
	if (node==NULL) return 0;

	unsigned int mySize = 0;

	switch(node->getNodeType()) {
		case xercesc_3_0::DOMNode::ELEMENT_NODE: {
			if (node->hasChildNodes()) mySize += 2*xercesc_3_0::XMLString::stringLen(node->getNodeName())+5;
			else mySize += xercesc_3_0::XMLString::stringLen(node->getNodeName())+3;

			xercesc_3_0::DOMNamedNodeMap* attributes = node->getAttributes();
			if (attributes!=NULL) {
				int attrCount = attributes->getLength();
				for (int i = 0; i < attrCount; i++) {
					xercesc_3_0::DOMNode* attr = attributes->item(i);
					mySize += xercesc_3_0::XMLString::stringLen(attr->getNodeName())+xercesc_3_0::XMLString::stringLen(attr->getNodeValue())+4;
				}
			}
			
			xercesc_3_0::DOMNode* child=node->getFirstChild();
			while (child!=NULL) {
				mySize += XyDelta::estimateSubtreeSize(child);
				child=child->getNextSibling();
			}
			break;
		}
		case xercesc_3_0::DOMNode::TEXT_NODE: {
			mySize += xercesc_3_0::XMLString::stringLen(node->getNodeValue());
			break;
		}
		default:
			return 0;
	}
	return mySize;
}




