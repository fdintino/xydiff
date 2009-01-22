#ifndef DELTA_SORT_OPERATIONS_HXX__
#define DELTA_SORT_OPERATIONS_HXX__

#include "include/XID_DOMDocument.hpp"
#include "include/XID_map.hpp"

#include "xercesc/dom/DOMNode.hpp"

#include <vector>
#include <map>
#include <queue>

class SortDeleteOperationsEngine {
	public:
		SortDeleteOperationsEngine(XID_DOMDocument *sourceDoc, xercesc_3_0::DOMNode *firstOperatorSibling);
		xercesc_3_0::DOMNode* getNextDeleteOperation(void);
		bool isListEmpty(void);
	private:
		void registerOperations(XID_DOMDocument *sourceDoc, xercesc_3_0::DOMNode *firstOperatorSibling);
		void unrollList(XID_DOMDocument *sourceDoc, xercesc_3_0::DOMNode *node);
		std::vector<xercesc_3_0::DOMNode*> theList ;
		unsigned long count ;
		// map<XID_t, DOM_Node> operationBySourceNode ;
		std::map<long, xercesc_3_0::DOMNode*> operationBySourceNode ;
	} ;

class InsertOpWithPos {
	public:
		InsertOpWithPos(xercesc_3_0::DOMNode *_op);
		xercesc_3_0::DOMNode* op ;
		int pos ;
		XID_t parentXID ;
	};

class cmpSiblingInsertOrder {
	public:
		bool operator() (InsertOpWithPos op1, InsertOpWithPos op2) const;
	};
	
typedef std::priority_queue<InsertOpWithPos, std::vector<InsertOpWithPos>, cmpSiblingInsertOrder> siblingList ;

class SortInsertOperationsEngine {
	public:
		SortInsertOperationsEngine(XID_DOMDocument *sourceDoc, xercesc_3_0::DOMNode *firstOperatorSibling);
		xercesc_3_0::DOMNode* getNextInsertOperation(void);
		bool isListEmpty(void);
	private:
		std::vector<siblingList> insertList ;
		std::map<long, unsigned int> listIndexByParentXID ;
		unsigned long count ;
		XID_DOMDocument* theDoc ;
	} ;
	
#endif
