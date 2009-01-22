#include "include/XyLatinStr.hpp"
#include "DeltaReverse.hpp"
#include "DeltaException.hpp"

//#include <../src/dom/NodeImpl.hpp>

#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/dom/DOMAttr.hpp"
#include "xercesc/dom/DOMNode.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/dom/DOMElement.hpp"

#include "include/XID_DOMDocument.hpp"
#include "include/XyDelta_DOMInterface.hpp"

#include "DOMPrint.hpp"
#include "Tools.hpp"
#include <fstream>


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

bool CopyAttr(xercesc_3_0::DOMElement *target, const xercesc_3_0::DOMElement *source, const XMLCh* attrName, bool MustBeFound=true) {
	xercesc_3_0::DOMAttr *attrNode = source->getAttributeNode(attrName);
	if (attrNode==NULL) {
		if (MustBeFound==true) {
			THROW_AWAY(("Mandatory Attribute on Delta Operation Node was not found for CopyAttr()"));
		}
		else return false;
	}
	target->setAttribute(attrName, attrNode->getNodeValue());
	return true;
}

void CopyContent(xercesc_3_0::DOMElement *target, xercesc_3_0::DOMNode *source) {
	if (!source->hasChildNodes()) return;
	xercesc_3_0::DOMNode *content = source->getFirstChild();
	if (content==NULL) return;
	xercesc_3_0::DOMNode *contentNode = target->getOwnerDocument()->importNode(content, true);
	target->appendChild( contentNode );
}

