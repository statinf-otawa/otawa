/*
 *	BBPath class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_TSIM_BBPATH_H
#define OTAWA_TSIM_BBPATH_H

#include <otawa/cfg.h>
#include <otawa/ilp.h>
#include <otawa/sim/State.h>
#include <elm/genstruct/Vector.h>
#include <otawa/tsim/TreePath.h>


namespace otawa { namespace tsim {

class PathManager;

class BBPath: public PropList {
	friend class PathManager;
protected:
	BBPath(BasicBlock *start);
	BBPath(elm::genstruct::Vector<BasicBlock*> *path);
	//virtual ~BBPath();

	int _length;
	elm::genstruct::Vector<BasicBlock*> basicBlocks;
	//sim::State *ending_state;
	WorkSpace *last_framework_used; // Last framework used for simulation
	
	/*virtual elm::IteratorInst<BasicBlock*> *visit(void);
	virtual elm::MutableCollection<BasicBlock *> *empty(void);*/
	
	int simulate(WorkSpace *fw);
	//sim::State *getEndingState(FrameWork *fw);

public:
	static BBPath* getBBPath(BasicBlock *start);
	static BBPath* getBBPath(elm::genstruct::Vector<BasicBlock*> *path);

	elm::genstruct::Vector<BBPath*> *nexts() ;
	int time(WorkSpace *fw);
	int countInstructions();
	inline int t(WorkSpace *fw);
	inline int length();
	inline int l();
	inline BasicBlock* head();
	inline BasicBlock* tail();
	elm::String makeVarName();
	ilp::Var *getVar(ilp::System *system, bool explicit_names = false);
	BBPath* sub(int begin, int end);
	inline BBPath* operator() (int begin, int end);
	//inline Collection<BasicBlock*>& bbs();
	
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

io::Output& operator<<(io::Output& out, BBPath& path);

// BBPath inlines
inline int BBPath::t(WorkSpace *fw) {return time(fw);}
inline int BBPath::l() {return length();}
inline int BBPath::length() {return _length;}
inline BBPath* BBPath::operator() (int begin, int end){return sub(begin, end);}
//inline bool BBPath::operator== (BBPath &bbpath){return equals(bbpath);}
inline BasicBlock* BBPath::head() {return basicBlocks[0];}
inline BasicBlock* BBPath::tail() {return basicBlocks.top();}
//inline Collection<BasicBlock*>& BBPath::bbs(){return *this;}

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

} } // otawa::tsim

#endif /*OTAWA_TSIM_BBPATH_H*/
