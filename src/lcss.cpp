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
 
 2) The two sequences given as argument are modified: the value of elements that are not part
 of the resulting common subsequence is set to 0
 
*/

#include "CommonSubSequenceAlgorithms.hpp"

#include <stdio.h>
#define _PTHREAD
#include <vector>

//#define DEBUG_VERBOSE
//#define TESTING

#ifdef DEBUG_VERBOSE
#define vddprintf(a) printf a
#else
#define vddprintf(a)
#endif


inline char min3(float a, float b, float c) {
	if (a<=b) {
		if (a<=c) return 'A' ;
		if (c<=a) return 'C' ;
		}
	if (b<=c) return 'B' ;
	return 'C' ;
	}
	
class lcssEx {
	public:
		lcssEx(const char *errorMsg) {
			fprintf(stderr, "lcss.cxx(Longest Common SubSequence) error: %s\n",	errorMsg) ;
			} ;
	} ;

/* 
 * Longest Common Sub-Sequence
 */

void lcss(std::vector<wSequence> & s1, std::vector<wSequence> & s2) {
	
	// init
	
	unsigned int l1 = s1.size() ;
	unsigned int l2 = s2.size() ;
	// For large values, can NOT allocate on stack
	//float cost[l2][l1] ;
	//char origin[l2][l1] ;
	float** cost   = new float*[l2];
	char**  origin = new char* [l2];
	for(unsigned int j=0; j<l2; j++) {
		cost[j]  = new float[l1];
		origin[j]= new char [l1];
	}
	
	// Transforming empty string into empty string has no cost
	cost[0][0]=0.0 ;
	origin[0][0]='Z' ;

	// Transforming parts of S1 into empty string is just deleting
	for(unsigned int i=1; i<l1; i++) {
		cost[0][i]=cost[0][i-1]+s1[i].weight ;
		origin[0][i]='A';
		}
	// Transforming empty string into parts of S2 is just inserting
	for(unsigned int j=1; j<l2; j++) {
		cost[j][0]=cost[j-1][0]+s2[j].weight ;
		origin[j][0]='B';
		}

	// compute paths cost which transforms S1 into S2
	
	for(unsigned int j=1; j<l2; j++) {
		for(unsigned int i=1; i<l1; i++) {
			vddprintf(("Computing cost(i=%3d,j=%3d)\n", i, j));

			// delete item i in S1 and use tranformation of  S1[1..i-1] into S2[1..j]
			float deleteCost = s1[i].weight + cost[j][i-1] ;
			vddprintf(("    delete cost is %f\n", deleteCost ));
			// use tranformation of  S1[1..i] into S2[1..j-1] and insert S2[j]
			float insertCost = cost[j-1][i] + s2[j].weight ;
			vddprintf(("    insert cost is %f\n", insertCost ));
			
			if (s1[i].data!=s2[j].data) {
				if (deleteCost<insertCost) {
					cost[j][i]=deleteCost ;
					origin[j][i]='A' ;
					}
				else {
					cost[j][i]=insertCost ;
					origin[j][i]='B' ;
					}
				}
			else {
				float keep = cost[j-1][i-1] ;
				vddprintf(("    keep cost is %f\n", keep ));
				origin[j][i] = min3( deleteCost, insertCost, keep );
				if (origin[j][i]=='A') cost[j][i]=deleteCost ;
				else if (origin[j][i]=='B') cost[j][i]=insertCost ;
				else if (origin[j][i]=='C') cost[j][i]=keep ;
				}
				
			vddprintf(("cost(i=%3d,j=%3d)=%f, via %c\n", i, j, cost[j][i], origin[j][i]));
			}
		}
	
	// trace best path
	
	unsigned int i=l1-1 ;
	unsigned int j=l2-1 ;
	unsigned int balance = 0 ; // Delete/Insert balance
	
	while((i>0)||(j>0)) {
		if ((i<0)||(j<0)) throw lcssEx("Inconsistant computing: negative index!") ;
		vddprintf(("at (i=%3d,j=%3d) cost=%f origin=%c\n", i, j, cost[j][i], origin[j][i]));
		switch(origin[j][i]) {
			case 'A':
				vddprintf(("Delete %d\n", s1[i].data));
				s1[i].data = 0 ;
				i-- ;
				balance++ ;
				break;
			case 'B':
				vddprintf(("Insert %d\n", s2[j].data));
				s2[j].data = 0 ;
				j-- ;
				balance-- ;
				break;
			case 'C':
				if (s1[i].data!=s2[j].data) throw lcssEx("Inconsistant computing: Character not equal as supposed!") ;
				vddprintf(("Do nothing on %d\n", s1[i].data));
				i-- ;
				j-- ;
				break;
			}
		}
	
	if (balance!=0) {
		fprintf(stderr, "LongestCommonSubSequence Warning: Insert/Delete balance is not null!\n");
	}
	
#ifdef TESTING
	for(unsigned int k=1; k<s1.size(); k++) printf("s1: Keeping %d\n", s1[k].data);
	for(unsigned int k=1; k<s2.size(); k++) printf("s2: Keeping %d\n", s2[k].data);
#endif

	for(unsigned int j=0; j<l2; j++) {
		delete [] cost[j];
		delete [] origin[j];
	}
	delete [] cost;
	delete [] origin;
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
	
	lcss(s1, s2) ;
	}

#endif
