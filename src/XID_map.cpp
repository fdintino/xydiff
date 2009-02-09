#include "infra/general/Log.hpp"

#include "include/XID_map.hpp"

#include "include/XyLatinStr.hpp"
#include "DeltaException.hpp"
//#include "DOM_Node_PassThrough.hh"
#include "Tools.hpp"

//#include "xercesc/dom/DOMString.hpp"
#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/util/XMLString.hpp"

#include <stdio.h>

XERCES_CPP_NAMESPACE_USE

/*************************************************************************************
 *                                                                                   *
 * ---- ---- Xid Map Parser : Parses XID string and manage its information ---- ---- *
 *                                                                                   *
 *************************************************************************************/

class ParseError : public VersionManagerException {
  public:
	ParseError(const std::string &cause)
	  : VersionManagerException("XID-Map constructor / Parse XID-String Error", cause)
		{} ;
	} ;

XID_t XidMap_Parser::getNextXID(void) {
	if (postfix<xidList.size()) {
		return xidList[postfix++] ;
	}
	else THROW_AWAY(("next postfix (%d) is out of bound",(int)postfix));
}

XID_t XidMap_Parser::getFirstAvailableXID(XID_t actualValue) {
	if (firstAvailableXID>actualValue) return firstAvailableXID ;
	else return actualValue ;
}
	
bool XidMap_Parser::isListEmpty(void) {
	return (postfix==xidList.size()) ;
}

XID_t XidMap_Parser::getRootXID(void) {
	if (xidList.size()==0) THROW_AWAY(("list is empty"));
	return (xidList[xidList.size()-1]);
}
	
XidMap_Parser::XidMap_Parser( const char *str ) {

	postfix = 0 ;

	if (!str)                   THROW_AWAY(("char* is a null pointer")) ;
	
	vddprintf(("xidmap=%s", str));
	
	if (str[0]!='(')            THROW_AWAY(("string %s does not start with a left parenthesis", str)) ;
	char *s = (char*)str ;
	
	while((s[0]=='(')||(s[0]==';')) {
		if (s[0]=='-')            THROW_AWAY(("unexpected character %d : >>> %c <<<",(int)s[0],s[0]));
		XID_t xid = (XID_t) strtol(++s, &s, 10);
		if (xid==0)               THROW_AWAY(("invalid or null xid encountered is %s", str));
		if (xid<0)                THROW_AWAY(("negative xid %d encountered in %s", xid, str));
		if (s[0]=='-') {
			XID_t xidEnd = (XID_t) strtol(++s, &s, 10);
			if (xidEnd<=xid)        THROW_AWAY(("range end xid %d lower or equal to left xid %d in %s", xidEnd, xid, str));
			for(XID_t i=xid; i <= xidEnd; i++) xidList.push_back(i);
		} else {
			xidList.push_back(xid);
		}
	}
		
	if (s[0]=='|') {
		firstAvailableXID = (XID_t) strtol(++s, &s, 10);
		if (firstAvailableXID==0) THROW_AWAY(("first available xid is invalid or null in %s", str));
		if (firstAvailableXID<0)  THROW_AWAY(("first available xid is negative in %s", str));
	} else {
		firstAvailableXID=XID_INVALID ;	
	}
	
	if (s[0]!=')')              THROW_AWAY(("string %s does not end with right parenthesis", str));
	
}

/*************************************************************************
 *                                                                       *
 * ---- ---- CLASS Xid_Map : Management of XID-2-Node mappings ---- ---- *
 *                                                                       *
 *************************************************************************/

/*
 * Construct Default (empty) XID-map
 */

XID_map::XID_map(void)
	: referenceCount(0), nodeByXid(NULL), xidByNode(NULL), firstAvailableXID(XID_INVALID), docRoot(NULL)
{
	
	nodeByXid         = new std::map<XID_t, DOMNode*> ;
	xidByNode         = new std::map<DOMNode*, XID_t> ;
}

/*
 * Construct Default XID-map mapping the XID-String to the given Subtree
 */

XID_map::XID_map( const char *str, DOMNode *IncDocRoot)
	: referenceCount(0), nodeByXid(NULL), xidByNode(NULL), firstAvailableXID(XID_INVALID), docRoot(NULL)
{
	TRACE("init");
	docRoot = IncDocRoot ;
	TRACE("parse");
	XidMap_Parser parse( str );
	TRACE("register subtree");
	registerSubtree( parse, IncDocRoot );
	TRACE("ok");
}

void XID_map::SetRootElement( DOMNode *IncDocRoot ) {
	if (docRoot==NULL) docRoot = IncDocRoot ;
	else THROW_AWAY(("There was already a root element"));
}

/*
 * Destructor
 */

XID_map::~XID_map(void) {
	if (referenceCount > 0) THROW_AWAY(("reference count=%d is not NULL", referenceCount));
	if (nodeByXid) delete nodeByXid ;
	nodeByXid = NULL;
	if (xidByNode) delete xidByNode ;
	xidByNode = NULL ;
	referenceCount = -1 ;
	firstAvailableXID = XID_INVALID ;
}

