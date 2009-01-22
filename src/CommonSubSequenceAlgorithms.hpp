/* 
 * Here are the headers of various implementation of CSS Algorithms
 *
 * The Goal of a Common SubSequence Algorithm is, given to input Sequences,
 * to find a subsequence that exists in both of them
 * There may be many solutions, and various algorithms will try to
 * find the longest common subsequence, or the subsequence that has the most
 * important value (if sequence elements are given a weight)
 *
 * The common subsequence is used in the context of children of a specific
 * nodes. Although some child nodes have the same parent in the old and new
 * version of the document, their position may have changed.
 * We ignore position changes due to operation on their sibling.
 * Still, some nodes may have been reordered, for example nodes 1, 2, 3
 * may become nodes 2, 3, 1
 * Finding the longest common subsequence (here 2, 3) gives us the minimal
 * set of move operations that transforms the old sequence of children into
 * the new one. Here: <move node 1 from position 1 to position 3>
 *
 *
 * Our implementation of Longest Common SubSequence has a time cost in
 * n1*n2, where n1 and n2 are the length of both input sequences
 * The subsequence we identify is the 'longest' if the weight is the same for
 * all children, but in a more general weight is the subsequence which has
 * the most important weight.
 * So the set of operations that transforms the old list of children into
 * the new one is the 'smallest' set of operations, e.g. in general the set
 * of operations with the lowest weight.
 *
 * The other routine, called easy_css is a quick version which time cost is
 * linear in the size of input sequences. Of course the result is not optimal,
 * and weights are ignored
 *
 *
 * USAGE:
 * INPUT-1: S1[1]...S1[S1.size()-1] represent the first sequence
 * (note that S1[0] is ignored)
 * They must be equal to 1, 2, 3, ..., S1.size()-1
 * (because of easy_lcss implementation)
 * INPUT-2: S2[1]...S2[S2.size()-1] represent the second sequence
 * (note that S2[0] is ignored)
 *
 * RESULT:
 * Both sequences are modified, in that some element values are set to 0.
 * This mean that the element is NOT part of the subsequence.
 * For all other elements, the value has not changed, and the element is part
 * of the identified common subsequence.
 *
 */

#ifndef COMMONSUBSEQUENCEALGORITHMS_HXX__
#define COMMONSUBSEQUENCEALGORITHMS_HXX__

#define _PTHREAD
#include <vector>

class wSequence {
	public:
		wSequence(int d, float w) { data=d; weight=w; } ;
		int data ;
		float weight ;
	} ;


void easy_css(std::vector<wSequence> & s1, std::vector<wSequence> & s2) ;
void lcss(std::vector<wSequence> & s1, std::vector<wSequence> & s2) ;

#endif
