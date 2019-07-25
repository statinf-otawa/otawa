/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	prog/ExpNode.h -- ExpNode class.
 */
#ifndef OTAWA_EXP_NODE_H
#define OTAWA_EXP_NODE_H

#include <elm/assert.h>
#include <otawa/ilp.h>

namespace otawa {
	
class ExpNode {
public:
	typedef enum {
		NONE,
		CST,
		VAR,
		POS,
		NEG,
		ADD,
		SUB,
		MUL,
		DIV
	} kind_t;
	
private:
	kind_t knd;
	union {
		ExpNode *una;
		struct {
			ExpNode *arg1;
			ExpNode *arg2;
		} bin;
		ilp::Var *var;
		double cst;
	} args;
	
public:
	inline ExpNode(ilp::Var *v);
	inline ExpNode(double cst);
	inline ExpNode(kind_t kind, ExpNode *arg);
	inline ExpNode(kind_t kind, ExpNode *arg1, ExpNode *arg2);
	
	inline kind_t kind(void) const;
	inline ilp::Var *var(void) const;
	inline double cst(void) const;
	inline ExpNode *arg(void) const;
	inline ExpNode *arg1(void) const;
	inline ExpNode *arg2(void) const;
};

// Inlines
inline ExpNode::ExpNode(ilp::Var *var): knd(VAR) {
	args.var = var;
}

inline ExpNode::ExpNode(double cst): knd(CST) {
	args.cst = cst;
}

inline ExpNode::ExpNode(kind_t kind, ExpNode *arg): knd(kind) {
	ASSERT(knd == POS || knd == NEG);
	args.una = arg;
}

inline ExpNode::ExpNode(kind_t kind, ExpNode *arg1, ExpNode *arg2): knd(kind) {
	ASSERT(knd >= ADD);
	args.bin.arg1 = arg1;
	args.bin.arg2 = arg2;
}
	
inline ExpNode::kind_t ExpNode::kind(void) const {
	return knd;
}
	
inline ilp::Var *ExpNode::var(void) const {
	ASSERT(knd == VAR);
	return args.var;
}

inline double ExpNode::cst(void) const {
	ASSERT(knd == CST);
	return args.cst;
}

inline ExpNode *ExpNode::arg(void) const {
	ASSERT(knd == POS || knd == NEG);
	return args.una;
}

inline ExpNode *ExpNode::arg1(void) const {
	ASSERT(knd >= ADD);
	return args.bin.arg1;
}

inline ExpNode *ExpNode::arg2(void) const {
	ASSERT(knd >= ADD);
	return args.bin.arg2;
}

} // otawa

#endif	// OTAWA_EXP_NODE_H
