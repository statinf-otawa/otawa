/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/cfg/Edge.h -- interface of Edge class.
 */
#ifndef OTAWA_CFG_EDGE_H
#define OTAWA_CFG_EDGE_H

#include <assert.h>
#include <otawa/cfg/CFG.h>

namespace otawa {

// Classes
class BasicBlock;

// Edge class
class Edge: public PropList {
	friend class CFG;
public:
	typedef enum kind_t {
		NONE,
		TAKEN,
		NOT_TAKEN,
		CALL,
		VIRTUAL,
		VIRTUAL_CALL,
		VIRTUAL_RETURN
	} kind_t;
private:
	kind_t knd;
	BasicBlock *src, *tgt;
	void toCall(void);
public:
	Edge(BasicBlock *source, BasicBlock *target, kind_t kind = TAKEN);
	~Edge(void);
	inline BasicBlock *source(void) const { return src; };
	inline BasicBlock *target(void) const { return tgt; };
	inline kind_t kind(void) const { return knd; };
	inline CFG *calledCFG(void) const;
};


// Inlines
inline CFG *Edge::calledCFG(void) const {
	assert(knd == CALL);
	if(!tgt)
		return 0;
	else
		return ENTRY(tgt);
}


// Deprecated
typedef Edge::kind_t edge_kind_t;
#define EDGE_Null		Edge::NONE
#define EDGE_Taken		Edge::TAKEN
#define	EDGE_NotTaken	Edge::NOT_TAKEN
#define	EDGE_Call		Edge::CALL
#define EDGE_Virtual	Edge::VIRTUAL

} // otawa

#endif	// OTAWA_CFG_EDGE_H

