/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/cfg/Edge.h -- interface of Edge class.
 */
#ifndef OTAWA_CFG_EDGE_H
#define OTAWA_CFG_EDGE_H

namespace otawa {

// Edge kind
typedef enum edge_kind_t {
	EDGE_Null,
	EDGE_Taken,
	EDGE_NotTaken,
	EDGE_Call
} edge_kind_t;

// Classes
class BasicBlock;

// Edge class
class Edge: public PropList {
	edge_kind_t knd;
	BasicBlock *src, *tgt;
public:
	Edge(BasicBlock *source, BasicBlock *target, edge_kind_t kind = EDGE_Taken);
	~Edge(void);
	inline BasicBlock *source(void) const { return src; };
	inline BasicBlock *target(void) const { return tgt; };
	inline edge_kind_t kind(void) const { return knd; };
};


} // otawa

#endif	// OTAWA_CFG_EDGE_H

