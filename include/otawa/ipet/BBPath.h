/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/ipet/BBPath.h -- BBPath class interface.
 */
#ifndef OTAWA_BBPATH_H
#define OTAWA_BBPATH_H

#include <otawa/cfg.h>
#include <otawa/ilp.h>
#include <otawa/sim/State.h>
#include <elm/genstruct/Vector.h>
#include <otawa/ipet/TreePath.h>


namespace otawa { namespace ipet {

class PathManager;

class BBPath: public PropList, private Collection<BasicBlock*> {
	friend class PathManager;
protected:
	BBPath(BasicBlock *start);
	BBPath(elm::genstruct::Vector<BasicBlock*> *path);
	//virtual ~BBPath();

	int _length;
	elm::genstruct::Vector<BasicBlock*> basicBlocks;
	//sim::State *ending_state;
	FrameWork *last_framework_used; // Last framework used for simulation
	
	virtual elm::IteratorInst<BasicBlock*> *visit(void);
	virtual elm::MutableCollection<BasicBlock *> *empty(void);
	
	int simulate(FrameWork *fw);
	//sim::State *getEndingState(FrameWork *fw);

public:
	static BBPath* getBBPath(BasicBlock *start);
	static BBPath* getBBPath(elm::genstruct::Vector<BasicBlock*> *path);

	elm::genstruct::Vector<BBPath*> *nexts() ;
	int time(FrameWork *fw);
	int countInstructions();
	inline int t(FrameWork *fw);
	inline int length();
	inline int l();
	inline BasicBlock* head();
	inline BasicBlock* tail();
	elm::String makeVarName();
	ilp::Var *getVar(ilp::System *system, bool explicit_names = false);
	BBPath* sub(int begin, int end);
	inline BBPath* operator() (int begin, int end);
	inline Collection<BasicBlock*>& bbs();
	
	// Iterator
	class BBIterator: public elm::PreIterator<BBIterator, BasicBlock *> {
		elm::genstruct::Vector<BasicBlock *>& bbs;
		int pos;
	public:
		inline BBIterator(BBPath *bbpath);
		inline bool ended(void) const;
		inline BasicBlock *item(void) const;
		inline void next(void);
	};
	
	static int instructions_simulated;
	//static GenericIdentifier<TreePath<BasicBlock*,BBPath*>*> TREE;
	
	// Comparison
	//bool equals (BBPath &bbpath);
	//inline bool operator== (BBPath &bbpath);
};

// BBPath inlines
inline int BBPath::t(FrameWork *fw) {return time(fw);}
inline int BBPath::l() {return length();}
inline int BBPath::length() {return _length;}
inline BBPath* BBPath::operator() (int begin, int end){return sub(begin, end);}
//inline bool BBPath::operator== (BBPath &bbpath){return equals(bbpath);}
inline BasicBlock* BBPath::head() {return basicBlocks[0];}
inline BasicBlock* BBPath::tail() {return basicBlocks.top();}
inline Collection<BasicBlock*>& BBPath::bbs(){return *this;}

// BBIterator inlines
inline BBPath::BBIterator::BBIterator(BBPath *bbpath)
: bbs(bbpath->basicBlocks), pos(0) {};
inline bool BBPath::BBIterator::ended(void) const {
	return pos >= bbs.length();
};
inline BasicBlock *BBPath::BBIterator::item(void) const {
	return bbs[pos];
};
inline void BBPath::BBIterator::next(void) {
	pos++;
};

} } // otawa::ipet

#endif /*OTAWA_BBPATH_H*/