/* ---- Output to STRING functions ----
 *
 * Example : for sequence 1,2,3,4,10,12,14,15 and first XID available 100, string is:
 * (1-4;10;12;14-15|100)
 *
 */

/* PUBLIC */

std::string XID_map::String(void) {
	if (docRoot==NULL) THROW_AWAY(("document root is unknown"));
	return ( String( docRoot, true ) );
}

std::string XID_map::String(DOMNode *node, bool printFirstAvailXID) {
	std::vector<XID_t> xidList ;
	_internal_TraceTree( xidList, node );
	return StringFromList( xidList, printFirstAvailXID );
}

/* PRIVATE */

void XID_map::_internal_TraceTree( std::vector<XID_t> &xidList, DOMNode *node ) {
  if (node->hasChildNodes()) {
	  DOMNode *child = node->getFirstChild() ;
		while(child != NULL ) {
			_internal_TraceTree( xidList, child ) ;
			child = child->getNextSibling();
			}
		}
	xidList.push_back( getXIDbyNode( node ) );
	}

std::string XID_map::StringFromList( std::vector<XID_t> xidList, bool writeFirstAvail) {

	if (xidList.begin()==xidList.end()) THROW_AWAY(("List is empty"));

	// Write the first XID
	
	std::vector<XID_t>::iterator i = xidList.begin();
	std::vector<XID_t>::iterator precedent = i;
	std::string s = "(" + XID_map::xidString(*precedent) ;
	XID_t precedent_written = *precedent ;
	i++ ;
	
	// For all other XID, write when coming XID_t is not next in arithmetic sequence
	
	for(; i!=xidList.end(); i++) {
		if ( *i != (*precedent)+1 ) {
			if ( *precedent != precedent_written ) s+= "-" + XID_map::xidString(*precedent) ;
			s += ";" + XID_map::xidString(*i) ;
			precedent_written = *i ;
		}
		precedent = i ;
	}
	
	// Writes the last XID if not already written
	if (*precedent != precedent_written) {
		s+= "-" + XID_map::xidString( *precedent );
	}
		
	// Writes max XID used for the document ( if exists, e.g. not for subtree serialization )
	if ((writeFirstAvail)&&(firstAvailableXID != XID_INVALID )) s+= "|" + XID_map::xidString( firstAvailableXID );
	
	// Close string
	s+= ")" ;
	return s;
}

std::string XID_map::xidString(XID_t x) {
	char s[20];
	sprintf(s,"%d",(int)x);
	return std::string(s);
}

/*
 * Management of XID-2-Node mappings : Find functions
 */

bool XID_map::findNodeWithXID(const XID_t xid) {
	if ( nodeByXid->find(xid) != nodeByXid->end() ) return true;
	else return false;
};

DOMNode* XID_map::getNodeWithXID(const XID_t xid) {
	if ( nodeByXid->find(xid) != nodeByXid->end() ) {
		DOMNode* node =  (*nodeByXid)[xid];
		return node ;
	}else {
		THROW_AWAY(("could not find node with XID=%d",(int)xid));
	}
}

XID_t XID_map::getXIDbyNode(DOMNode *node) {
	if (node==NULL) THROW_AWAY(("can't find NULL DOMNode"));
	if (xidByNode->find( node ) != xidByNode->end()) {
		return (*xidByNode)[ node] ;
	} else {
		if (node->getNodeType()==DOMNode::TEXT_NODE) std::cerr << "CONTEXT:: text_node=" << XMLString::transcode(node->getNodeValue()) << std::endl ;
		else if (node->getNodeType()==DOMNode::ELEMENT_NODE) std::cerr << "CONTEXT:: element_node name=" << XMLString::transcode(node->getNodeName()) << std::endl ;
		else std::cerr << "Node type="<<node->getNodeType()<<std::endl;
		THROW_AWAY(("Node(Impl) not found"));
	}
}
	
/* FirstAvaibleXID */

XID_t XID_map::getFirstAvailableXID(void) {
	return firstAvailableXID ;
}

void XID_map::initFirstAvailableXID(XID_t xid) {
	if (firstAvailableXID!=XID_INVALID) THROW_AWAY(("can't init value is already a valid value"));
	if (xid==XID_INVALID) THROW_AWAY(("can't init with XID_INVALID value"));
	
	firstAvailableXID = xid;
}

XID_t XID_map::allocateNewXID(void) {
	if (firstAvailableXID==XID_INVALID) THROW_AWAY(("firstAvailableXID is XID_INVALID"));
	return firstAvailableXID++ ;
}

/*
 * Management of Node-2-XID mappings
 *
 * If FirstAvailableXID is valid, it will be updated if a greater XID is registered
 *
 */

