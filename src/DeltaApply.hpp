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
		void ApplyDeltaElement(xercesc_3_0::DOMNode* incDeltaElement);
	
		XID_DOMDocument* getResultDocument (void) ;

		static std::string getSourceURI(XID_DOMDocument *IncDeltaDoc);
		static std::string getDestinationURI(XID_DOMDocument *IncDeltaDoc);

	private:
		XID_DOMDocument* xiddoc ;
		XID_DOMDocument* deltaDoc ;

		static xercesc_3_0::DOMNode* getDeltaElement(XID_DOMDocument *IncDeltaDoc);
		xercesc_3_0::DOMNode* deltaElement ;
		
		XID_DOMDocument* moveDocument ;
		
		void ApplyOperation(xercesc_3_0::DOMNode *operationNode);

		void Subtree_MoveFrom( XID_t myXID );
		void Subtree_Delete( const char *xidmapStr );
		void Subtree_MoveTo( XID_t myXID, XID_t parentXID, int position );
		void Subtree_Insert( xercesc_3_0::DOMNode *insertSubtreeRoot, XID_t parentXID, int position, const char *xidmapStr );
		void TextNode_Update( XID_t nodeXID, const XMLCh* newValue );
		void Attribute_Insert( XID_t nodeXID, const XMLCh* attr, const XMLCh* value ) ;
		void Attribute_Update( XID_t nodeXID, const XMLCh* attr, const XMLCh* value ) ;
		void Attribute_Delete( XID_t nodeXID, const XMLCh* attr ) ;

	} ;

#endif
