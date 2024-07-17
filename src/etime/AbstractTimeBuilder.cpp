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
 * @fn ParExeGraph *Factory::make(ParExeProc *proc,  Vector<Resource *> *hw_resources, ParExeSequence *seq);
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

/**
 * @class StandardFactory;
 * Provides default implementation of XG factory.
 * @ingroup etime
 */

///
ParExeGraph *StandardFactory::make(ParExeProc *proc,  Vector<Resource *> *resources, ParExeSequence *seq) {
	PropList props;
	return new ParExeGraph(nullptr, proc, resources, seq);
}

///
ParExeNode *StandardFactory::makeNode(ParExeGraph *g, ParExeInst *i, ParExeStage *stage) {
	return new ParExeNode(g, stage, i);
}

///
ParExeEdge *StandardFactory::makeEdge(ParExeNode *src, ParExeNode *snk, ParExeEdge::edge_type_t_t type, int latency, string name) {
	return new ParExeEdge(src, snk, type, latency, name);
}


/**
 * Default implementation of a factory.
 */
Factory& Factory::def = elm::single<StandardFactory>();


/**
 * @class AbstractTimeBuilder
 * This is the base class of @ref etime classes producing time for block.
 * It is made of:
 * * @ref Builder object in charge of building the execution graph,
 * * @ref Engine object in charge of computing the execution considering all event configurations,
 * * @ref Generator object in charge of generating objective function and ILP constraints to
 * taken into account the different configurations and times.
 * @ingroup etime
 */



/** */
p::declare AbstractTimeBuilder::reg = p::init("otawa::etime::AbstractTimeBuilder", Version(1, 0, 0))
	.extend<Processor>()
	.maker<AbstractTimeBuilder>()
	.require(ipet::ASSIGNED_VARS_FEATURE)
	.require(ipet::ILP_SYSTEM_FEATURE)
	.require(hard::PROCESSOR_FEATURE)
	.require(COLLECTED_CFG_FEATURE)
	.provide(ipet::OBJECT_FUNCTION_FEATURE)
	.provide(EDGE_TIME_FEATURE);



/**
 */
AbstractTimeBuilder::AbstractTimeBuilder(p::declare& r)
:	Processor(r),
	_builder(nullptr),
	_solver(nullptr),
	_generator(nullptr),
	_proc(nullptr),
	_explicit(false),
	_predump(false),
	_event_th(0),
	_record(false),
	_sys(nullptr),
	_only_start(false)
{
}


/**
 */
void AbstractTimeBuilder::processWorkSpace(WorkSpace *ws) {
	_generator->process(ws);
}


/**
 */
void AbstractTimeBuilder::configure(const PropList& props) {
	Processor::configure(props);

	// configure the analysis
	_explicit = ipet::EXPLICIT(props);
	_predump = PREDUMP(props);
	_event_th = EVENT_THRESHOLD(props);
	_record = Processor::COLLECT_STATS(props);
	_dir = GRAPHS_OUTPUT_DIRECTORY(props);
	if(_dir != sys::Path(""))
		_explicit = true;
	_only_start = ONLY_START(props);

	// select components
	if(_builder == nullptr)
		setBuilder(XGraphBuilder::make(*this));
	if(_solver == nullptr)
		setSolver(XGraphSolver::make(*this));
	if(_generator == nullptr)
		setGenerator(ILPGenerator::make(*this));

	// configure components
	_builder->configure(props);
	_solver->configure(props);
	_generator->configure(props);
}


/**
 * Set the graoph builder to use for the time computation.
 * @param builder	Builder to use.
 */
void AbstractTimeBuilder::setBuilder(XGraphBuilder *builder) {
	ASSERT(_builder == nullptr || builder->_atb == nullptr);
	if(_builder != nullptr)
		_builder->_atb = nullptr;
	_builder = builder;
	_builder->_atb = this;
}

