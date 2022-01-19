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

#include <elm/data/HashSet.h>
#include <elm/data/quicksort.h>
#include <elm/sys/System.h>
#include <otawa/etime/AbstractTimeBuilder.h>

namespace otawa { namespace etime {

/**
 * @class XGraphSolver
 *
 * The XGraphSolver is in charge to compute the execution times of a ParExeGraph
 * taking into account the possible events.
 *
 * Basically, the function XGraphSolver::solve() is called for each possible
 * ParExeGraph. After the resolution, the XGraphSolver has to provide the found
 * time and bounds on the occurrence of this time to the ILPGenerator. This is
 * done by internal functions, called contribute().
 *
 * XGraphSolver provides also convenient functions to enrich the ILP system with
 * the computed times:
 * * contributeBase() provides a base time for the processed code sequence,
 * * contributeTime() provides an alternative time (depending on events),
 * * contributePositive(), applied on a former call to contributeTime(), bounds its
 * occurrences with the activation of an event,
 * * contributeNegative(), applied on a former call to contributeTime(), bounds its
 * occurrences with the inactivation of an event.
 *
 * @ingroup etime
 */

/**
 */
XGraphSolver::XGraphSolver(Monitor& mon): Monitor(mon), _atb(nullptr) {

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
	return &Factory::def;
}

/**
 * @fn void XGraphSolver::compute(const PropList *entity, ParExeGraph *g, const Vector<EventCase>& events);
 * Compute the times of the given ParExeGraph taking into account the given events.
 * The obtained times are then dumped to the ILPGenerator using XGraphSolver::contribute()
 * functions.
 *
 * @param entity	Entity (block, edge, sequence, etc) for which the time is computed.
 * @param g			ParExeGraph to compute.
 * @param events	Events arising during the ParExeGraph execution.
 */

/**
 * @fn sys::Path XGraphSolver::dumpDir(void) const;
 * Get the directory used to dump the resolved execution graphs.
 * As the default, this value is empty meaning there is no dump.
 * @return	Execution graph dump directory.
 */

/**
 * @fn void XGraphSolver::setDumpDir(sys::Path dir);
 * Set the directory used to dump the execution graphs.
 * Pass an empty string to disable dumping.
 * @param dir	Directory used to dump execution graphs.
 */

/**
 * Called with the same configuration properties passed
 * to AbstractTimebuilder to let the builder configure
 * itself. Override this function to customize the
 * configuration.
 * @param props	Configuration properties.
 */
void XGraphSolver::configure(const PropList& props) {
}


/**
 * Simple implementation of an execution graph based on:
 * * topological traversal of the graph to extract maximum time for
 * instruction and resources,
 * * each event configuration is tested in turn,
 * * LTS/HTS scheme of time production.
 *
 * @ingroup etime
 */
class StandardXGraphSolver: public XGraphSolver {
public:
	typedef t::uint32 mask_t;

	StandardXGraphSolver(Monitor& mon): XGraphSolver(mon), bedge(nullptr), no_ilp(false) {
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
	void compute(const PropList *entity, ParExeGraph *g, const Vector<EventCase>& all_events) override {
		Vector<EventCase> events;
		Vector<EventCase> always_events;
		List<ConfigSet *> times;

		// applying static events (always, never)
		Vector<ParExeInst *> insts;
		ParExeSequence::InstIterator inst(g->getSequence());
		for(auto evt = *all_events; evt(); evt++) {

			// find the instruction
			while(!(inst->inst() == (*evt).event()->inst() && partOf(*inst) == (*evt).part())) {
				inst++;
				ASSERTP(inst, "no instruction for event " << *evt);
			}

			// apply the event
			switch(evt->occurrence()) {
			case NEVER:			continue;
			case SOMETIMES:		events.add(*evt); insts.add(*inst); break;
			case ALWAYS:		apply((*evt).event(), *inst, g); always_events.add(*evt); break;
			default:			ASSERT(0); break;
			}
		}

		if(logFor(LOG_BB)) {
			log << "\t\t\tdynamic events = " << events.count() << io::endl;
			for(int i = 0; i < events.count(); i++)
				log << "\t\t\t(" << char(i + 'a') << ") " << events[i]->inst()->address() << "\t" << events[i]->name()
					<< " " << events[i].part() << io::endl;
		}

		// simple trivial case
		if(events.isEmpty()) {
			ot::time cost = g->analyze();
			ConfigSet *set = new ConfigSet(cost);
			set->add(Config());
			if(logFor(LOG_BB))
				log << "\t\t\tcost = " << cost << io::endl;
			if(!no_ilp)
				contributeBase(cost);
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
			// re-enable at some later time
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
				for(auto cur = times.begin(); cur(); prev = cur, cur++) {
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
					if(!prev)
						times.addFirst(set);
					else
						times.addAfter(prev, set);
				}
			}
		}

		// process the times
		if(!no_ilp)
			split(entity, all_events, times);
	}

