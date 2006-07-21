/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	BBSequence.h -- interface of basic blocks sequence (path) class.
 */
 
#ifndef OTAWA_IPET_BBSEQUENCE_H
#define OTAWA_IPET_BBSEQUENCE_H

#include <otawa/cfg.h>
#include <otawa/ilp.h>
#include <otawa/properties.h>
#include <otawa/sim/State.h>
#include <elm/string.h>
#include <elm/util/Option.h>
#include <elm/genstruct/Vector.h>
#include <otawa/ipet/Delta.h>

namespace otawa{ namespace ipet {

class BBSequence;
class Delta;

class BBSequence: public PropList{
	friend class Delta;
	BBSequence(Delta *delta, BasicBlock *start);
	BBSequence(Delta *delta, elm::genstruct::Vector<BasicBlock*> *path);
	
	int _length;
	elm::Option<int> _delta;
	Delta *deltaProcessor;
	sim::State *ending_state;
	elm::genstruct::Vector<BasicBlock*> basicBlocks;
	
public:
	
	elm::genstruct::Vector<BBSequence*> *nexts() ;
	int delta();
	int time();
	inline int t();
	int simulate();
	inline int length();
	inline int l();
	elm::String makeVarName(ilp::System *system);
	ilp::Var *getVar(ilp::System *system);
	sim::State *getEndingState();
	BBSequence* sub(int begin, int end);
	bool equals(BBSequence &bbs);
	inline BBSequence* operator() (int begin, int end);
	inline bool operator== (BBSequence &bbs);
	inline bool operator!= (BBSequence &bbs);
	
	static int instructionsSimulated;
	static int nbDeltasCalculated;
};

inline int BBSequence::t() {return time();}
inline int BBSequence::l() {return length();}
inline int BBSequence::length() {return _length;}
inline bool BBSequence::operator== (BBSequence &bbs) {return equals(bbs);}
inline bool BBSequence::operator!= (BBSequence &bbs) {return !equals(bbs);}
inline BBSequence* BBSequence::operator() (int begin, int end){return sub(begin, end);}

} }

#endif /*OTAWA_IPET_BBSEQUENCE_H*/