xercesc_3_0::DOMNode* DeltaReverse( xercesc_3_0::DOMNode *deltaElement, xercesc_3_0::DOMDocument *reversedDoc ) {
	
	vddprintf(("    reversing time-version header\n"));

	xercesc_3_0::DOMNode* reversedElement = reversedDoc->importNode( deltaElement, false );

	//xercesc_3_0::DOMString from = deltaElement.getAttributes().getNamedItem("from").getNodeValue() ;
        //xercesc_3_0::DOMString to   = deltaElement.getAttributes().getNamedItem("to"  ).getNodeValue() ;
        const XMLCh* from = deltaElement->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("from"))->getNodeValue() ;
        const XMLCh* to = deltaElement->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("to"))->getNodeValue() ;
        
	reversedElement->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("to"))->setNodeValue( from );
	reversedElement->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("from"))->setNodeValue( to );

	xercesc_3_0::DOMNode* fromXidMap = deltaElement->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("fromXidMap"));
	xercesc_3_0::DOMNode* toXidMap   = deltaElement->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("toXidMap"));
	if (fromXidMap!=NULL) {
          //xercesc_3_0::DOMString from = fromXidMap.getNodeValue();
		reversedElement->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("toXidMap"))->setNodeValue( fromXidMap->getNodeValue() );
	}
	if (toXidMap!=NULL) {
          //xercesc_3_0::DOMString to = toXidMap.getNodeValue();
		reversedElement->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("fromXidMap"))->setNodeValue( toXidMap->getNodeValue() );
	}

	// No chanhges in the delta -> ok
	if (!deltaElement->hasChildNodes()) return( reversedElement );
	
	// Input : read Elementary Operation
	xercesc_3_0::DOMNode* child = deltaElement->getFirstChild() ;
	
	// Output : precedent will be used to write Elementary Operation in reverse order
	xercesc_3_0::DOMNode* precedent = NULL; // =NULL by default

	while (child != NULL) {
		if (child->getNodeType()!=xercesc_3_0::DOMNode::ELEMENT_NODE) THROW_AWAY(("Bad type (%d) for Delta Operation Node", (int)child->getNodeType()));
		xercesc_3_0::DOMElement* operationNode = (xercesc_3_0::DOMElement*) child ;
		XyLatinStr operation(child->getNodeName());
		
		// Reverse DELETE into INSERT
		
		if (strcmp(operation, "d")==0) {
			vddprintf(("    reversing delete into insert\n"));
			xercesc_3_0::DOMElement* iElement = reversedDoc->createElement(xercesc_3_0::XMLString::transcode("i")) ;
			CopyAttr(iElement, operationNode, xercesc_3_0::XMLString::transcode("par"), true);
			CopyAttr(iElement, operationNode, xercesc_3_0::XMLString::transcode("pos"), true);
			CopyAttr(iElement, operationNode, xercesc_3_0::XMLString::transcode("xm"), true);
			CopyAttr(iElement, operationNode, xercesc_3_0::XMLString::transcode("move"), false);
			CopyAttr(iElement, operationNode, xercesc_3_0::XMLString::transcode("update"), false);
			CopyContent(iElement, operationNode);
			reversedElement->insertBefore( iElement, precedent ) ;
			precedent = iElement ;
			}

		// Reverse INSERT into DELETE
		
		else if (strcmp(operation, "i")==0) {
	    		vddprintf(("    reversing insert into delete\n")); 
			xercesc_3_0::DOMElement *iElement = reversedDoc->createElement(xercesc_3_0::XMLString::transcode("d")) ;
			CopyAttr(iElement, operationNode, xercesc_3_0::XMLString::transcode("par"), true);
			CopyAttr(iElement, operationNode, xercesc_3_0::XMLString::transcode("pos"), true);
			CopyAttr(iElement, operationNode, xercesc_3_0::XMLString::transcode("xm"), true);
			CopyAttr(iElement, operationNode, xercesc_3_0::XMLString::transcode("move"), false);
			CopyAttr(iElement, operationNode, xercesc_3_0::XMLString::transcode("update"), false);
			CopyContent(iElement, operationNode);
			reversedElement->insertBefore( iElement, precedent ) ;
			precedent = iElement ;
		  }
	// Attribute Update
		else if (strcmp(operation, "au")==0) {
			vddprintf(("    reversing attribute update\n"));
			
			//xercesc_3_0::DOMString xidElem  = operationNode.getAttributes().getNamedItem("xid").getNodeValue();
			//xercesc_3_0::DOMString attrName = operationNode.getAttributes().getNamedItem("a").getNodeValue();
			//xercesc_3_0::DOMString oldValue = operationNode.getAttributes().getNamedItem("ov").getNodeValue();
			//xercesc_3_0::DOMString newValue = operationNode.getAttributes().getNamedItem("nv").getNodeValue();
			const XMLCh* xidElem = operationNode->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("xid"))->getNodeValue();
                        const XMLCh* attrName = operationNode->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("a"))->getNodeValue();
                        const XMLCh* oldValue = operationNode->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("ov"))->getNodeValue();
                        const XMLCh* newValue = operationNode->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("nv"))->getNodeValue();
                        

			xercesc_3_0::DOMElement* auElement = reversedDoc->createElement(xercesc_3_0::XMLString::transcode("au"));
			auElement->setAttribute(xercesc_3_0::XMLString::transcode("xid"), xidElem);
			auElement->setAttribute(xercesc_3_0::XMLString::transcode("a"),   attrName);
			auElement->setAttribute(xercesc_3_0::XMLString::transcode("nv"),  oldValue);
			auElement->setAttribute(xercesc_3_0::XMLString::transcode("ov"),  newValue);
			
			reversedElement->insertBefore(auElement, precedent);
			precedent = auElement ;
		}
	// Attribute Delete
		else if (strcmp(operation, "ad")==0) {
			vddprintf(("    reversing attribute insert into attribute delete\n"));

			//xercesc_3_0::DOMString xidElem  = operationNode.getAttributes().getNamedItem("xid").getNodeValue();
			//xercesc_3_0::DOMString attrName = operationNode.getAttributes().getNamedItem("a").getNodeValue();
			//xercesc_3_0::DOMString attrVal  = operationNode.getAttributes().getNamedItem("v").getNodeValue();
			const XMLCh* xidElem  = operationNode->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("xid"))->getNodeValue();
			const XMLCh* attrName = operationNode->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("a"))->getNodeValue();
			const XMLCh* attrVal  = operationNode->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("v"))->getNodeValue();
			
			xercesc_3_0::DOMElement* aiElement = reversedDoc->createElement(xercesc_3_0::XMLString::transcode("ai"));
			aiElement->setAttribute(xercesc_3_0::XMLString::transcode("xid"), xidElem);
			aiElement->setAttribute(xercesc_3_0::XMLString::transcode("a"), attrName);
			aiElement->setAttribute(xercesc_3_0::XMLString::transcode("v"), attrVal);
			
			reversedElement->insertBefore(aiElement, precedent);
			precedent = aiElement ;
		}
	// Attribute Insert
		else if (strcmp(operation, "ai")==0) {
			vddprintf(("    reversing attribute delete into attribute insert\n"));

			//xercesc_3_0::DOMString xidElem  = operationNode.getAttributes().getNamedItem("xid").getNodeValue();
			//xercesc_3_0::DOMString attrName = operationNode.getAttributes().getNamedItem("a").getNodeValue();
			//xercesc_3_0::DOMString attrVal  = operationNode.getAttributes().getNamedItem("v").getNodeValue();
                        const XMLCh* xidElem  = operationNode->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("xid"))->getNodeValue();
			const XMLCh* attrName = operationNode->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("a"))->getNodeValue();
			const XMLCh* attrVal  = operationNode->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("v"))->getNodeValue();
			
			xercesc_3_0::DOMElement* adElement = reversedDoc->createElement(xercesc_3_0::XMLString::transcode("ad"));
			adElement->setAttribute(xercesc_3_0::XMLString::transcode("xid"), xidElem);
			adElement->setAttribute(xercesc_3_0::XMLString::transcode("a"), attrName);
			adElement->setAttribute(xercesc_3_0::XMLString::transcode("v"), attrVal);
			
			reversedElement->insertBefore(adElement, precedent);
			precedent = adElement ;
		}
		
    // Reverse UPDATE to UPDATE
		
		else if (strcmp(operation, "u")==0) {
			vddprintf(("    reversing update\n"));
		  
			xercesc_3_0::DOMNode* oldValue = child->getFirstChild() ;
			if ( oldValue==NULL ) throw ReverseUpdateException("<update> has no child!");
			
			xercesc_3_0::DOMNode* newValue = oldValue->getNextSibling() ;
			if ( newValue==NULL ) throw ReverseUpdateException("<update> has only one child!");
			
			//xercesc_3_0::DOMString xid= child.getAttributes().getNamedItem("xid").getNodeValue() ;
                        const XMLCh* xid= child->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("xid"))->getNodeValue() ;
			
			xercesc_3_0::DOMElement *uElement = reversedDoc->createElement(xercesc_3_0::XMLString::transcode("u")) ;
			uElement->setAttribute(xercesc_3_0::XMLString::transcode("xid"), xid);
			
			xercesc_3_0::DOMElement* uOld = reversedDoc->createElement(xercesc_3_0::XMLString::transcode("ov"));
			xercesc_3_0::DOMNode* uOldText = reversedDoc->importNode( newValue->getFirstChild(), true );
			uOld->appendChild( uOldText );
			
			xercesc_3_0::DOMElement* uNew = reversedDoc->createElement(xercesc_3_0::XMLString::transcode("nv"));
			xercesc_3_0::DOMNode* uNewText = reversedDoc->importNode( oldValue->getFirstChild(), true );
			uNew->appendChild( uNewText );
			
			uElement->appendChild( uOld ) ;
			uElement->appendChild( uNew ) ;
			
		  reversedElement->insertBefore( uElement, precedent ) ;
			precedent = uElement ;
			}
		else if (strcmp(operation, "renameRoot")==0) {
			vddprintf(("    reversing renameRoot operation\n"));

			const XMLCh* rFrom = operationNode->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("from"))->getNodeValue();
			const XMLCh* rTo   = operationNode->getAttributes()->getNamedItem(xercesc_3_0::XMLString::transcode("to"))->getNodeValue();
			
			xercesc_3_0::DOMElement *rrElement = reversedDoc->createElement(xercesc_3_0::XMLString::transcode("renameRoot"));
			rrElement->setAttribute(xercesc_3_0::XMLString::transcode("from"), rTo);
			rrElement->setAttribute(xercesc_3_0::XMLString::transcode("to"), rFrom);
			
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

