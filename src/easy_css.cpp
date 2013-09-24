/*******************************************************************
 *                                                                 *
 * LCSS.cxx                                                        *
 * Implementation of standard Longest Common SubSequence Algorithm *
 *                                                                 *
 * Author: Gregory COBENA, Mars 2001                               *
 *                                                                 *
 *******************************************************************/

/*

 Introduction:

 Given two sequences, s1[1..m] and s2[1..n] of a pair of (value, weight),
 this computes the heaviest common subsequence, e.g. the heaviest set of elements
 that can be enumerated in the same order in s1 and s2
 
 Implementation Details:
 
 1) The two sequences are given as a vector, in which element 0 is *NOT* part of the sequence.
 For example sequence 1,2,3 is represented by the vector{999, 1, 2, 3} or vector{0, 1, 2, 3}
 
 2) s1 sequence *MUST* be of the form: x, 1, 2, 3, 4, 5, 6, ... n

 3) Weights are ignored
 
 4) The two sequences given as argument are modified: the value of elements that are not part
 of the resulting common subsequence is set to 0
 
*/

#include <cstdlib>
#include <vector>
#include "xydiff/VersionManagerException.hpp"
#include "CommonSubSequenceAlgorithms.hpp"

void easy_css(std::vector<wSequence> & s1, std::vector<wSequence> & s2) {

	unsigned int cursor1 = 1 ;
	unsigned int cursor2 = 1 ;
	
	while((cursor2<s2.size()) && (cursor1<s1.size())) {
	
		if (s1[cursor1].data==0) {
			vddprintf(("s1[%d] already removed\n", cursor1));
			cursor1++;
			}
		else if (s2[cursor2].data==0) {
			vddprintf(("s2[%d] already removed... but how is that possible????\n", cursor2));
			cursor2++;
			}
		else if (s1[cursor1].data==s2[cursor2].data) {
			vddprintf(("Ok s1[%d]==s2[%d]\n", cursor1, cursor2));
			cursor1++;
			cursor2++;
			}
		else {
			vddprintf(("throwing away s2[%d]\n", cursor2));
			unsigned int x = (unsigned int)s2[cursor2].data;
			if (x>=s1.size()) {
				fprintf(stderr, "INTERNAL ERROR : s2 item %d is not in s1\n", x);
				exit(-1);
			}
			s1[x].data = 0 ;
			s2[cursor2].data = 0 ;
			cursor2++ ;
			}
		}
	
#ifdef TESTING
	for(unsigned int k=1; k<s1.size(); k++) printf("s1: Keeping %d\n", s1[k].data);
	for(unsigned int k=1; k<s2.size(); k++) printf("s2: Keeping %d\n", s2[k].data);
#endif
	}

#ifdef TESTING
	
int main() {
	std::vector<wSequence> s1 ;
	s1.push_back(wSequence(-1, 999.99));
	s1.push_back(wSequence(1, 1.0));
	s1.push_back(wSequence(2, 1.0));
	s1.push_back(wSequence(3, 1.5));
	s1.push_back(wSequence(4, 1.0));
	s1.push_back(wSequence(5, 1.0));
	s1.push_back(wSequence(6, 10.0));
	s1.push_back(wSequence(7, 1.0));
	std::vector<wSequence> s2 ;
	s2.push_back(wSequence(-1, 999.99));
	s2.push_back(wSequence(2, 1.0));
	s2.push_back(wSequence(3, 1.0));
	s2.push_back(wSequence(4, 1.0));
	s2.push_back(wSequence(6, 1.0));
	s2.push_back(wSequence(1, 1.0));
	s2.push_back(wSequence(5, 1.3));
	s2.push_back(wSequence(7, 1.0));
	
	easy_css(s1, s2) ;
	}

#endif
