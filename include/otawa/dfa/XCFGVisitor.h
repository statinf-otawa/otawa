/*
 * $Id$
 * Copyright (c) 2006 IRIT-UPS
 *
 * XCFGVisitor class interface.
 */
#ifndef OTAWA_DFA_XCFG_VISITOR_H
#define OTAWA_DFA_XCFG_VISITOR_H

#include <elm/assert.h>
#include <otawa/cfg.h>
#include <otawa/cfg/CFGCollector.h>
#include <elm/genstruct/FragTable.h>
#include <otawa/dfa/XIterativeDFA.h>
#include <elm/util/Pair.h>

namespace otawa { namespace dfa {

using namespace elm;
using namespace elm::genstruct;

extern Identifier<int> INDEX;

// XCFGVisitor class
template <class P>
class XCFGVisitor {
	typedef struct node_t {
		BasicBlock *bb;
		int cfg, to, from;
		inline node_t(void): bb(0), cfg(-1), to(-1), from(-1) { }
	} node_t;
	P& dom;
	const CFGCollection& cfgs;
	FragTable<node_t> nodes;
	int *offs;
	Vector<int> *preds;

public:

	XCFGVisitor(const CFGCollection& cfgs, P& domain);
	~XCFGVisitor(void); 
	
	typedef typename P::domain_t domain_t;
	inline domain_t *empty(void) { return dom.empty(); }
	inline void free(domain_t *d) { dom.free(d); }
 	inline domain_t *gen(int node)
		{ return dom.gen(cfgs[nodes[node].cfg], nodes[node].bb); }
 	inline domain_t *preserve(int node)
		{ return dom.preserve(cfgs[nodes[node].cfg], nodes[node].bb); }

	typedef Pair<CFG *, BasicBlock *> key_t;
	inline int index(const key_t& key)
		{ return offs[INDEX(key.fst)] + otawa::INDEX(key.snd); }

	inline int size(void) { return nodes.length(); }
	void visitPreds(XIterativeDFA< XCFGVisitor<P> >& engine, int node);
	void visitSuccs(XIterativeDFA< XCFGVisitor<P> >& engine, int node);
};

// Inlines
template <class D>
XCFGVisitor<D>::XCFGVisitor(const CFGCollection& _cfgs, D& domain)
:	dom(domain),
	cfgs(_cfgs),
	offs(new int[cfgs.count() + 1]),
	preds(new Vector<int>[cfgs.count()])
{
	// Compute CFG offsets
	int i = 0;
	offs[0] = 0;
	for(CFGCollection::Iterator cfg(cfgs); cfg; cfg++) {
		INDEX(cfg) = i++;
		offs[i] = offs[i - 1] + cfg->countBB();
	}
	nodes.alloc(offs[i]);
	
	// Record nodes and CFG next nodes
	// !!NOTE!! Does not support CFG ending with a branch on another CFG
	// !!TOCHECK!! For multicall
	int bbi = 0, cfgi = 0;
	for(CFGCollection::Iterator cfg(cfgs); cfg; cfg++, cfgi++)
		for(CFG::BBIterator bb(cfg); bb; bb++, bbi++) {
			nodes[bbi].bb = bb;
			nodes[bbi].cfg = cfgi;
			BasicBlock *next = 0;
			//CFG *called = 0;
			for(BasicBlock::OutIterator edge(bb); edge; edge++) {
				if(edge->kind() == Edge::CALL) {
					CFG *called = edge->calledCFG();
					int called_index = INDEX(called);
					nodes[bbi].to = offs[called_index];
					nodes[offs[cfgi] + next->number()].from =
						offs[called_index + 1] - 1;
					preds[called_index].add(bbi);
				}
				/*else
					next = edge->target();*/
			}
			/*if(called) {
				int called_index = INDEX(called);
				nodes[bbi].to = offs[called_index];
				nodes[offs[cfgi] + next->number()].from =
					offs[called_index + 1] - 1;
				preds[called_index].add(bbi);
			}*/
		}
}


template <class D>
XCFGVisitor<D>::~XCFGVisitor(void) {
	
	// Release INDEX on CFGs
	for(CFGCollection::Iterator cfg(cfgs); cfg; cfg++)
		cfg->removeProp(&INDEX);
	
	// Free memory
	delete [] offs;
	delete [] preds;
} 


template <class D>
void XCFGVisitor<D>::visitPreds(XIterativeDFA< XCFGVisitor<D> >& engine, int node)
{
	node_t info = nodes[node];
	if(info.bb->isEntry()) {
		Vector<int>& pred = preds[info.cfg];
		for(int i = 0; i < pred.length(); i++)
			engine.nextPred(pred[i]);
	}
	else if(info.from != -1)
		engine.nextPred(info.from);
	else
		for(BasicBlock::InIterator edge(info.bb); edge; edge++)
			engine.nextPred(offs[info.cfg] + edge->source()->number());
}


template <class D>
void XCFGVisitor<D>::visitSuccs(XIterativeDFA< XCFGVisitor<D> >& engine, int node) {
}

} } // otawa::dfa

#endif // OTAWA_DFA_XCFG_VISITOR_H
