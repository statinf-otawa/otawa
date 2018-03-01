/*
 *	StandardEngine class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2017, IRIT UPS.
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

#include <elm/sys/System.h>
#include <otawa/etime/AbstractTimeBuilder.h>

namespace otawa { namespace etime {

/**
 * @class XGraphSolver
 * TODO
 * @ingroup etime
 */

/**
 */
XGraphSolver::XGraphSolver(const Monitor& mon): Monitor(mon) {

}

/**
 */
XGraphSolver::~XGraphSolver(void) {
}

/**
 * Get the factory build an execution graph adapted to the engine.
 * Default implementation returns Factory::DEFAULT.
 * @return	Engine factory for execution graph.
 */
Factory *XGraphSolver::getFactory(void) {
	return Factory::make();
}

/**
 * @fn void XGraphSolver::compute(ParExeGraph *g, List<ConfigSet *> times, const Vector<EventCase>& events);
 * TODO
 */

/**
 * @fn sys::Path XGraphSolver::dumpDir(void) const;
 * TODO
 */

/**
 * @fn void XGraphSolver::setDumpDir(sys::Path dir);
 * TODO
 */


/**
 * TODO
 */
class StandardXGraphSolver: public XGraphSolver {
public:
	typedef t::uint32 mask_t;

	StandardXGraphSolver(const Monitor& mon): XGraphSolver(mon), bedge(nullptr) {
	}

	/**
	 * Convert ExeGraph position to etime part.
	 * @param i		Instruction to get part from.
	 * @return		Corresponding etime part.
	 */
	part_t partOf(ParExeInst *i) {
		if(i->codePart() == PROLOGUE)
			return IN_PREFIX;
		else
			return IN_BLOCK;
	}

	/**
	 */
	void compute(ParExeGraph *g, List<ConfigSet *>& times, const Vector<EventCase>& all_events) {
		Vector<EventCase> events;
		Vector<EventCase> always_events;

		// applying static events (always, never)
		Vector<ParExeInst *> insts;
		ParExeSequence::InstIterator inst(g->getSequence());
		for(auto evt = *all_events; evt; evt++) {

			// find the instruction
			while(!(inst->inst() == (*evt).event()->inst() && partOf(inst) == (*evt).part())) {
				inst++;
				ASSERTP(inst, "no instruction for event " << *evt);
			}

			// apply the event
			switch(evt->occurrence()) {
			case NEVER:			continue;
			case SOMETIMES:		events.add(evt); insts.add(*inst); break;
			case ALWAYS:		apply((*evt).event(), inst, g); always_events.add(evt); break;
			default:			ASSERT(0); break;
			}
		}

		if(logFor(LOG_BB)) {
			log << "\t\t\t\tdynamic events = " << events.count() << io::endl;
			for(int i = 0; i < events.count(); i++)
				log << "\t\t\t\t(" << char(i + 'a') << ") " << events[i]->inst()->address() << "\t" << events[i]->name()
					<< " " << events[i].part() << io::endl;
		}

		// simple trivial case
		if(events.isEmpty()) {
			ot::time cost = g->analyze();
			ConfigSet *set = new ConfigSet(cost);
			set->add(Config());
			return;
		}

		// compute all cases
		t::uint32 prev = 0;
		Vector<ConfigSet> confs;
		for(t::uint32 event_mask = 0; event_mask < t::uint32(1 << events.count()); event_mask++) {

			// adjust the graph
			for(int i = 0; i < events.count(); i++) {
				if((prev & (1 << i)) != (event_mask & (1 << i))) {
					if(event_mask & (1 << i))
						apply(events[i].event(), insts[i], g);
					else
						rollback(events[i].event(), insts[i], g);
				}
			}
			prev = event_mask;

			// predump implementation
			// TODO
			/*if(_do_output_graphs && predump)
				outputGraph(graph, 666, 666, 666, _ << source << " -> " << target);*/

			// compute and store the new value
			ot::time cost = g->analyze();

			// dump it if needed
			if(!dumpDir().isEmpty()) {

				// gather information
				Block *v = g->lastNode()->inst()->basicBlock();
				Block *w = g->firstNode()->inst()->basicBlock();
				bool alone = partOf(g->firstNode()->inst()) == IN_BLOCK;

				// build the name
				StringBuffer buffer;
				buffer	<< dumpDir() << "/"
						<< v->cfg()->name()
						<< "-bb" << v->index()
						<< "-ctxt" << w->index()
						<< "-case" << prev
						<< ".dot";

				// build the information
				StringBuffer info;
				info << "CFG " << v->cfg() << "|";
				if(!alone) {
					info << w;
					if(w->cfg() != v->cfg())
						info << " (" << w->cfg() << ")";
					info << " -> ";
				}
				info << v;

				// dump to the file
				io::OutStream *stream = sys::System::createFile(buffer.toString());
				io::Output out(*stream);
				g->dump(out, info.toString(), dumpEvents(all_events, prev));
				delete stream;
			}

			// insert the time
			if(times.isEmpty()) {
				ConfigSet *set = new ConfigSet(cost);
				set->add(Config(event_mask));
				times.add(set);
			}
			else {
				bool done = false;
				List<ConfigSet *>::Iter prev;
				for(auto cur = *times; cur; prev = cur, cur++) {
					if(cur->time() == cost) {
						cur->add(Config(event_mask));
						done = true;
						break;
					}
					else if(cur->time() > cost)
						break;
				}
				if(!done) {
					ConfigSet *set = new ConfigSet(cost);
					set->add(Config(event_mask));
					times.addAfter(prev, set);
				}
			}
		}
	}

