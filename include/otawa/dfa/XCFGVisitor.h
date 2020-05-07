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
#include <elm/data/FragTable.h>
#include <otawa/dfa/XIterativeDFA.h>
#include <elm/util/Pair.h>

namespace otawa { namespace dfa {

using namespace elm;

extern Identifier<int> INDEX;

// XCFGVisitor class
template <class P>
class XCFGVisitor {
	typedef struct node_t {
		Block *bb;
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
		{ return offs[key.fst->index()] + key.snd->index(); }

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
	for(CFGCollection::Iter cfg(cfgs); cfg(); cfg++) {
		INDEX(*cfg) = i++;
		offs[i] = offs[i - 1] + cfg->count();
	}
	nodes.alloc(offs[i]);
	
	// Record nodes and CFG next nodes
	int bbi = 0, cfgi = 0;
	for(CFGCollection::Iter cfg(cfgs); cfg(); cfg++, cfgi++)
		for(CFG::BlockIter bb = cfg->blocks(); bb(); bb++, bbi++) {
			nodes[bbi].bb = bb;
			nodes[bbi].cfg = cfgi;
			if(bb->isSynth()) {
				int called_index = bb->toSynth()->index();
				nodes[bbi].to = offs[called_index];
				ASSERT(bb->ins());
				nodes[offs[cfgi] + bb->ins()->sink()->index()].from = offs[called_index + 1] - 1;
				preds[called_index].add(bbi);
			}
		}
}


template <class D>
XCFGVisitor<D>::~XCFGVisitor(void) {
	
	// Release INDEX on CFGs
	for(CFGCollection::Iter cfg(cfgs); cfg(); cfg++)
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
		for(BasicBlock::EdgeIter edge = info.bb->ins(); edge(); edge++)
			engine.nextPred(offs[info.cfg] + edge->source()->index());
}


template <class D>
void XCFGVisitor<D>::visitSuccs(XIterativeDFA< XCFGVisitor<D> >& engine, int node) {
}

} } // otawa::dfa

#endif // OTAWA_DFA_XCFG_VISITOR_H
