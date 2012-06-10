#include "xydiff/XyLatinStr.hpp"
#include "DeltaReverse.hpp"
#include "xydiff/DeltaException.hpp"

//#include <../src/dom/NodeImpl.hpp>

#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/dom/DOMAttr.hpp"
#include "xercesc/dom/DOMNode.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/dom/DOMElement.hpp"

#include "xydiff/XID_DOMDocument.hpp"
#include "xydiff/XyDelta_DOMInterface.hpp"

#include "DOMPrint.hpp"
#include "Tools.hpp"
#include <fstream>

XERCES_CPP_NAMESPACE_USE

class DeltaReverseException : public VersionManagerException {
  public:
	  DeltaReverseException(const std::string &status, const std::string &context, const std::string &cause)
	    : VersionManagerException(status, std::string("DeltaReverse::")+context, cause ) {} ;
	  DeltaReverseException(const std::string &context, const std::string &cause)
	    : VersionManagerException(std::string("DeltaReverse::")+context, cause ) {} ;
	} ;

VersionManagerException ReverseUpdateException( const std::string &cause ) {
  return VersionManagerException("Bad data", "DeltaReverse::update", cause);
	}

bool CopyAttr(DOMElement *target, const DOMElement *source, const XMLCh* attrName, bool MustBeFound=true) {
	DOMAttr *attrNode = source->getAttributeNode(attrName);
	if (attrNode==NULL) {
		if (MustBeFound==true) {
			THROW_AWAY(("Mandatory Attribute on Delta Operation Node was not found for CopyAttr()"));
		}
		else return false;
	}
	target->setAttribute(attrName, attrNode->getNodeValue());
	return true;
}

void CopyContent(DOMElement *target, DOMNode *source) {
	if (!source->hasChildNodes()) return;
	DOMNode *content = source->getFirstChild();
	if (content==NULL) return;
	DOMNode *contentNode = target->getOwnerDocument()->importNode(content, true);
	target->appendChild( contentNode );
}