	/**
	 * Apply the given event to the given instruction.
	 * @param event		Event to apply.
	 * @param inst		Instruction to apply to.
	 * @param seq		Sequence.
	 */
	void apply(Event *event, ParExeInst *inst, ParExeGraph *g) {
		static string pred_msg = "pred";

		switch(event->kind()) {

		case FETCH: {
			switch (event->type()) {
			case LOCAL:
				inst->fetchNode()->setLatency(inst->fetchNode()->latency() + event->cost());
				break;

			case AFTER:
			case NOT_BEFORE: {
				// Edge type depends on the event type
				ParExeEdge::edge_type_t_t edge_type = event->type() == AFTER ?
						ParExeEdge::SOLID : ParExeEdge::SLASHED;

				// Find the related ParExeInst
				ParExeInst *rel_inst = 0;
				for(ParExeSequence::Iterator inst_it(*g->getSequence()); inst_it; ++inst_it)
					if(inst_it->inst() == event->related().fst) {
						rel_inst = *inst_it;
						break;
					}
				ASSERTP(rel_inst, "related instruction (" << event->related().fst->address()
						<< ") not found in the sequence");

				// Find the related ParExeNode
				bool found = false;
				for(ParExeInst::NodeIterator rel_node(rel_inst); rel_node; rel_node++)
					if(rel_node->stage()->unit() == event->related().snd) {
						found = true;

						// Add cost to the edge between the related node and the fetch code
						bool edge_found = false;
						for (ParExeGraph::Successor succ(*rel_node); succ; ++succ)
							if (*succ == inst->fetchNode() && succ.edge()->type() == edge_type) {
								succ.edge()->setLatency(succ.edge()->latency() + event->cost());
								edge_found = true;
								break;
							}
						if (!edge_found)
							throw otawa::Exception(_ << "edge from related stage "
									<< event->related().snd << " not found");

						break;
					}
				if(!found)
					throw otawa::Exception(_ << "related stage " << event->related().snd << " not found");
				break;
			}

			default:
				ASSERTP(0, _ << "unsupported event type " << event->type());
			}

			break;
		}

		case MEM: {
			bool found = false;
			for(ParExeInst::NodeIterator node(inst); node; node++)
				if(node->stage()->unit()->isMem()) {
					node->setLatency(node->latency() + event->cost() - 1);
					found = true;
					break;
				}

			if(!found && event->related().fst) {
				ParExeEdge::edge_type_t_t edge_type = event->type() == AFTER ? ParExeEdge::SOLID : ParExeEdge::SLASHED;
				ParExeInst *rel_inst = 0;
				for(ParExeSequence::Iterator inst_it(*g->getSequence()); inst_it; ++inst_it) {
					if(inst_it->inst() == event->related().fst) {
						rel_inst = *inst_it;
						break;
					}
				}
				// ASSERTP(rel_inst, "related instruction (" << event->related().fst->address() << ") not found in the sequence");

				for(ParExeInst::NodeIterator rel_node(rel_inst); rel_inst && rel_node; rel_node++)
					if(rel_node->stage()->unit() == event->related().snd) {
						found = true;

						// Add cost to the edge between the related node and the fetch code
						bool edge_found = false;
						for (ParExeGraph::Successor succ(*rel_node); succ; ++succ) {
							if (*succ == inst->execNode() && succ.edge()->type() == edge_type) {
								succ.edge()->setLatency(succ.edge()->latency() + event->cost());
								edge_found = true;
								break;
							}
						}
						ASSERTP(edge_found, "edge from related stage " << event->related().snd->getName() << " not found");
						break;
					}
			}

			if(!found && inst->execNode()) {
				inst->execNode()->setLatency(inst->execNode()->latency() + event->cost() - 1);
				found = true;
			}

			if(!found)
				throw otawa::Exception("no memory stage / FU found in this pipeline");
			break;
		}

		case BRANCH:
			// TODO fix to use factory
			bedge =  new ParExeEdge(getBranchNode(g), inst->fetchNode(), ParExeEdge::SOLID, 0, pred_msg);
			bedge->setLatency(event->cost());
			break;

		default:
			ASSERTP(0, _ << "unsupported event kind " << event->kind());
			break;
		}

	}

