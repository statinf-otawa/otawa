/*
 *	AbstractTimeBuilder class implementation
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

#include <elm/data/quicksort.h>
#include <otawa/etime/AbstractTimeBuilder.h>
#include <otawa/ipet/features.h>

namespace otawa {

extern Identifier<String> GRAPHS_OUTPUT_DIRECTORY;


namespace etime {

io::Output& operator<<(io::Output& out, part_t p) {
	static cstring label[IN_SIZE] = {
			"in-prefix",
			"in-block"
	};
	out << label[p];
	return out;
}


/**
 * @class EventCase
 * TODO
 *
 * @ingroup etime
 */

/**
 * EventCase::EventCase(void);
 * TODO
 */

/**
 * EventCase::EventCase(Event *e, part_t p);
 * TODO
 */

/**
 * Event *EventCase::event(void) const;
 * TODO
 */

/**
 * @fn part_t EventCase::part(void) const;
 * TODO
 */

/**
 * @fn int EventCase::index(void) const;
 * TODO
 */

/**
 * @fn void EventCase::setIndex(int i);
 * TODO
 */

/**
 */
io::Output& operator<<(io::Output& out, const EventCase& e) {
	out << e->name() << " at " << e.part() << "/" << e.event()->inst()->address() << " (" << e.event()->occurrence() << ", "<< e.event()->detail() << ")";
	return out;
}


/**
 * @class Factory
 * TODO
 * @ingroup etime
 */

/**
 */
Factory::~Factory(void) {
}

/**
 * @fn ParExeGraph *Factory::make(ParExeProc *proc,  elm::genstruct::Vector<Resource *> *hw_resources, ParExeSequence *seq);
 * TODO
 */

/**
 * @fn ParExeNode *Factory::makeNode(ParExeGraph *g, ParExeInst *i, ParExeStage *stage);
 * TODO
 */

/**
 * @fn ParExeEdge *Factory::makeEdge(ParExeNode *src, ParExeNode *snk, ParExeEdge::edge_type_t_t type = ParExeEdge::SOLID, int latency = 0, string name = "");
 * TODO
 */

/* internal */
class StandardFactory: public Factory {
public:

	ParExeGraph *make(ParExeProc *proc,  elm::genstruct::Vector<Resource *> *resources, ParExeSequence *seq) override {
		PropList props;
		return new ParExeGraph(nullptr, proc, resources, seq);
	}

	ParExeNode *makeNode(ParExeGraph *g, ParExeInst *i, ParExeStage *stage) override {
		return new ParExeNode(g, stage, i);
	}

	ParExeEdge *makeEdge(ParExeNode *src, ParExeNode *snk, ParExeEdge::edge_type_t_t type, int latency, string name) override {
		return new ParExeEdge(src, snk, type, latency, name);
	}

};


/**
 * TODO
 */
Factory *Factory::make(void) {
	return new StandardFactory;
}


/**
 * @class AbstractTimeBuilder
 * This is the base class of @ref etime classes producing time for block.
 * It is made of:
 * * @ref Builder object in charge of building the execution graph,
 * * @ref Engine object in charge of computing the execution considering all event configurations,
 * * @ref Generator object in charge of generating objective function and ILP constraints to
 * taken into account the different configurations and times.
 */



/** */
p::declare AbstractTimeBuilder::reg = p::init("otawa::etime::AbstractTimeBuilder", Version(1, 0, 0))
	.extend<BBProcessor>()
	.maker<AbstractTimeBuilder>()
	.require(ipet::ASSIGNED_VARS_FEATURE)
	.require(ipet::ILP_SYSTEM_FEATURE)
	.require(EVENTS_FEATURE)
	.require(hard::PROCESSOR_FEATURE)
	.provide(ipet::OBJECT_FUNCTION_FEATURE)
	.provide(EDGE_TIME_FEATURE);


/**
 */
AbstractTimeBuilder::AbstractTimeBuilder(p::declare& r)
:	BBProcessor(r),
	_builder(nullptr),
	_solver(nullptr),
	_generator(nullptr),
	_proc(nullptr),
	_explicit(false),
	_predump(false),
	_event_th(0),
	_record(false)
{
}