	/**
	 * Apply the latency to a node.
	 * @param node		Node to add latency to.
	 * @param latency	Latency to add.
	 */
	inline void addLatency(ParExeNode *node, int latency) {
		if(node->latency() == 1)
			node->setLatency(latency);
		else
			node->setLatency(node->latency() + latency);
	}

	/**
	 * Remove latency from a node.
	 * @param node		Node to remove latency from.
	 * @param latency	Latency to remove.
	 */
	inline void removeLatency(ParExeNode *node, int latency) {
		ASSERT(node->latency() >= latency);
		if(node->latency() == latency)
			node->setLatency(1);
		else
			node->setLatency(node->latency() - latency);
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
				addLatency(inst->fetchNode(), event->cost());
				break;

			case AFTER:
			case NOT_BEFORE: {
				// Edge type depends on the event type
				ParExeEdge::edge_type_t_t edge_type = event->type() == AFTER ?
						ParExeEdge::SOLID : ParExeEdge::SLASHED;

				// Find the related ParExeInst
				ParExeInst *rel_inst = 0;
				for(ParExeSequence::Iter inst_it(*g->getSequence()); inst_it(); ++inst_it)
					if(inst_it->inst() == event->related().fst) {
						rel_inst = *inst_it;
						break;
					}
				ASSERTP(rel_inst, "related instruction (" << event->related().fst->address()
						<< ") not found in the sequence");

				// Find the related ParExeNode
				bool found = false;
				for(ParExeInst::NodeIterator rel_node(rel_inst); rel_node(); rel_node++)
					if(rel_node->stage()->unit() == event->related().snd) {
						found = true;

						// Add cost to the edge between the related node and the fetch code
						bool edge_found = false;
						for (ParExeGraph::Successor succ(*rel_node); succ(); ++succ)
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
			for(ParExeInst::NodeIterator node(inst); node(); node++)
				if(node->stage()->unit()->isMem()) {
					addLatency(*node, event->cost());
					found = true;
					break;
				}

			if(!found && event->related().fst) {
				ParExeEdge::edge_type_t_t edge_type = event->type() == AFTER ? ParExeEdge::SOLID : ParExeEdge::SLASHED;
				ParExeInst *rel_inst = 0;
				for(ParExeSequence::Iter inst_it(*g->getSequence()); inst_it(); ++inst_it) {
					if(inst_it->inst() == event->related().fst) {
						rel_inst = *inst_it;
						break;
					}
				}
				// ASSERTP(rel_inst, "related instruction (" << event->related().fst->address() << ") not found in the sequence");

				for(ParExeInst::NodeIterator rel_node(rel_inst); rel_inst && rel_node(); rel_node++)
					if(rel_node->stage()->unit() == event->related().snd) {
						found = true;

						// Add cost to the edge between the related node and the fetch code
						IN_ASSERT(bool edge_found = false);
						for (ParExeGraph::Successor succ(*rel_node); succ(); ++succ) {
							if (*succ == inst->execNode() && succ.edge()->type() == edge_type) {
								succ.edge()->setLatency(succ.edge()->latency() + event->cost());
								IN_ASSERT(edge_found = true);
								break;
							}
						}
						ASSERTP(edge_found, "edge from related stage " << event->related().snd->getName() << " not found");
						break;
					}
			}

			if(!found && inst->execNode()) {
				addLatency(inst->execNode(), event->cost());
				found = true;
			}

			if(!found)
				throw otawa::Exception("no memory stage / FU found in this pipeline");
			break;
		}

		case BRANCH:
			bedge =  getFactory()->makeEdge(getBranchNode(g), inst->fetchNode(), ParExeEdge::SOLID, 0, pred_msg);
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
		for(ParExeSequence::Iter pinst(*g->getSequence()); pinst() && pinst->codePart() == PROLOGUE; pinst++)
			if(pinst->inst()->isControl()) {
				for(ParExeInst::NodeIterator node(*pinst); node(); node++)
					if(node->stage()->unit()->isBranch())
						return *node;
				ParExeStage *stage = g->getMicroprocessor()->execStage()->findFU(pinst->inst()->kind())->lastStage();
				for(ParExeInst::NodeIterator node(*pinst); node(); node++)
					if(node->stage() == stage)
						return *node;
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
				removeLatency(inst->fetchNode(), event->cost());
				break;

			case AFTER:
			case NOT_BEFORE: {
				// Edge type depends on the event type
				ParExeEdge::edge_type_t_t edge_type = event->type() == AFTER ?
						ParExeEdge::SOLID : ParExeEdge::SLASHED;

				// Find the related ParExeInst
				ParExeInst *rel_inst = 0;
				for(ParExeSequence::Iter inst_it(*g->getSequence()); inst_it(); ++inst_it)
					if(inst_it->inst() == event->related().fst) {
						rel_inst = *inst_it;
						break;
					}
				ASSERTP(rel_inst, "related instruction (" << event->related().fst->address()
						<< ") not found in the sequence");

				// Find the related ParExeNode
				bool found = false;
				for(ParExeInst::NodeIterator rel_node(rel_inst); rel_node(); rel_node++)
					if(rel_node->stage()->unit() == event->related().snd) {
						found = true;

						// Add cost to the edge between the related node and the fetch code
						bool edge_found = false;
						for (ParExeGraph::Successor succ(*rel_node); succ(); ++succ)
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
			for(ParExeInst::NodeIterator node(inst); node(); node++)
				if(node->stage()->unit()->isMem()) {
					removeLatency(*node, event->cost());
					found = true;
					break;
				}

			if(!found && event->related().fst) {
				ParExeEdge::edge_type_t_t edge_type = event->type() == AFTER ? ParExeEdge::SOLID : ParExeEdge::SLASHED;
				ParExeInst *rel_inst = 0;
				for(ParExeSequence::Iter inst_it(*g->getSequence()); inst_it(); ++inst_it) {
					if(inst_it->inst() == event->related().fst) {
						rel_inst = *inst_it;
						break;
					}
				}
				ASSERTP(rel_inst, "related instruction (" << event->related().fst->address() << ") not found in the sequence");

				for(ParExeInst::NodeIterator rel_node(rel_inst); rel_inst && rel_node(); rel_node++)
					if(rel_node->stage()->unit() == event->related().snd) {
						found = true;

						// Add cost to the edge between the related node and the fetch code
						IN_ASSERT(bool edge_found = false);
						for (ParExeGraph::Successor succ(*rel_node); succ(); ++succ) {
							if (*succ == inst->execNode() && succ.edge()->type() == edge_type) {
								succ.edge()->setLatency(succ.edge()->latency() - event->cost());
								IN_ASSERT(edge_found = true);
								break;
							}
						}
						ASSERTP(edge_found, "edge from related stage " << event->related().snd->getName() << " not found");
						break;
					}
			}

			if(!found && inst->execNode()) {
				removeLatency(inst->execNode(), event->cost());
				found = true;
			}
			break;
		}

		case BRANCH:
			ASSERT(bedge);
			g->remove(bedge);
			bedge = 0;
			break;

		default:
			ASSERTP(0, _ << "unsupported event kind " << event->kind());
			break;
		}

	}

	/**
	 * Display the list variable events according to the given event mask.
	 * @param all_events	All events.
	 * @param mask			Activation mask.
	 */
	string dumpEvents(const Vector<EventCase>& all_events, mask_t mask) {
		StringBuffer out;
		out << "events|";
		bool first = true;
		for(auto e = *all_events; e(); e++) {

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

	class Split {
	public:
		inline Split(int c):
			cnt(c),
			pos(0),
			neg(0),
			com(0),
			unu(0),
			lts_time(0),
			hts_time(0)
		{ }

		int cnt;
		mask_t pos;
		mask_t neg;
		mask_t com;
		mask_t unu;
		ot::time lts_time;
		ot::time hts_time;
	};

	/**
	 * Apply an algorithm of splits or the found time for the current sequence.
	 * The times are divided in 2 sets, Low Time Set (LTS) and High time Set (HTS)
	 * producing a LTS time and an HTS time.
	 *
	 * @param entity		Computed entity.
	 * @param all_events	Event of the sequence.
	 * @param times			Found times.
	 */
	void split(const PropList *entity, const Vector<EventCase>& all_events, const List<ConfigSet *>& times) {
		ASSERT(0 < times.count());
		displayTimes(times, all_events);

		// mono-time case
		if(times.count() == 1) {
			genForOneCost(times.first()->time(), all_events);
			return;
		}

		// count dynamic events
		int dyn_cnt = 0;
		for(auto e = *all_events; e(); e++)
			if((*e).event()->occurrence() == SOMETIMES)
				dyn_cnt++;
		ASSERTP(times.count() <= (1 << dyn_cnt), times.count() << " events");

		// put all configurations in a vector
		Vector<ConfigSet *> confs;
		for(auto conf = *times; conf(); conf++)
			confs.add(*conf);
		class ConfigSetCompare {
		public:
			int doCompare(const ConfigSet *cs1, const ConfigSet *cs2) const{
				if(cs1->time() == cs2->time())
					return 0;
				else if(cs1->time() < cs2->time())
					return -1;
				else
					return +1;
			}
		};
		quicksort(confs, ConfigSetCompare());

		// initialization
		int best_p = 0;
		ot::time best_cost = type_info<ot::time>::max;

		// set of configurations
		ConfigSet set;
		for(int i = confs.length() - 1; i >= 0; i--)
			set.push(*confs[i]);

		// computation
		for(int p = 1; p < confs.length(); p++) {

			// update set and values
			set.pop(*confs[p - 1]);

			// scan the set of values
			Split split(dyn_cnt);
			set.scan(split.pos, split.neg, split.unu, split.com, split.cnt);
			ot::time x_hts = 0;

			// x^c_hts = sum{e in E_i /\ (\E c in HTS /\ e in c) /\ (\E c in HTS /\ e not in c)} w_e
			for(auto ev = *all_events; ev(); ev++)
				if((*ev).index() >= 0 && (split.com & (1 << (*ev).index())) != 0)
					x_hts += (*ev).event()->weight();

			// x^p_hts = max{e in E_i /\ (\A c in HTS -> e in c)} w_e
			for(auto ev = *all_events; ev(); ev++)
				if((*ev).index() >= 0 && (split.pos & (1 << (*ev).index())) != 0)
					x_hts = max(x_hts, ot::time((*ev).event()->weight()));
			// x_hts = max(x^c_hts, x^p_hts)

			// cost = x_hts t_hts + (x_i - x_hts) t_lts
			int weight = WEIGHT(entity);
			if(x_hts > weight)
				x_hts = weight;
			ot::time cost = x_hts * confs.top()->time() + (weight - x_hts) * confs[p - 1]->time();
			if (isVerbose())
				log << "\t\t\tHTS [" << p << " - " << (confs.length() - 1) << "], cost = " << cost << " (" << x_hts << "/" << weight << ")\n";

			// look for best cost
			if(cost < best_cost) {
				best_p = p;
				best_cost = cost;
			}
		}
		if (logFor(LOG_BB))
			log << "\t\t\tbest HTS [" << best_p << " - " << (confs.length() - 1) << "]\n";

		// look in the split
		ConfigSet hts;
		Split split(dyn_cnt);
		makeSplit(confs, best_p, hts, split);
		hts.scan(split.pos, split.neg, split.unu, split.com, split.cnt);
		if(isVerbose())
			log << "\t\t\t"
				<<   "pos = " 		<< Config(split.pos).toString(split.cnt)
				<< ", neg = " 		<< Config(split.neg).toString(split.cnt)
				<< ", unused = " 	<< Config(split.unu).toString(split.cnt)
				<< ", complex = "	<< Config(split.com).toString(split.cnt)
				<< io::endl;

		// contribute
		contributeSplit(entity, all_events, split);
	}

	/**
	 * Generate the constraints when only one cost is considered for the edge.
	 * @param edge		Current edge.
	 * @param events	List of edge events.
	 */
	void genForOneCost(ot::time cost, const Vector<EventCase>&  events) {

		// logging
		if(logFor(LOG_BB))
			log << "\t\t\tcost = " << cost << io::endl;

		// record the time
		contributeBase(cost);

	}

	/**
	 * Build the set after split.
	 * @param confs		Current configuration.
	 * @param p			Split position.
	 * @param hts		HTS result set.
	 * @param lts_time	LTS time.
	 * @param hts_time	HTS time.
	 * @param cnt		Count of dynamic events.
	 */
	void makeSplit(const Vector<ConfigSet *>& confs, int p, ConfigSet& hts, Split& split) {
		split.lts_time = confs[p - 1]->time();
		split.hts_time = confs.top()->time();
		hts = ConfigSet(split.hts_time);
		for(int i = p; i < confs.length(); i++)
			hts.add(*confs[i]);
		if(logFor(LOG_BLOCK)) {
			log << "\t\t\t" << "LTS time = " << split.lts_time << ", HTS time = " << split.hts_time << " for { ";
			bool fst = true;
			for(ConfigSet::Iter conf(hts); conf(); conf++) {
				if(fst)
					fst = false;
				else
					log << ", ";
				log << (*conf).toString(split.cnt);
			}
			log << " }\n";
		}
	}

	/**
	 * Contribute to WCET estimation in split way, x_HTS and x_LTS,
	 * with two sets of times.
	 * @param e			Evaluated edge.
	 * @param events	Dynamic event list.
	 * @param split		Split result.
	 */
	void contributeSplit(const PropList *entity, const Vector<EventCase>& events, const Split& split) {
		Vector<EventCase> exact, imprec;

		// contribute LTS
		contributeBase(split.lts_time);

		// contribute HTS
		contributeTime(split.hts_time);

		// build effects of events
		for(auto ev = *events; ev(); ev++)
			if((*ev).event()->occurrence() == SOMETIMES) {

			// positive contribution
			if((split.pos & (1 << ev.index())) != 0)
				contributePositive(*ev, false);

			// else if e in neg_events then C^e_p += x_edge - x_hts / p = prefix if e in prefix, block
			else if((split.neg & (1 << ev.index())) != 0)
				contributeNegative(*ev, false);
		}

	}

	/**
	 * Display the list of configuration sorted by cost.
	 * @param times		List of times to display.
	 * @param events	Events producing the times.
	 */
	void displayTimes(const List<ConfigSet *>& times, const Vector<EventCase>& events) {
		if(times) {
			int i = 0;
			for(auto t: times) {
				log << "\t\t\t[" << i << "] cost = " << t->time() << " -> ";
				for(ConfigSet::Iter conf(*t); conf(); conf++)
					log << " " << (*conf).toString(events.length());
				log << io::endl;
				i++;
			}
		}
	}

	// TODO so ugly
	ParExeEdge *bedge;
	bool no_ilp;
};

/**
 * Record a base time for the current code sequence.
 *
 * @warning This function must be
 * called before contributeTime().
 *
 * @param time	Base time of the code sequence.
 */
void XGraphSolver::contributeBase(ot::time time) {
	_atb->generator()->contributeBase(time);
}

/**
 * Record a new non-base time for the current code sequence.
 * Calles to contributePositive() and contributeNegative() applies
 * to the occurrences of this time.
 *
 * @warning This function must be called after a call to contributeBase().
 *
 * @param time	New time to record.
 */
void XGraphSolver::contributeTime(ot::time time) {
	_atb->generator()->contributeTime(time);
}

/**
 * Record that the given event bounds the current code sequence
 * when it is activated.
 * @param event		Bounding event.
 * @param prec		If true, the bound is exact, else it is just an overestimation.
 */
void XGraphSolver::contributePositive(EventCase event, bool prec) {
	_atb->generator()->contributePositive(event, prec);
}

/**
 * Record that the given event bounds the current code sequence
 * when it is inactivated.
 * @param event		Bounding event.
 * @param prec		If true, the bound is exact, else it is just an overestimation.
 */
void XGraphSolver::contributeNegative(EventCase event, bool prec) {
	_atb->generator()->contributeNegative(event, prec);
}


/**
 * Build a default graph solver.
 * @param mon	Monitor to use.
 * @return		Default graph solver.
 */
XGraphSolver *XGraphSolver::make(Monitor& mon) {
	return new StandardXGraphSolver(mon);
}

/**
 * Convenient function to obtain, if defined, the dump directory
 * for outputting execution graph. If an empty path is returned,
 * no dumping is needed.
 * @return		Directory path to dump graph to or an empty path.
 *
 */
sys::Path XGraphSolver::dumpDir(void) const {
	return _atb->_dir;
}

} }	// otawa::etime

