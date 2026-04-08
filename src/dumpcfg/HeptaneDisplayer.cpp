/*
 *	$Id$
 *	Copyright (c) 2025, StatInf.
 *
 *	src/dumpcfg/HeptaneDisplayer.cpp -- HeptaneDisplayer class implementation.
 */

#include <elm/io.h>
#include <otawa/cfg/features.h>
#include "HeptaneDisplayer.h"
#include <otawa/prog/Process.h>
#include <otawa/prog/File.h>
#include <string>
#include <otawa/cfg/features.h>
#include <otawa/cfg/Loop.h>
#include <otawa/cfg/ContextTree.h>
#include <otawa/flowfact/features.h>

using namespace elm;
using namespace otawa;

/**
 */
Identifier<int> HeptaneDisplayer::OFFSET("", -1);

/**
 */
HeptaneDisplayer::HeptaneDisplayer(void): Displayer("HeptaneDisplayer", Version(1, 0, 0)), cfg_cnt(0) {
	require(otawa::COLLECTED_CFG_FEATURE);
	require(EXTENDED_LOOP_FEATURE);
	require(CONTEXT_TREE_BY_CFG_FEATURE);
}

void test(ContextTree *tree, HashMap<BasicBlock*, t::uint32> *loop_id, t::uint32 *glob_id) {
	for(ContextTree::ChildrenIterator child(tree); child(); child++) {
		if(child->kind() == ContextTree::LOOP) {
			loop_id->add(child->bb(), (*glob_id)++);
			test(*child, loop_id, glob_id);
		}
	}
}

Vector<Block*> test2(CFG *cfg, ContextTree *tree, HashMap<BasicBlock*, t::uint32> &loop_id, HashMap<Block*, t::uint32> &node_id, HashMap<Edge*, t::uint32> &edge_id) {
	Vector<Block*> parent_nested;
	for(ContextTree::ChildrenIterator child(tree); child(); child++) {
		if(child->kind() == ContextTree::LOOP) {
			ContextualPath path;
			path.push(ContextualStep::FUNCTION, cfg->address());
			int maxiter = path(MAX_ITERATION, child->bb()->first()).get();

			Vector<Block*> nested = test2(cfg, *child, loop_id, node_id, edge_id);

			Loop *loop = Loop::of(child->bb());
			cout << "\t\t<LOOP id=\""<<loop_id[child->bb()].get()<<"\" head=\""<<node_id.get(child->bb(), -1)<<"\" nodes=\"";
			for(Block *b : loop->blocks()) {
				cout << node_id[b].get() << ", ";
				parent_nested.add(b);
			}
			for(Block *b : nested) {
				cout << node_id[b].get() << ", ";
				parent_nested.add(b);
			}
			cout << "\" backedges=\"";
			for(Edge *e : loop->backs())
				cout << edge_id[e].get() << ", ";
			cout << "\">\n";
			cout << "\t\t\t<ATTRS_LIST>\n";
			cout << "\t\t\t\t<ATTR type=\"integer\" name=\"maxiter\" value=\""<<maxiter<<"\" />\n";
			cout << "\t\t\t</ATTRS_LIST>\n";
			cout << "\t\t</LOOP>\n";
		}
	}
	return parent_nested;
}

/**
 */