/**
 */
void AbstractTimeBuilder::configure(const PropList& props) {
	BBProcessor::configure(props);
	_explicit = ipet::EXPLICIT(props);
	_predump = PREDUMP(props);
	_event_th = EVENT_THRESHOLD(props);
	_record = RECORD_TIME(props);
	_dir = GRAPHS_OUTPUT_DIRECTORY(props);
	if(_dir != sys::Path(""))
		_explicit = true;
}


/**
 * @fn Engine *AbstractTimeBuilder::engine(void) const;
 * TODO
 */

/**
 * @fn Generator *AbstractTimeBuilder::generator(void) const;
 * TODO
 */

/**
 * @fn void AbstractTimeBuilder::setEngine(Engine *engine);
 * TODO
 */

/**
 * @fn Builder *AbstractTimeBuilder::builder(void) const;
 * TODO
 */

/**
 * @fn void AbstractTimeBuilder::setBuilder(Builder *builder);
 * TODO
 */

/**
 *
 */
void AbstractTimeBuilder::setup(WorkSpace *ws) {

	// build the microprocessor
	const hard::Processor *proc = hard::PROCESSOR(ws);
	if(proc == &hard::Processor::null)
		throw ProcessorException(*this, "no processor to work with");
	_proc = new ParExeProc(proc);
	buildResources();

	// if required, add default actions
	if(_builder == nullptr)
		_builder = XGraphBuilder::make(*this);
	if(_solver == nullptr)
		_solver = XGraphSolver::make(*this);
	if(_generator == nullptr)
		_generator = ILPGenerator::make(*this);

	// initialize the workers
	_builder->setFactory(_solver->getFactory());
	_builder->setProcessor(_proc);
	_builder->setResources(&_resources);
	_builder->setWorkSpace(ws);
	_builder->setExplicit(_explicit);
	_solver->setDumpDir(_dir);
	_generator->setSystem(ipet::SYSTEM(ws));
	_generator->setWorkspace(ws);
	_generator->setExplicit(_explicit);
}


/**
 *
 */
void AbstractTimeBuilder::cleanup(WorkSpace *ws) {

	// finalize generator
	_generator->complete();

	// cleanup
	delete _builder;
	delete _solver;
	delete _proc;
	for(auto res = resources_t::Iterator(_resources); res; res++)
		delete *res;
	_resources.clear();
}



/**
 * Build the resources required by processor model.
 */
void AbstractTimeBuilder::buildResources(void) {
    int resource_index = 0;
    //bool is_ooo_proc = false;

    // build the start resource
    StartResource * new_resource = new StartResource("start", resource_index++);
    _resources.add(new_resource);

    // build resource for stages and FUs
    for(ParExePipeline::StageIterator stage(_proc->pipeline()); stage; stage++) {

    	// all except execute stage
    	if(stage->category() != ParExeStage::EXECUTE) {
			for(int i = 0; i<stage->width(); i++) {
				StringBuffer buffer;
				buffer << stage->name() << "[" << i << "]";
				StageResource * new_resource = new StageResource(buffer.toString(), stage, i, resource_index++);
				_resources.add(new_resource);
			}
		}

    	// execute stage
		else {
			if(stage->orderPolicy() == ParExeStage::IN_ORDER) {
				for (int i=0 ; i<stage->numFus() ; i++) {
					ParExePipeline * fu = stage->fu(i);
					ParExeStage *fu_stage = fu->firstStage();
					for (int j=0 ; j<fu_stage->width() ; j++) {
						StringBuffer buffer;
						buffer << fu_stage->name() << "[" << j << "]";
						StageResource * new_resource = new StageResource(buffer.toString(), fu_stage, j, resource_index++);
						_resources.add(new_resource);
					}
				}
			}
			/*else
				is_ooo_proc = true;*/
		}
    }

    // build resources for queues
    for(ParExeProc::QueueIterator queue(_proc) ; queue ; queue++) {
		int num = queue->size();
		for (int i=0 ; i<num ; i++) {
			StringBuffer buffer;
			buffer << queue->name() << "[" << i << "]";
			StageResource * upper_bound;
			for (elm::genstruct::Vector<Resource *>::Iterator resource(_resources) ; resource ; resource++) {
				if (resource->type() == Resource::STAGE) {
					if (((StageResource *)(*resource))->stage() == queue->emptyingStage()) {
						if (i < queue->size() - ((StageResource *)(*resource))->stage()->width() - 1) {
							if (((StageResource *)(*resource))->slot() == ((StageResource *)(*resource))->stage()->width()-1) {
								upper_bound = (StageResource *) (*resource);
							}
						}
						else {
							if (((StageResource *)(*resource))->slot() == i - queue->size() + ((StageResource *)(*resource))->stage()->width()) {
								upper_bound = (StageResource *) (*resource);
							}
						}
					}
				}
			}
			ASSERT(upper_bound);
			// build the queue resource
			QueueResource * new_resource = new QueueResource(buffer.toString(), queue, i, resource_index++, upper_bound, _proc->pipeline()->numStages());
			_resources.add(new_resource);
		}
    }
}


