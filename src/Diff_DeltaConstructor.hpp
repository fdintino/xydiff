#ifndef DIFF_DELTACONSTRUCTOR_HXX
#define DIFF_DELTACONSTRUCTOR_HXX

#include "Tools.hpp"
#include "include/XID_map.hpp"
#include "include/XID_DOMDocument.hpp"

#include <string>
#include <vector>

extern unsigned long long int   clocksCDDinitDomDelta      ;
extern unsigned long long int   clocksCDDprepareVectors    ;
extern unsigned long long int   clocksCDDsaveXidMap        ;
extern unsigned long long int   clocksCDDcomputeWeakMove   ;
extern unsigned long long int   clocksCDDdetectUpdate      ;
extern unsigned long long int   clocksCDDaddAttributeOps   ;
extern unsigned long long int   clocksConstructDOMDelta    ;

class NodesManager ;

class DeltaConstructor {
	public:
		DeltaConstructor(class NodesManager *incMappings, const char *from, XID_DOMDocument* fromXDD, const char *to, XID_DOMDocument* toXDD, bool IncIgnoreUnimportantData=false) ;
		XID_DOMDocument *getDeltaDocument(void) ;
		
		void constructDeltaDocument(void) ;

	private:
		void ConstructDeleteScript( int v0nodeID, bool ancestorDeleted ) ;
		void ConstructInsertScript( int v1nodeID, bool ancestorInserted ) ;
		void AddAttributeOperations( int v1nodeID ) ;
		xercesc_3_0::DOMNode* deltaDoc_ImportInsertTree( int v1nodeID, std::vector<XID_t> &xidList );

		XID_DOMDocument *v0XML, *v1XML ;
		std::string v0filename, v1filename ;

		class NodesManager *nodesManager ;
		
		int moveCount, updateCount ;

		XID_DOMDocument* deltaDoc ;
		xercesc_3_0::DOMNode*        scriptRoot ;
	
		bool ignoreUnimportantData ;
	} ;

#endif