DOMNode* DeltaReverse( DOMNode *deltaElement, DOMDocument *reversedDoc ) {
	
	vddprintf(("    reversing time-version header\n"));

	DOMNode* reversedElement = reversedDoc->importNode( deltaElement, false );

	//DOMString from = deltaElement.getAttributes().getNamedItem("from").getNodeValue() ;
        //DOMString to   = deltaElement.getAttributes().getNamedItem("to"  ).getNodeValue() ;
        const XMLCh* from = deltaElement->getAttributes()->getNamedItem(XMLString::transcode("from"))->getNodeValue() ;
        const XMLCh* to = deltaElement->getAttributes()->getNamedItem(XMLString::transcode("to"))->getNodeValue() ;
        
	reversedElement->getAttributes()->getNamedItem(XMLString::transcode("to"))->setNodeValue( from );
	reversedElement->getAttributes()->getNamedItem(XMLString::transcode("from"))->setNodeValue( to );

	DOMNode* fromXidMap = deltaElement->getAttributes()->getNamedItem(XMLString::transcode("fromXidMap"));
	DOMNode* toXidMap   = deltaElement->getAttributes()->getNamedItem(XMLString::transcode("toXidMap"));
	if (fromXidMap!=NULL) {
          //DOMString from = fromXidMap.getNodeValue();
		reversedElement->getAttributes()->getNamedItem(XMLString::transcode("toXidMap"))->setNodeValue( fromXidMap->getNodeValue() );
	}
	if (toXidMap!=NULL) {
          //DOMString to = toXidMap.getNodeValue();
		reversedElement->getAttributes()->getNamedItem(XMLString::transcode("fromXidMap"))->setNodeValue( toXidMap->getNodeValue() );
	}

	// No chanhges in the delta -> ok
	if (!deltaElement->hasChildNodes()) return( reversedElement );
	
	// Input : read Elementary Operation
	DOMNode* child = deltaElement->getFirstChild() ;
	
	// Output : precedent will be used to write Elementary Operation in reverse order
	DOMNode* precedent = NULL; // =NULL by default

	while (child != NULL) {
		if (child->getNodeType()!=DOMNode::ELEMENT_NODE) THROW_AWAY(("Bad type (%d) for Delta Operation Node", (int)child->getNodeType()));
		DOMElement* operationNode = (DOMElement*) child ;
		XyLatinStr operation(child->getNodeName());
		
		// Reverse DELETE into INSERT
		
		if (strcmp(operation, "d")==0) {
			vddprintf(("    reversing delete into insert\n"));
			DOMElement* iElement = reversedDoc->createElement(XMLString::transcode("i")) ;
			CopyAttr(iElement, operationNode, XMLString::transcode("par"), true);
			CopyAttr(iElement, operationNode, XMLString::transcode("pos"), true);
			CopyAttr(iElement, operationNode, XMLString::transcode("xm"), true);
			CopyAttr(iElement, operationNode, XMLString::transcode("move"), false);
			CopyAttr(iElement, operationNode, XMLString::transcode("update"), false);
			CopyContent(iElement, operationNode);
			reversedElement->insertBefore( iElement, precedent ) ;
			precedent = iElement ;
			}

		// Reverse INSERT into DELETE
		
		else if (strcmp(operation, "i")==0) {
	    		vddprintf(("    reversing insert into delete\n")); 
			DOMElement *iElement = reversedDoc->createElement(XMLString::transcode("d")) ;
			CopyAttr(iElement, operationNode, XMLString::transcode("par"), true);
			CopyAttr(iElement, operationNode, XMLString::transcode("pos"), true);
			CopyAttr(iElement, operationNode, XMLString::transcode("xm"), true);
			CopyAttr(iElement, operationNode, XMLString::transcode("move"), false);
			CopyAttr(iElement, operationNode, XMLString::transcode("update"), false);
			CopyContent(iElement, operationNode);
			reversedElement->insertBefore( iElement, precedent ) ;
			precedent = iElement ;
		  }
	// Attribute Update
		else if (strcmp(operation, "au")==0) {
			vddprintf(("    reversing attribute update\n"));
			
			//DOMString xidElem  = operationNode.getAttributes().getNamedItem("xid").getNodeValue();
			//DOMString attrName = operationNode.getAttributes().getNamedItem("a").getNodeValue();
			//DOMString oldValue = operationNode.getAttributes().getNamedItem("ov").getNodeValue();
			//DOMString newValue = operationNode.getAttributes().getNamedItem("nv").getNodeValue();
			const XMLCh* xidElem = operationNode->getAttributes()->getNamedItem(XMLString::transcode("xid"))->getNodeValue();
                        const XMLCh* attrName = operationNode->getAttributes()->getNamedItem(XMLString::transcode("a"))->getNodeValue();
                        const XMLCh* oldValue = operationNode->getAttributes()->getNamedItem(XMLString::transcode("ov"))->getNodeValue();
                        const XMLCh* newValue = operationNode->getAttributes()->getNamedItem(XMLString::transcode("nv"))->getNodeValue();
                        

			DOMElement* auElement = reversedDoc->createElement(XMLString::transcode("au"));
			auElement->setAttribute(XMLString::transcode("xid"), xidElem);
			auElement->setAttribute(XMLString::transcode("a"),   attrName);
			auElement->setAttribute(XMLString::transcode("nv"),  oldValue);
			auElement->setAttribute(XMLString::transcode("ov"),  newValue);
			
			reversedElement->insertBefore(auElement, precedent);
			precedent = auElement ;
		}
	// Attribute Delete
		else if (strcmp(operation, "ad")==0) {
			vddprintf(("    reversing attribute insert into attribute delete\n"));

			//DOMString xidElem  = operationNode.getAttributes().getNamedItem("xid").getNodeValue();
			//DOMString attrName = operationNode.getAttributes().getNamedItem("a").getNodeValue();
			//DOMString attrVal  = operationNode.getAttributes().getNamedItem("v").getNodeValue();
			const XMLCh* xidElem  = operationNode->getAttributes()->getNamedItem(XMLString::transcode("xid"))->getNodeValue();
			const XMLCh* attrName = operationNode->getAttributes()->getNamedItem(XMLString::transcode("a"))->getNodeValue();
			const XMLCh* attrVal  = operationNode->getAttributes()->getNamedItem(XMLString::transcode("v"))->getNodeValue();
			
			DOMElement* aiElement = reversedDoc->createElement(XMLString::transcode("ai"));
			aiElement->setAttribute(XMLString::transcode("xid"), xidElem);
			aiElement->setAttribute(XMLString::transcode("a"), attrName);
			aiElement->setAttribute(XMLString::transcode("v"), attrVal);
			
			reversedElement->insertBefore(aiElement, precedent);
			precedent = aiElement ;
		}
	// Attribute Insert
		else if (strcmp(operation, "ai")==0) {
			vddprintf(("    reversing attribute delete into attribute insert\n"));

			//DOMString xidElem  = operationNode.getAttributes().getNamedItem("xid").getNodeValue();
			//DOMString attrName = operationNode.getAttributes().getNamedItem("a").getNodeValue();
			//DOMString attrVal  = operationNode.getAttributes().getNamedItem("v").getNodeValue();
                        const XMLCh* xidElem  = operationNode->getAttributes()->getNamedItem(XMLString::transcode("xid"))->getNodeValue();
			const XMLCh* attrName = operationNode->getAttributes()->getNamedItem(XMLString::transcode("a"))->getNodeValue();
			const XMLCh* attrVal  = operationNode->getAttributes()->getNamedItem(XMLString::transcode("v"))->getNodeValue();
			
			DOMElement* adElement = reversedDoc->createElement(XMLString::transcode("ad"));
			adElement->setAttribute(XMLString::transcode("xid"), xidElem);
			adElement->setAttribute(XMLString::transcode("a"), attrName);
			adElement->setAttribute(XMLString::transcode("v"), attrVal);
			
			reversedElement->insertBefore(adElement, precedent);
			precedent = adElement ;
		}
		
    // Reverse UPDATE to UPDATE
		
		else if (strcmp(operation, "u")==0) {
			vddprintf(("    reversing update\n"));
		  
			DOMNode* oldValue = child->getFirstChild() ;
			if ( oldValue==NULL ) throw ReverseUpdateException("<update> has no child!");
			
			DOMNode* newValue = oldValue->getNextSibling() ;
			if ( newValue==NULL ) throw ReverseUpdateException("<update> has only one child!");
			
			//DOMString xid= child.getAttributes().getNamedItem("xid").getNodeValue() ;
                        const XMLCh* xid= child->getAttributes()->getNamedItem(XMLString::transcode("xid"))->getNodeValue() ;
			
			DOMElement *uElement = reversedDoc->createElement(XMLString::transcode("u")) ;
			uElement->setAttribute(XMLString::transcode("xid"), xid);
			
			DOMElement* uOld = reversedDoc->createElement(XMLString::transcode("ov"));
			DOMNode* uOldText = reversedDoc->importNode( newValue->getFirstChild(), true );
			uOld->appendChild( uOldText );
			
			DOMElement* uNew = reversedDoc->createElement(XMLString::transcode("nv"));
			DOMNode* uNewText = reversedDoc->importNode( oldValue->getFirstChild(), true );
			uNew->appendChild( uNewText );
			
			uElement->appendChild( uOld ) ;
			uElement->appendChild( uNew ) ;
			
		  reversedElement->insertBefore( uElement, precedent ) ;
			precedent = uElement ;
			}
		else if (strcmp(operation, "renameRoot")==0) {
			vddprintf(("    reversing renameRoot operation\n"));

			const XMLCh* rFrom = operationNode->getAttributes()->getNamedItem(XMLString::transcode("from"))->getNodeValue();
			const XMLCh* rTo   = operationNode->getAttributes()->getNamedItem(XMLString::transcode("to"))->getNodeValue();
			
			DOMElement *rrElement = reversedDoc->createElement(XMLString::transcode("renameRoot"));
			rrElement->setAttribute(XMLString::transcode("from"), rTo);
			rrElement->setAttribute(XMLString::transcode("to"), rFrom);
			
			reversedElement->insertBefore(rrElement, precedent);
			precedent = rrElement ;			
		}
		
		else {
			throw DeltaReverseException("Invalid Data", "main", "Unknown operation <" + std::string(operation) + ">")  ;
		}
				
		child = child->getNextSibling();
		}
	
	return ( reversedElement ) ;
	}

