#include "XyDiff/include/XyLatinStr.hpp"
#include "XyDiff/DeltaReverse.hpp"
#include "XyDiff/DeltaException.hpp"

//#include <../src/dom/NodeImpl.hpp>

#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/dom/DOMAttr.hpp"
#include "xercesc/dom/DOMNode.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/dom/DOMElement.hpp"

#include "XyDiff/include/XID_DOMDocument.hpp"
#include "XyDiff/include/XyDelta_DOMInterface.hpp"

#include "XyDiff/DOMPrint.hpp"
#include "XyDiff/Tools.hpp"
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

bool CopyAttr(xercesc_2_2::DOMElement *target, const xercesc_2_2::DOMElement *source, const XMLCh* attrName, bool MustBeFound=true) {
	xercesc_2_2::DOMAttr *attrNode = source->getAttributeNode(attrName);
	if (attrNode==NULL) {
		if (MustBeFound==true) {
			THROW_AWAY(("Mandatory Attribute on Delta Operation Node was not found for CopyAttr()"));
		}
		else return false;
	}
	target->setAttribute(attrName, attrNode->getNodeValue());
	return true;
}

void CopyContent(xercesc_2_2::DOMElement *target, xercesc_2_2::DOMNode *source) {
	if (!source->hasChildNodes()) return;
	xercesc_2_2::DOMNode *content = source->getFirstChild();
	if (content==NULL) return;
	xercesc_2_2::DOMNode *contentNode = target->getOwnerDocument()->importNode(content, true);
	target->appendChild( contentNode );
}