void XID_map::registerNode( DOMNode *node, XID_t xid ) {
	if (xid==XID_INVALID) THROW_AWAY(("XID is INVALID"));
        if (node==NULL) THROW_AWAY(("can not register NULL DOMNode"));
          
	if (nodeByXid->find(xid)!=nodeByXid->end()) THROW_AWAY(("XID %d already used", (int)xid));
	if (xidByNode->find(node)!=xidByNode->end()) THROW_AWAY(("this node is already registered"));
          
	if ((firstAvailableXID!=XID_INVALID)&&(xid>=firstAvailableXID)) firstAvailableXID=xid+1;
	(*nodeByXid)[ xid ] = node ;
	(*xidByNode)[ node ] = xid ;
}

void XID_map::removeNode( DOMNode *node ) {
	XID_t xid = getXIDbyNode( node );
	if (!nodeByXid->erase( xid )) THROW_AWAY(("xid %d not found",(int)xid));
       
        if (!xidByNode->erase( node)) THROW_AWAY(("node pointer not found"));
	if (node==docRoot) docRoot=NULL;
}
        

/*
 * Management of Node-2-XID mappings at Subtree level
 */

void XID_map::mapSubtree( const char *str, DOMNode *node ) {
	XidMap_Parser parse( str );
	registerSubtree( parse, node );
}

void XID_map::registerSubtree( XidMap_Parser &parse, DOMNode *node ) {
	
	if (node==NULL) THROW_AWAY(("can not register NULL node"));
	
	if (!nodeByXid) nodeByXid = new std::map<XID_t, DOMNode*> ;
	if (!xidByNode) xidByNode = new std::map<DOMNode*, XID_t> ;
	
	firstAvailableXID = parse.getFirstAvailableXID( firstAvailableXID ) ;
	_internal_registerSubtree( parse, node );
	
	if (!parse.isListEmpty()) {
		while(!parse.isListEmpty()) {
			fprintf(stderr, "error: value %d not used\n",(int)parse.getNextXID());
		}
		THROW_AWAY(("Xid String contains more values than required"));
	}
}

void XID_map::_internal_registerSubtree( XidMap_Parser &parse, DOMNode *node ) {
	if ( node->hasChildNodes() ) {
	  DOMNode* child = node->getFirstChild() ;
		while(child!=NULL) {
		  _internal_registerSubtree( parse, child );
			child = child->getNextSibling() ;
		}
	}

	XID_t nodeXid = parse.getNextXID() ;
	registerNode( node, nodeXid );
}

void XID_map::removeSubtree( DOMNode *node ) {
  if ( node->hasChildNodes() ) {
	  DOMNode* child = node->getFirstChild() ;
		while(child!=NULL) {
		  removeSubtree( child );
			child = child->getNextSibling() ;
		}
	}
	removeNode( node );
}

/*
 *
 * ---- XID Map References Management : add/remove Reference ----
 *
 */


VersionManagerException addRefException( const std::string &cause ) {
	return VersionManagerException( "XID_map::addReference", cause ) ;
}
VersionManagerException removeRefException( const std::string &cause ) {
	return VersionManagerException( "XID_map::removeReference", cause ) ;
}
	
XID_map* XID_map::addReference(XID_map *target) {

	if (!target)                  THROW_AWAY(("argument is NULL pointer"));
	if (target->referenceCount<0) THROW_AWAY(("referenceCount (%d) invalid",target->referenceCount));

	target->referenceCount++ ;
	return target;
}
	
void XID_map::removeReference(XID_map* &inoutXidmapPtr) {

	if (!inoutXidmapPtr)                   THROW_AWAY(("argument is NULL pointer"));
	if (inoutXidmapPtr->referenceCount==0) THROW_AWAY(("referenceCount was NULL"));

	inoutXidmapPtr->referenceCount-- ;

	if (inoutXidmapPtr->referenceCount<0)  THROW_AWAY(("referenceCount (%d) invalid",inoutXidmapPtr->referenceCount));
	if (inoutXidmapPtr->referenceCount==0) {
		delete inoutXidmapPtr;
	}
	inoutXidmapPtr=NULL;
}

/** Some Tools about XIDs **/

XID_t XID_map::getXidFromAttribute(DOMNode* elem, const XMLCh *attrName, bool parenthesis) {
	if (elem==NULL) THROW_AWAY(("node is null"));
	if (elem->getNodeType()!=DOMNode::ELEMENT_NODE) THROW_AWAY(("node is not an element node"));
	DOMNode* attrNode = elem->getAttributes()->getNamedItem(attrName);
	if (attrNode==NULL) THROW_AWAY(("attribute <%s> not found", attrName));
	
	XyLatinStr xidStr(attrNode->getNodeValue());
	int xid = -1;
	int ok = -1 ;
	if (parenthesis) ok=sscanf(xidStr.localForm(), "(%d)", &xid);
	else ok=sscanf(xidStr.localForm(), "%d", &xid);
	
	if (ok<1) {
		THROW_AWAY(("wrong Xid format for attribute <%s> string <%s>", XyLatinStr(attrName).localForm(), xidStr.localForm()));
	}
	
	if (xid<0) THROW_AWAY(("Bad value %d for Xid", xid));
	return (XID_t)xid;
}
