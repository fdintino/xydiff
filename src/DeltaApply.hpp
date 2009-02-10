#ifndef DELTAAPPLY_HXX_
#define DELTAAPPLY_HXX_

#include "xercesc/dom/DOMNode.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "include/XID_DOMDocument.hpp"
#include "include/XID_map.hpp"
#include <string>

class DeltaApplyEngine {
	public:
		DeltaApplyEngine(XID_DOMDocument *sourceDoc) ;
		void ApplyDelta(XID_DOMDocument *IncDeltaDoc);
		void ApplyDelta(XID_DOMDocument *IncDeltaDoc, int backwardNumber);
		void ApplyDeltaElement(xercesc::DOMNode* incDeltaElement);
	
		XID_DOMDocument* getResultDocument (void) ;

		static std::string getSourceURI(XID_DOMDocument *IncDeltaDoc);
		static std::string getDestinationURI(XID_DOMDocument *IncDeltaDoc);
		bool getApplyAnnotations();
		void setApplyAnnotations(bool paramApplyAnnotations);
	private:
		XID_DOMDocument* xiddoc ;
		XID_DOMDocument* deltaDoc ;

		static xercesc::DOMNode* getDeltaElement(XID_DOMDocument *IncDeltaDoc);
		xercesc::DOMNode* deltaElement ;
		
		XID_DOMDocument* moveDocument ;
		
		void ApplyOperation(xercesc::DOMNode *operationNode);

		void Subtree_MoveFrom( XID_t myXID );
		void Subtree_Delete( const char *xidmapStr );
		void Subtree_MoveTo( XID_t myXID, XID_t parentXID, int position );
		void Subtree_Insert( xercesc::DOMNode *insertSubtreeRoot, XID_t parentXID, int position, const char *xidmapStr );
		void TextNode_Update( XID_t nodeXID, xercesc::DOMNode *operationNode );
		void Attribute_Insert( XID_t nodeXID, const XMLCh* attr, const XMLCh* value ) ;
		void Attribute_Update( XID_t nodeXID, const XMLCh* attr, const XMLCh* value ) ;
		void Attribute_Delete( XID_t nodeXID, const XMLCh* attr ) ;
		bool applyAnnotations;
	} ;

#endif