	/**
	 * Get the branch node resolving a branch prediction.
	 * @param seq	Sequence to look in.
	 * @return		Node of resolution.
	 */
	ParExeNode *getBranchNode(ParExeGraph *g) {
		for(ParExeSequence::Iterator pinst(*g->getSequence()); pinst && pinst->codePart() == PROLOGUE; pinst++)
			if(pinst->inst()->isControl()) {
				for(ParExeInst::NodeIterator node(*pinst); node; node++)
					if(node->stage()->unit()->isBranch())
						return node;
				ParExeStage *stage = g->getMicroprocessor()->execStage()->findFU(pinst->inst()->kind())->lastStage();
				for(ParExeInst::NodeIterator node(*pinst); node; node++)
					if(node->stage() == stage)
						return node;
			}
		return nullptr;
	}

	/**
	 * Rollback the given event to the given instruction.
	 * @param event		Event to apply.
	 * @param inst		Instruction to apply to.
	 */
	void rollback(Event *event, ParExeInst *inst, ParExeGraph *g) {

		switch(event->kind()) {

		case FETCH: {
			switch (event->type()) {
			case LOCAL:
				inst->fetchNode()->setLatency(inst->fetchNode()->latency() - event->cost());
				break;

			case AFTER:
			case NOT_BEFORE: {
				// Edge type depends on the event type
				ParExeEdge::edge_type_t_t edge_type = event->type() == AFTER ?
						ParExeEdge::SOLID : ParExeEdge::SLASHED;

				// Find the related ParExeInst
				ParExeInst *rel_inst = 0;
				for(ParExeSequence::Iterator inst_it(*g->getSequence()); inst_it; ++inst_it)
					if(inst_it->inst() == event->related().fst) {
						rel_inst = *inst_it;
						break;
					}
				ASSERTP(rel_inst, "related instruction (" << event->related().fst->address()
						<< ") not found in the sequence");

				// Find the related ParExeNode
				bool found = false;
				for(ParExeInst::NodeIterator rel_node(rel_inst); rel_node; rel_node++)
					if(rel_node->stage()->unit() == event->related().snd) {
						found = true;

						// Add cost to the edge between the related node and the fetch code
						bool edge_found = false;
						for (ParExeGraph::Successor succ(*rel_node); succ; ++succ)
							if (*succ == inst->fetchNode() && succ.edge()->type() == edge_type) {
								succ.edge()->setLatency(succ.edge()->latency() - event->cost());
								edge_found = true;
								break;
							}
						if (!edge_found)
							throw otawa::Exception(_ << "edge from related stage "
									<< event->related().snd << " not found");

						break;
					}
				if(!found)
					throw otawa::Exception(_ << "related stage " << event->related().snd << " not found");
				break;
			}

			default:
				ASSERTP(0, _ << "unsupported event type " << event->type());
			}
			break;
		}

		case MEM: {
			bool found = false;
			for(ParExeInst::NodeIterator node(inst); node; node++)
				if(node->stage()->unit()->isMem()) {
					node->setLatency(node->latency() - event->cost() + 1);
					found = true;
					break;
				}

			if(!found && event->related().fst) {
				ParExeEdge::edge_type_t_t edge_type = event->type() == AFTER ? ParExeEdge::SOLID : ParExeEdge::SLASHED;
				ParExeInst *rel_inst = 0;
				for(ParExeSequence::Iterator inst_it(*g->getSequence()); inst_it; ++inst_it) {
					if(inst_it->inst() == event->related().fst) {
						rel_inst = *inst_it;
						break;
					}
				}
				ASSERTP(rel_inst, "related instruction (" << event->related().fst->address() << ") not found in the sequence");

				for(ParExeInst::NodeIterator rel_node(rel_inst); rel_inst && rel_node; rel_node++)
					if(rel_node->stage()->unit() == event->related().snd) {
						found = true;

						// Add cost to the edge between the related node and the fetch code
						bool edge_found = false;
						for (ParExeGraph::Successor succ(*rel_node); succ; ++succ) {
							if (*succ == inst->execNode() && succ.edge()->type() == edge_type) {
								succ.edge()->setLatency(succ.edge()->latency() - event->cost());
								edge_found = true;
								break;
							}
						}
						ASSERTP(edge_found, "edge from related stage " << event->related().snd->getName() << " not found");
						break;
					}
			}

			if(!found && inst->execNode()) {
				inst->execNode()->setLatency(inst->execNode()->latency() - event->cost() + 1);
				found = true;
			}
			break;
		}

		case BRANCH:
			ASSERT(bedge);
			cerr << "DEBUG: rollbacking " << (void *)bedge << io::endl;
			g->remove(bedge);
			bedge = 0;
			break;

		default:
			ASSERTP(0, _ << "unsupported event kind " << event->kind());
			break;
		}

	}

	/**
	 * TODO
	 */
	string dumpEvents(const Vector<EventCase>& all_events, mask_t mask) {
		StringBuffer out;
		out << "events|";
		bool first = true;
		for(auto e = *all_events; e; e++) {

			// display separator
			if(first)
				first = false;
			else
				out << "\n";

			// define activation
			out << "[";
			if((*e).index() < 0) {
				if((*e).event()->occurrence() == ALWAYS)
					out << "A";
				else
					out << "N";
			}
			else {
				if((mask & (1 << (*e).index())) != 0)
					out << "1";
				else
					out << "0";
			}
			out << "] ";

			// display event
			out << (*e).event()->inst()->address() << " " << (*e).event()->detail();
		}
		return out.toString();
	}

	// TODO so ugly
	ParExeEdge *bedge;
};

/**
 * TODO
 */
XGraphSolver *XGraphSolver::make(const Monitor& mon) {
	return new StandardXGraphSolver(mon);
}

} }	// otawa::etime