/**
 * Set the graph solver to use for the time computation.
 * @param solver	Solver to use.
 */
void AbstractTimeBuilder::setSolver(XGraphSolver *solver) {
	ASSERT(solver == nullptr || solver->_atb == nullptr);
	if(_solver != nullptr)
		_solver->_atb = nullptr;
	_solver = solver;
	_solver->_atb = this;
}

void AbstractTimeBuilder::setGenerator(ILPGenerator *generator) {
	ASSERT(_generator == nullptr || _generator->_atb == nullptr);
	if(_generator != nullptr)
		_generator->_atb = nullptr;
	_generator = generator;
	_generator->_atb = this;
}


/**
 *
 */
void AbstractTimeBuilder::setup(WorkSpace *ws) {
	const hard::Processor *proc = hard::PROCESSOR_FEATURE.get(ws);
	if(proc == &hard::Processor::null)
		throw ProcessorException(*this, "no processor to work with");
	_proc = new ParExeProc(proc);
	buildResources();
	_sys = ipet::SYSTEM(ws);
}


/**
 *
 */
void AbstractTimeBuilder::cleanup(WorkSpace *ws) {

	// cleanup
	delete _builder;
	delete _solver;
	delete _proc;
	for(auto res = resources_t::Iter(_resources); res(); res++)
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
    if(_only_start)
    	return;

    // build resource for stages and FUs
    for(ParExePipeline::StageIterator stage(_proc->pipeline()); stage(); stage++) {

    	// all except execute stage
    	if(stage->category() != ParExeStage::EXECUTE) {
			for(int i = 0; i < stage->width(); i++) {
				StringBuffer buffer;
				buffer << stage->name() << "[" << i << "]";
				StageResource * new_resource = new StageResource(buffer.toString(), *stage, i, resource_index++);
				_resources.add(new_resource);
			}
		}

    	// execute stage
		else {
			if(stage->orderPolicy() != ParExeStage::IN_ORDER)
				throw ProcessorException(*this, "out-of-order execution unsupported");
			if(stage->numFus() != 0)
				for(int i = 0; i<stage->numFus(); i++) {
					ParExePipeline * fu = stage->fu(i);
					ParExeStage *fu_stage = fu->firstStage();
					for (int j=0 ; j<fu_stage->width() ; j++) {
						StringBuffer buffer;
						buffer << fu_stage->name() << "[" << j << "]";
						StageResource * new_resource = new StageResource(buffer.toString(), fu_stage, j, resource_index++);
						_resources.add(new_resource);
					}
				}
			else
				for(int i = 0; i < stage->width(); i++) {
					auto r = new StageResource(
						_ << stage->name() << "[" << i << "]",
						*stage, i, resource_index++);
					_resources.add(r);
				}
		}
    }

    // build resources for queues
    for(ParExeProc::QueueIterator queue(_proc) ; queue() ; queue++) {
		int num = queue->size();
		for(int i = 0; i < num; i++) {
			StringBuffer buffer;
			buffer << queue->name() << "[" << i << "]";
			StageResource * upper_bound = nullptr;
			for(auto resource: _resources) {
				if(resource->type() == Resource::STAGE) {
					StageResource *sresource = static_cast<StageResource *>(resource);
					if(sresource->stage() == queue->emptyingStage()) {
                        if(i <= queue->size() - sresource->stage()->width() - 1) {
							if(sresource->slot() == sresource->stage()->width() - 1)
								upper_bound = sresource;
						}
						else {
							if(sresource->slot() == i - queue->size() + sresource->stage()->width())
                            {
								upper_bound = sresource;
                            }
						}
					}
				}
			}
			ASSERT(upper_bound);

			// build the queue resource
			QueueResource * new_resource = new QueueResource(buffer.toString(), *queue, i, resource_index++, upper_bound, _proc->pipeline()->numStages());
			_resources.add(new_resource);
		}
    }
}


} }	// otawa::etime