/**
 *
 */
void AbstractTimeBuilder::processBB(WorkSpace *ws, CFG *cfg, Block *b) {

	// get basic block
	if(!b->isBasic())
		return;
	BasicBlock *bb = b->toBasic();

	// use each basic edge
	bool one = false;
	for(BasicBlock::BasicIns e(bb); e; e++) {
		processEdge((*e).source(), (*e).edge(), (*e).sink());
		one = true;
	}

	// first block: no predecessor
	if(!one)
		processEdge(0, 0, bb);

}


/**
 * TODO
 */
void AbstractTimeBuilder::prepareEvents(Vector<EventCase>& events) {

	// sort events
	class EventCaseComparator {
	public:
		static inline int compare(const EventCase& c1, const EventCase& c2) {
			if(c1.part() != c2.part())
				return c1.part() - c2.part();
			else
				return c1.event()->inst()->address().compare(c2.event()->inst()->address());
		}
	};
	elm::quicksort(events, EventCaseComparator());

	// index the dynamic events
	int di = 0;
	for(int i = 0; i < events.length(); i++)
		if(events[i].event()->occurrence() == etime::SOMETIMES) {
			events[i].setIndex(di);
			di++;
		}
}


/**
 * Compute and record time for the given edge.
 * @param e		Edge to compute for.
 */
void AbstractTimeBuilder::processEdge(BasicBlock *src, Edge *e, BasicBlock *snk) {

	if(logFor(Processor::LOG_BLOCK)) {
		log << "\t\t\t";
		if(src)
			log << src << ", ";
		if(e)
			log << e << ", ";
		log << snk << io::endl;
	}

	// collect events
	Vector<EventCase> all_events;
	if(src != nullptr)
		collectEvents(all_events, src, IN_PREFIX, EVENT);
	if(e != nullptr) {
		collectEvents(all_events, e, IN_PREFIX, PREFIX_EVENT);
		collectEvents(all_events, e, IN_BLOCK, EVENT);
	}
	collectEvents(all_events, snk, IN_BLOCK, EVENT);
	prepareEvents(all_events);

	// usual simple case: few events
	if(countDynEvents(all_events) <= _event_th) {
		ParExeSequence *seq = new ParExeSequence();
		int index = 0;

		// fill the prefix
		if(src)
			for(BasicBlock::InstIter inst = src->insts(); inst; inst++) {
				ParExeInst * par_exe_inst = new ParExeInst(inst, src, PROLOGUE, index++);
				seq->addLast(par_exe_inst);
			}

		// fill the current block
		for(BasicBlock::InstIter inst = snk->insts(); inst; inst++) {
			ParExeInst * par_exe_inst = new ParExeInst(inst, snk, otawa::BLOCK, index++);
			seq->addLast(par_exe_inst);
		}

		// perform the computation
		processSequence(e, seq, all_events);
		delete seq;
		return;
	}

	// remove prefix case
	if(logFor(LOG_BLOCK))
		log << "\t\t\ttoo many dynamic events (" << countDynEvents(all_events) << "). Discarding prefix: overestimation increase\n";

	// recollect events of block only
	all_events.clear();
	collectEvents(all_events, snk, IN_BLOCK, EVENT);
	prepareEvents(all_events);

	// check again
	if(countDynEvents(all_events) <= _event_th) {
		ParExeSequence *seq = new ParExeSequence();

		// fill the current block
		int index = 0;
		for(BasicBlock::InstIter inst(snk); inst; inst++) {
			ParExeInst * par_exe_inst = new ParExeInst(inst, snk, otawa::BLOCK, index++);
			seq->addLast(par_exe_inst);
		}

		// perform the computation
		processSequence(e, seq, all_events);
		delete seq;
		return;
	}

	// for now, just crash
	else {
		if(logFor(LOG_BLOCK)) {
			log << "\t\t\ttoo many dynamic events: " << countDynEvents(all_events) << ". Giving up. Sorry.\n";
			// log used events
			for(auto e = *all_events; e; e++)
				if((*e).event()->occurrence() == SOMETIMES)
					log << "\t\t\t\t" << (*e).part() << ": " << (*e).event()->inst()->address() << " -> " << (*e).event()->name() << " (" << (*e).event()->detail() << ") " << io::endl;
		}
		throw ProcessorException(*this, _ << "too many events (" << countDynEvents(all_events) << ") in " << src << "!");
	}
}


