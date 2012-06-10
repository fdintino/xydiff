#include "DeltaSortOperations.hpp"
#include "xydiff/DeltaException.hpp"
#include "xydiff/XID_map.hpp"
#include "xydiff/XyLatinStr.hpp"
#include "xydiff/XyInt.hpp"

#include "xercesc/dom/DOMNamedNodeMap.hpp"
#include "xercesc/dom/DOMElement.hpp"
#include "xercesc/util/XMLString.hpp"

XERCES_CPP_NAMESPACE_USE

/*                                       */
/*                                       */
/* ---- ++++ DELETE OPERATIONS ++++ ---- */
/*                                       */
/*                                       */

SortDeleteOperationsEngine::SortDeleteOperationsEngine(XID_DOMDocument *sourceDoc, DOMNode* firstOperatorSibling) {
	registerOperations(sourceDoc, firstOperatorSibling);
	DOMNode* docRoot = sourceDoc->getDocumentElement();
	unrollList(sourceDoc, docRoot);
	count=0;
	}
	
void SortDeleteOperationsEngine::registerOperations(XID_DOMDocument *sourceDoc, DOMNode *firstOperatorSibling) {
	
	DOMNode* op=firstOperatorSibling;
	XMLCh tempStr[3];
	
	while(op!=NULL) {
		XMLString::transcode("d", tempStr, 2);
		if (XMLString::equals(op->getNodeName(),tempStr)) {
			XMLString::transcode("xm", tempStr, 2);
			char *xidmapStr = XMLString::transcode(op->getAttributes()->getNamedItem(tempStr)->getNodeValue());
			XidMap_Parser parse(xidmapStr) ;
			XID_t myXid = parse.getRootXID();
			operationBySourceNode[(long)myXid]=op;
			XMLString::release(&xidmapStr);
			}
		op=op->getNextSibling();
		}
	
	}

void SortDeleteOperationsEngine::unrollList(XID_DOMDocument *sourceDoc, DOMNode *node) {
	
	if (node->hasChildNodes()) {
		DOMNode* child = node->getLastChild();
		while(child!=NULL) {
			unrollList(sourceDoc, child);
			child=child->getPreviousSibling();
			}
		}
	
	XID_t myXid = sourceDoc->getXidMap().getXIDbyNode(node);
	std::map<XID_t, DOMNode*>::iterator p = operationBySourceNode.find((long)myXid);
	if (p!=operationBySourceNode.end()) theList.push_back(p->second);

	}

bool SortDeleteOperationsEngine::isListEmpty(void) {
	return(count>=theList.size());
	}

DOMNode* SortDeleteOperationsEngine::getNextDeleteOperation(void) {
	if (count>=theList.size()) THROW_AWAY(("count %d larger than list size %d",(int)count,(int)theList.size()));
	return theList[count++];
	}

/*                                       */
/*                                       */
/* ---- ++++ INSERT OPERATIONS ++++ ---- */
/*                                       */
/*                                       */
	
/*******************************************************************
 *                                                                 *
 * Function Object that compare two nodes to decide which one      *
 * should be tried first.                                          *
 *                                                                 *
 * if F(x,y) is true, that means <y> will be placed first          *
 * (highest priority)                                              *
 * Here the priority is given to the first sibling (insert order ) *
 *                                                                 *
 *******************************************************************/

InsertOpWithPos::InsertOpWithPos(DOMNode *_op) {
	op=_op;
	XMLCh tempStr[4];
	XMLString::transcode("pos", tempStr, 3);
	pos = (int)XyInt(op->getAttributes()->getNamedItem(tempStr)->getNodeValue());
	XMLString::transcode("par", tempStr, 3);
	parentXID = (XID_t)(int)XyInt(op->getAttributes()->getNamedItem(tempStr)->getNodeValue());
	}

bool cmpSiblingInsertOrder::operator() (InsertOpWithPos op1, InsertOpWithPos op2) const {
	XMLCh tempStr[4];
	XMLString::transcode("par", tempStr, 3);
	if (!XMLString::equals(
						   op1.op->getAttributes()->getNamedItem(tempStr)->getNodeValue(),
						   op2.op->getAttributes()->getNamedItem(tempStr)->getNodeValue())) {
		THROW_AWAY(("two supposed sibling insert operations do not have the same parents"));
		}
	return (op1.pos>op2.pos);
	}

SortInsertOperationsEngine::SortInsertOperationsEngine(XID_DOMDocument *sourceDoc, DOMNode *firstOperatorSibling) {

	DOMNode* op=firstOperatorSibling;
	XMLCh iStr[2];
	XMLString::transcode("i", iStr, 1);
	while(op!=NULL) {
		if ( XMLString::equals(op->getNodeName(), iStr) ) {

			InsertOpWithPos iOp(op);
			
			if (listIndexByParentXID.find((long)iOp.parentXID)==listIndexByParentXID.end()) {
				listIndexByParentXID[iOp.parentXID]=insertList.size();
				siblingList s ;
				s.push(iOp);
				insertList.push_back(s);
				}
			else {
				unsigned int index=listIndexByParentXID[iOp.parentXID];
				insertList[index].push(iOp);
				}
			
			}
		op=op->getNextSibling();
		}

	count=0;
	theDoc=sourceDoc ;
	}
	
DOMNode* SortInsertOperationsEngine::getNextInsertOperation(void) {
	if (insertList.size()==0) THROW_AWAY(("insert list is empty"));
	
	int countSurvey=insertList.size();
	while(!theDoc->getXidMap().findNodeWithXID(insertList[count].top().parentXID)) {
		count=(count+1)%insertList.size();
		countSurvey--;
		if (countSurvey<0) THROW_AWAY(("this is a bad Delta: can not find any insert with existing parent. cross-dependencies??"));
		}

	DOMNode* op = insertList[count].top().op;
	insertList[count].pop();
	if (insertList[count].empty()) {
		std::vector<siblingList>::iterator i=insertList.begin();
		i += count ;
		(void)insertList.erase(i);
		}
	
	return op ;
	}

bool SortInsertOperationsEngine::isListEmpty(void) {
	return(insertList.size()==0);
	}
	
