/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/oshell/CFGCursor.h -- interface of CFGCursor class.
 */
#ifndef OTAWA_OSHELL_CFGCURSOR_H
#define OTAWA_OSHELL_CFGCURSOR_H

#include <elm/genstruct/SortedBinTree.h>
#include <elm/genstruct/DLList.h>
#include <otawa/cfg.h>
#include "oshell.h"

namespace otawa {

// CFGCursor class
class CFGCursor: public Cursor {
	CFG *cfg;
	bool built;
	genstruct::SortedBinTree< AutoPtr<BasicBlock> > bbs;
	void build(void);
	static id_t ID_Number;

	// Visitor for numbering basic blocks
	class BasicBlockVisitor
	: public genstruct::SortedBinTree< AutoPtr<BasicBlock> >::Visitor {
		int cnt;
	public:
		inline BasicBlockVisitor(void): cnt(0) { };
		int process(AutoPtr<BasicBlock> bb) {
			bb->set<int>(ID_Number, cnt++);
			return 1;
		}
	};
	
	// Visitor for listing the CFG
	class ListVisitor
	: public genstruct::SortedBinTree< AutoPtr<BasicBlock> >::Visitor {
		Output& out;
	public:
		inline ListVisitor(Output& _out): out(_out) { };
		int process(AutoPtr<BasicBlock> bb);
	};
	
public:
	CFGCursor(Cursor *back, CFG *_cfg);
	virtual void path(Output& out);	
	virtual void list(Output& out);
};

} // otawa

#endif // OTAWA_OSHELL_CFGCURSOR_H