/**
 * TODO
 */
void AbstractTimeBuilder::collectEvents(Vector<EventCase>& events, PropList *props, part_t part, p::id<Event *>& id) {
	for(Identifier<Event *>::Getter event(props, id); event; event++) {
		events.add(EventCase(event, part));
		//cerr << "DEBUG: adding evnt at " << event->inst()->address() << io::endl;
	}
}


/**
 * Count the number of variable events in the event list.
 * @param events	Event list to process.
 * @return			Number of variable events.
 */
int AbstractTimeBuilder::countDynEvents(const Vector<EventCase>& events) {
	int cnt = 0;
	for(int i = 0; i < events.length(); i++)
		if(events[i].event()->occurrence() == SOMETIMES)
			cnt++;
	return cnt;
}


/**
 * TODO
 */
void AbstractTimeBuilder::processSequence(Edge *e, ParExeSequence *seq, Vector<EventCase>& events) {

	// log used events
	if(logFor(LOG_BB))
		for(auto e = *events; e; e++)
			/*log << "\t\t\t\t" << (*e).part() << ": " << (*e).event()->inst()->address()
				<< " -> " << (*e).event()->name() << " (" << (*e).event()->detail() << ") "
				<< io::endl;*/
			log << "\t\t\t\t" << *e << io::endl;

	// numbers the dynamic events
	int cnt = 0;
	for(int i = 0; i < events.count(); i++)
		if(events[i].event()->occurrence() == SOMETIMES)
			events[i].setIndex(cnt++);

	// build the graph
	ParExeGraph *g = _builder->build(seq);
	ASSERTP(g->firstNode(), "no first node found: empty execution graph");
	g->setExplicit(_explicit);

	// compute the times
	List<ConfigSet *> times;
	_solver->compute(g, times, events);
	if(logFor(LOG_BB))
		displayTimes(times, events);

	// apply the times to the ILP
	_generator->add(e, times, events);

	// clean all
	delete g;
	ASSERT(_proc->pipeline()->stages()->numNodes() == 0);
	for(auto t = *times; t; t++)
		delete t;
}


/**
 * Display the list of configuration sorted by cost.
 * @param times		List of times to display.
 * @param events	Events producing the times.
 */
void AbstractTimeBuilder::displayTimes(const List<ConfigSet *>& times, const Vector<EventCase>& events) {
	if(times) {
		int i = 0;
		for(auto t = *times; t; t++, i++) {
			log << "\t\t\t\t[" << i << "] cost = " << t->time() << " -> ";
			for(ConfigSet::Iter conf(**t); conf; conf++)
				log << " " << (*conf).toString(events.length());
			log << io::endl;
		}
	}
}

} }	// otawa::etime

