#ifndef XID_MAP_HXX__
#define XID_MAP_HXX__

#include <map>
#include "infra/general/hash_map.hpp"
#include <vector>
#include <string>

//#include "xercesc/dom/deprecated/DOM_Node.hpp"
#include "xercesc/dom/DOMNode.hpp"

#define XID_INVALID (-1)
typedef long int XID_t ;

class XidMap_Parser {
	public:
		XidMap_Parser( const char *str );
		XID_t getNextXID(void) ; // in Postfix order
		XID_t getRootXID(void); // =the last one
		XID_t getFirstAvailableXID(XID_t actualValue=XID_INVALID) ;
		bool  isListEmpty(void) ;
	private:
		std::vector<XID_t> xidList ;
		XID_t firstAvailableXID ;
		unsigned int postfix ;
	} ;


class XID_map {

  public:

		/* Create empty XID map */

		XID_map(void);

		/* Create XID map that maps the XID-String to the given subtree */

		XID_map(const char *str , xercesc_3_0::DOMNode *IncDocRoot);
		void SetRootElement( xercesc_3_0::DOMNode *IncDocRoot );

		/* Destructor */

		~XID_map();

		/* Once an XID-map is created, use pointers only through reference counter functions
		 */
		
		static XID_map* addReference(XID_map* target);
		static void     removeReference(XID_map* & inoutXidmapPtr);
		
		/* Give XID-String corresponding to the whole subtree starting from DocRoot
		 * Note that next available XID information is always included if it has been defined (e.g. not INVALID )
		 */
		
		std::string String(void);
		
		/* Give XID-String corresponding to the subtree starting at node
		   option: add next available XID information at the end of the String 
		 */
		
		std::string String(xercesc_3_0::DOMNode *node, bool printFirstAvailXID=false) ;

		/* Functions that manage XID-2-Node mappings
		 */
		
		void     registerNode( xercesc_3_0::DOMNode *node, XID_t xid );
		void     removeNode( xercesc_3_0::DOMNode *node ) ;
		
		xercesc_3_0::DOMNode* getNodeWithXID(const XID_t xid);
		bool     findNodeWithXID(const XID_t xid);
		XID_t    getXIDbyNode(xercesc_3_0::DOMNode *node);
		
		XID_t    allocateNewXID(void);
		XID_t    getFirstAvailableXID(void);
		void     initFirstAvailableXID(XID_t xid);
		
		/* Functions that manage XID-2-Node mappings at the subtree level
		 */
		
		void mapSubtree   ( const char *str, xercesc_3_0::DOMNode *node ) ;
		void removeSubtree( xercesc_3_0::DOMNode *node ) ;

		/* PRIVATE */
		
		std::string StringFromList( std::vector<XID_t> xidList, bool writeFirstAvail = false ) ;

		static XID_t getXidFromAttribute(xercesc_3_0::DOMNode* elem, const XMLCh *attrName, bool parenthesis=false);

	private:

		void _internal_TraceTree( std::vector<XID_t> &xidList, xercesc_3_0::DOMNode *node ) ;
		static std::string xidString(XID_t x) ;

		void registerSubtree( XidMap_Parser &parse, xercesc_3_0::DOMNode *node ) ;
		void _internal_registerSubtree( XidMap_Parser &parse, xercesc_3_0::DOMNode *node );
		
		int                    referenceCount ;
		std::map<XID_t, xercesc_3_0::DOMNode*> *nodeByXid ;
		std::map<xercesc_3_0::DOMNode*, XID_t> *xidByNode ;
		XID_t                  firstAvailableXID ;

		xercesc_3_0::DOMNode*               docRoot ;
		
		// No default copy or equal operator
		XID_map( const XID_map &other );
		XID_map & operator = (const XID_map &other);
	} ;


#endif