void HeptaneDisplayer::processWorkSpace(WorkSpace *ws) {
	auto cfgs = COLLECTED_CFG_FEATURE.get(ws);

	t::uint32 glob_id = 1;
	HashMap<CFG*, t::uint32> cfg_id;
	HashMap<Block*, t::uint32> node_id;
	HashMap<Edge*, t::uint32> edge_id;
	HashMap<Block*, HashMap<address_t, t::uint32>*> inst_id;
	HashMap<BasicBlock*, t::uint32> loop_id;
	for(CFG* cfg : *cfgs) {
		cfg_id.add(cfg, glob_id++);
		for(Block* v: cfg->vertices()) {
			node_id.add(v, glob_id++);
			if(v->isBasic()) {
				BasicBlock *bb = v->toBasic();
				inst_id.add(v, new HashMap<address_t,t::uint32>());
				for(BasicBlock::InstIter i(bb); i(); i++) {
					inst_id[v].get()->add(i->address(), glob_id++);
				}
			}
			else if(v->isCall()) {
				SynthBlock *bb = v->toSynth();
				auto prevnode = v->inEdges().begin();
				BasicBlock *prevbb = prevnode->source()->toBasic();
				Inst *i = prevbb->control();
				inst_id.add(v, new HashMap<address_t,t::uint32>());
				inst_id[v].get()->add(i->address(), glob_id++);
			}
			for(Edge* e: v->outEdges()) {
				edge_id.add(e, glob_id++);
			}
		}
		ContextTree *ctree = CONTEXT_TREE(cfg);
		// for(ContextTree::ChildrenIterator child(ctree); child(); child++) {
		// 	if(child->kind() == ContextTree::LOOP) {
		// 		loop_id.add(child->bb(), glob_id++);
		// 	}
		// }
		test(ctree, &loop_id, &glob_id);
	}
	
	int entry = -1;
	string entry_name = TASK_INFO_FEATURE.get(ws)->entryName();
	for(auto cfg : *cfgs) {
		if(cfg->name() == entry_name) {
			entry = cfg_id[cfg];
			break;
		}
	}

	cout << "<?xml version=\"1.0\" ?>\n";
	cout << "<!DOCTYPE PROGRAM SYSTEM 'cfglib.dtd'>\n";
	cout << "<PROGRAM id=\"0\" name=\"" << ws->process()->program()->name() << "\" entry=\"" << entry << "\">\n";

	// process CFGs
	for(auto cfg : *cfgs) {
		std::string startnode = "";
		std::string endnodes = "";
		for(auto v: cfg->vertices()) {
			if(v->isEntry()) {
				auto nextnode = v->outEdges().begin();
				startnode = std::to_string(node_id[nextnode->sink()].get());
			}
			else if(v->isExit()) {
				for(Edge *e : v->inEdges())
					endnodes += std::to_string(node_id[e->source()].get())+", ";
			}
		}

		cout << "\t<CFG id=\"" << cfg_id[cfg].get() << "\" name=\"" << cfg->name() << "\" startnode=\"" << String(startnode.c_str()) << "\" endnodes=\""<< String(endnodes.c_str()) << "\">" << io::endl;
		for(auto v: cfg->vertices()) {
			if(v->isBasic()) {
				cout << "\t\t<NODE id=\"" << node_id[v].get() << "\" type=\"BasicBlock\">\n";
				BasicBlock *bb = v->toBasic();
				for(BasicBlock::InstIter i(bb); i(); i++) {
					cout << "\t\t\t<INSTRUCTION id=\""<<inst_id[v].get()->get(i->address()).value()<<"\" asm_type=\"Code\" code=\"";
					i->dump(cout);
					cout << "\">\n";
					cout << "\t\t\t\t<ATTRS_LIST>\n";
					cout << "\t\t\t\t\t<ATTR type=\"address\" name=\"address\">\n";
					cout << "\t\t\t\t\t\t<ACCES type=\"read\" seg=\"code\" varname=\"\" precision=\"1\">\n";
					cout << "\t\t\t\t\t\t\t<ADDRSIZE begin=\"0x"<<ot::address(i->address())<<"\" size=\""<<i->size()<<"\" />\n";
					cout << "\t\t\t\t\t\t</ACCES>\n";
					cout << "\t\t\t\t\t</ATTR>\n";
					cout << "\t\t\t\t</ATTRS_LIST>\n";
					cout << "\t\t\t</INSTRUCTION>\n";
				}
				cout << "\t\t</NODE>\n";
			}
			else if(v->isCall()) {
				SynthBlock *bb = v->toSynth();
				auto prevnode = v->inEdges().begin();
				BasicBlock *prevbb = prevnode->source()->toBasic();
				Inst *i = prevbb->control();
				if(bb->callee())
					cout << "\t\t<NODE id=\"" << node_id[v].get() << "\" type=\"FunctionCall\" called=\""<<(bb->callee() ? bb->callee()->name() : "<unknown>") <<"\">\n";
				else //indirect call
					cout << "\t\t<NODE id=\"" << node_id[v].get() << "\" type=\"BasicBlock\">\n";
					
				cout << "\t\t\t<INSTRUCTION id=\""<<inst_id[v].get()->get(i->address()).value()<<"\" asm_type=\"Code\" code=\"";
				i->dump(cout);
				cout << "\">\n";
				cout << "\t\t\t\t<ATTRS_LIST>\n";
				cout << "\t\t\t\t\t<ATTR type=\"address\" name=\"address\">\n";
				cout << "\t\t\t\t\t\t<ACCES type=\"read\" seg=\"code\" varname=\"\" precision=\"1\">\n";
				cout << "\t\t\t\t\t\t\t<ADDRSIZE begin=\"0x"<<ot::address(i->address())<<"\" size=\""<<i->size()<<"\" />\n";
				cout << "\t\t\t\t\t\t</ACCES>\n";
				cout << "\t\t\t\t\t</ATTR>\n";
				cout << "\t\t\t\t</ATTRS_LIST>\n";
				cout << "\t\t\t</INSTRUCTION>\n";
				cout << "\t\t</NODE>\n";

			}
		}
		for(auto v: cfg->vertices()) {
			if(v->isEntry() || v->isExit())
				continue;
			t::uint32 src_id = node_id[v].get();
			for(auto e: v->outEdges()) {
				if(e->sink()->isExit())
					continue;
				t::uint32 dst_id = node_id.get(e->sink(), -1);
				cout << "\t\t<EDGE id=\""<<edge_id[e].get()<<"\" origin=\"" << src_id << "\" destination=\"" << dst_id << "\" />\n";
			}
		}

		ContextTree *ctree = CONTEXT_TREE(cfg);
		test2(cfg, ctree, loop_id, node_id, edge_id);

	// 	// generate blocks
	// 	for(CFG::BlockIter v(cfg->blocks()); v(); v++)
	// 		if(v->isBasic()) {
	// 			BasicBlock *b = **v;
	// 			cout << (b->index() + off - 1) << ' ' << b->address() << ' ' << b->last()->address();

	// 			// look leaving edges
	// 			for(Block::EdgeIter e = b->outs(); e(); e++) {
	// 				Block *w = e->sink();
	// 				if(w->isBasic())
	// 					cout << ' ' << (w->index() + off - 1);
	// 				else if(w->isSynth())
	// 					cout << ' ' << offset(w->toSynth()->callee());
	// 			}

	// 			cout << " -1\n";
	// 		}
		cout << "\t</CFG>\n";
	}

	cout << "</PROGRAM>\n";
}

/**
 * Get offset (in basic block count) of the CFG.
 * @param cfg	CFG to get offset for.
 * @return		CFG offset.
 */
int HeptaneDisplayer::offset(CFG *cfg) {
	int off = OFFSET(cfg);
	if(off < 0) {

		// count real BB
		int cnt = 0;
		for(CFG::BlockIter v(cfg->blocks()); v(); v++)
			if(v->isBasic())
				cnt++;

		// compute offset
		off = cfg_cnt;
		OFFSET(cfg) = off;
		cfg_cnt += cnt;
	}
	return off;
}