xercesc_2_2::DOMNode* DeltaReverse( xercesc_2_2::DOMNode *deltaElement, xercesc_2_2::DOMDocument *reversedDoc ) {
	
	vddprintf(("    reversing time-version header\n"));

	xercesc_2_2::DOMNode* reversedElement = reversedDoc->importNode( deltaElement, false );

	//xercesc_2_2::DOMString from = deltaElement.getAttributes().getNamedItem("from").getNodeValue() ;
        //xercesc_2_2::DOMString to   = deltaElement.getAttributes().getNamedItem("to"  ).getNodeValue() ;
        const XMLCh* from = deltaElement->getAttributes()->getNamedItem(L"from")->getNodeValue() ;
        const XMLCh* to = deltaElement->getAttributes()->getNamedItem(L"to")->getNodeValue() ;
        
	reversedElement->getAttributes()->getNamedItem(L"to")->setNodeValue( from );
	reversedElement->getAttributes()->getNamedItem(L"from")->setNodeValue( to );

	xercesc_2_2::DOMNode* fromXidMap = deltaElement->getAttributes()->getNamedItem(L"fromXidMap");
	xercesc_2_2::DOMNode* toXidMap   = deltaElement->getAttributes()->getNamedItem(L"toXidMap");
	if (fromXidMap!=NULL) {
          //xercesc_2_2::DOMString from = fromXidMap.getNodeValue();
		reversedElement->getAttributes()->getNamedItem(L"toXidMap")->setNodeValue( fromXidMap->getNodeValue() );
	}
	if (toXidMap!=NULL) {
          //xercesc_2_2::DOMString to = toXidMap.getNodeValue();
		reversedElement->getAttributes()->getNamedItem(L"fromXidMap")->setNodeValue( toXidMap->getNodeValue() );
	}

	// No chanhges in the delta -> ok
	if (!deltaElement->hasChildNodes()) return( reversedElement );
	
	// Input : read Elementary Operation
	xercesc_2_2::DOMNode* child = deltaElement->getFirstChild() ;
	
	// Output : precedent will be used to write Elementary Operation in reverse order
	xercesc_2_2::DOMNode* precedent = NULL; // =NULL by default

	while (child != NULL) {
		if (child->getNodeType()!=xercesc_2_2::DOMNode::ELEMENT_NODE) THROW_AWAY(("Bad type (%d) for Delta Operation Node", (int)child->getNodeType()));
		xercesc_2_2::DOMElement* operationNode = (xercesc_2_2::DOMElement*) child ;
		XyLatinStr operation(child->getNodeName());
		
		// Reverse DELETE into INSERT
		
		if (strcmp(operation, "d")==0) {
			vddprintf(("    reversing delete into insert\n"));
			xercesc_2_2::DOMElement* iElement = reversedDoc->createElement(L"i") ;
			CopyAttr(iElement, operationNode, L"par", true);
			CopyAttr(iElement, operationNode, L"pos", true);
			CopyAttr(iElement, operationNode, L"xm", true);
			CopyAttr(iElement, operationNode, L"move", false);
			CopyAttr(iElement, operationNode, L"update", false);
			CopyContent(iElement, operationNode);
			reversedElement->insertBefore( iElement, precedent ) ;
			precedent = iElement ;
			}

		// Reverse INSERT into DELETE
		
		else if (strcmp(operation, "i")==0) {
	    		vddprintf(("    reversing insert into delete\n")); 
			xercesc_2_2::DOMElement *iElement = reversedDoc->createElement(L"d") ;
			CopyAttr(iElement, operationNode, L"par", true);
			CopyAttr(iElement, operationNode, L"pos", true);
			CopyAttr(iElement, operationNode, L"xm", true);
			CopyAttr(iElement, operationNode, L"move", false);
			CopyAttr(iElement, operationNode, L"update", false);
			CopyContent(iElement, operationNode);
			reversedElement->insertBefore( iElement, precedent ) ;
			precedent = iElement ;
		  }
	// Attribute Update
		else if (strcmp(operation, "au")==0) {
			vddprintf(("    reversing attribute update\n"));
			
			//xercesc_2_2::DOMString xidElem  = operationNode.getAttributes().getNamedItem("xid").getNodeValue();
			//xercesc_2_2::DOMString attrName = operationNode.getAttributes().getNamedItem("a").getNodeValue();
			//xercesc_2_2::DOMString oldValue = operationNode.getAttributes().getNamedItem("ov").getNodeValue();
			//xercesc_2_2::DOMString newValue = operationNode.getAttributes().getNamedItem("nv").getNodeValue();
			const XMLCh* xidElem = operationNode->getAttributes()->getNamedItem(L"xid")->getNodeValue();
                        const XMLCh* attrName = operationNode->getAttributes()->getNamedItem(L"a")->getNodeValue();
                        const XMLCh* oldValue = operationNode->getAttributes()->getNamedItem(L"ov")->getNodeValue();
                        const XMLCh* newValue = operationNode->getAttributes()->getNamedItem(L"nv")->getNodeValue();
                        

			xercesc_2_2::DOMElement* auElement = reversedDoc->createElement(L"au");
			auElement->setAttribute(L"xid", xidElem);
			auElement->setAttribute(L"a",   attrName);
			auElement->setAttribute(L"nv",  oldValue);
			auElement->setAttribute(L"ov",  newValue);
			
			reversedElement->insertBefore(auElement, precedent);
			precedent = auElement ;
		}
	// Attribute Delete
		else if (strcmp(operation, "ad")==0) {
			vddprintf(("    reversing attribute insert into attribute delete\n"));

			//xercesc_2_2::DOMString xidElem  = operationNode.getAttributes().getNamedItem("xid").getNodeValue();
			//xercesc_2_2::DOMString attrName = operationNode.getAttributes().getNamedItem("a").getNodeValue();
			//xercesc_2_2::DOMString attrVal  = operationNode.getAttributes().getNamedItem("v").getNodeValue();
			const XMLCh* xidElem  = operationNode->getAttributes()->getNamedItem(L"xid")->getNodeValue();
			const XMLCh* attrName = operationNode->getAttributes()->getNamedItem(L"a")->getNodeValue();
			const XMLCh* attrVal  = operationNode->getAttributes()->getNamedItem(L"v")->getNodeValue();
			
			xercesc_2_2::DOMElement* aiElement = reversedDoc->createElement(L"ai");
			aiElement->setAttribute(L"xid", xidElem);
			aiElement->setAttribute(L"a", attrName);
			aiElement->setAttribute(L"v", attrVal);
			
			reversedElement->insertBefore(aiElement, precedent);
			precedent = aiElement ;
		}
	// Attribute Insert
		else if (strcmp(operation, "ai")==0) {
			vddprintf(("    reversing attribute delete into attribute insert\n"));

			//xercesc_2_2::DOMString xidElem  = operationNode.getAttributes().getNamedItem("xid").getNodeValue();
			//xercesc_2_2::DOMString attrName = operationNode.getAttributes().getNamedItem("a").getNodeValue();
			//xercesc_2_2::DOMString attrVal  = operationNode.getAttributes().getNamedItem("v").getNodeValue();
                        const XMLCh* xidElem  = operationNode->getAttributes()->getNamedItem(L"xid")->getNodeValue();
			const XMLCh* attrName = operationNode->getAttributes()->getNamedItem(L"a")->getNodeValue();
			const XMLCh* attrVal  = operationNode->getAttributes()->getNamedItem(L"v")->getNodeValue();
			
			xercesc_2_2::DOMElement* adElement = reversedDoc->createElement(L"ad");
			adElement->setAttribute(L"xid", xidElem);
			adElement->setAttribute(L"a", attrName);
			adElement->setAttribute(L"v", attrVal);
			
			reversedElement->insertBefore(adElement, precedent);
			precedent = adElement ;
		}
		
    // Reverse UPDATE to UPDATE
		
		else if (strcmp(operation, "u")==0) {
			vddprintf(("    reversing update\n"));
		  
			xercesc_2_2::DOMNode* oldValue = child->getFirstChild() ;
			if ( oldValue==NULL ) throw ReverseUpdateException("<update> has no child!");
			
			xercesc_2_2::DOMNode* newValue = oldValue->getNextSibling() ;
			if ( newValue==NULL ) throw ReverseUpdateException("<update> has only one child!");
			
			//xercesc_2_2::DOMString xid= child.getAttributes().getNamedItem("xid").getNodeValue() ;
                        const XMLCh* xid= child->getAttributes()->getNamedItem(L"xid")->getNodeValue() ;
			
			xercesc_2_2::DOMElement *uElement = reversedDoc->createElement(L"u") ;
			uElement->setAttribute(L"xid", xid);
			
			xercesc_2_2::DOMElement* uOld = reversedDoc->createElement(L"ov");
			xercesc_2_2::DOMNode* uOldText = reversedDoc->importNode( newValue->getFirstChild(), true );
			uOld->appendChild( uOldText );
			
			xercesc_2_2::DOMElement* uNew = reversedDoc->createElement(L"nv");
			xercesc_2_2::DOMNode* uNewText = reversedDoc->importNode( oldValue->getFirstChild(), true );
			uNew->appendChild( uNewText );
			
			uElement->appendChild( uOld ) ;
			uElement->appendChild( uNew ) ;
			
		  reversedElement->insertBefore( uElement, precedent ) ;
			precedent = uElement ;
			}
		else if (strcmp(operation, "renameRoot")==0) {
			vddprintf(("    reversing renameRoot operation\n"));

			const XMLCh* rFrom = operationNode->getAttributes()->getNamedItem(L"from")->getNodeValue();
			const XMLCh* rTo   = operationNode->getAttributes()->getNamedItem(L"to")->getNodeValue();
			
			xercesc_2_2::DOMElement *rrElement = reversedDoc->createElement(L"renameRoot");
			rrElement->setAttribute(L"from", rTo);
			rrElement->setAttribute(L"to", rFrom);
			
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

