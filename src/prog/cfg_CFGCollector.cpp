/*
 *	CFGCollector class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-08, IRIT UPS.
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

#include <elm/assert.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/cfg/features.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/prog/File.h>
#include <otawa/prog/Manager.h>
#include <otawa/prog/TextDecoder.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/view/features.h>
#include <otawa/flowfact/FlowFactLoader.h>

namespace otawa {

class CollectorCleaner: public Cleaner {
public:

	CollectorCleaner(WorkSpace *ws, CFGCollection *coll): _ws(ws), _coll(coll) { }

	virtual void clean(void) {
		for(CFGCollection::Iter g(_coll); g(); g++)
			delete *g;
		delete _coll;
		INVOLVED_CFGS(_ws).remove();
	}

private:
	WorkSpace *_ws;
	CFGCollection *_coll;
};


/**
 * @class CFGCollection <otawa/cfg/features.h>
 * Contains a collection of CFGs (used with @ref INVOLVED_CFGS property).
 * @see otawa::CFGCollector, otawa::INVOLVED_CFGS
 *
 * @ingroup cfg
 */

/**
 * @fn int CFGCollection::count(void) const;
 * Get the count of CFG in the collection.
 * @return CFG count.
 */

/**
 * @fn CFG *CFGCollection::get(int index) const;
 * Get a CFG from the collection using its index.
 * @param index	Index of the got CFG.
 * @return		CFG matching the index.
 */

/**
 * @fn CFG *CFGCollection::operator[](int index) const;
 * Shortcut to @ref get().
 */

/**
 * @fn CFG *CFGCollection::entry(void) const;
 * Get the entry CFG of the collection.
 * @return	Entry CFG.
 */

/**
 * @class CFGCollection::Iter
 * Iterator on the CFG contained in a @ref CFGCollection.
 */

/**
 * @fn CFGCollection::Iter::Iter(const CFGCollection *cfgs);
 * Build an iterator on the given CFG collection.
 * @param cfgs	CFG collection to iterate on.
 */

/**
 * @fn CFGCollection::Iter::Iter(const CFGCollection& cfgs);
 * Build an iterator on the given CFG collection.
 * @param cfgs	CFG collection to iterate on.
 */

/**
 * @fn CFGCollection::Iter::Iter(WorkSpace *ws);
 * CFG iterator builder on the CFG collection of a workspace.
 * @warning The COLLECTED_CFG_FEATURE must be provided!
 * @param ws	Workspace containing the CFG collection.
 */

/**
 * @class CFGCollection::BlockIter
 * Iterator on the total list of blocks composing CFGs of a CFG collection.
 */

/**
 * @fn CFGCollection::BlockIter::BlockIter(WorkSpace *ws);
 * Block iterator builder on the CFG collection of a workspace.
 * @warning The COLLECTED_CFG_FEATURE must be provided!
 * @param ws	Workspace containing the CFG collection.
 */

/**
 * Get the CFG collection from a workspace.
 * @param ws	Workspace to look in.
 * @return		Found CFG collection.
 * @warning The COLLECTED_CFG_FEATURE must be provided!
 */
const CFGCollection *CFGCollection::get(WorkSpace *ws) {
	const CFGCollection *coll = COLLECTED_CFG_FEATURE.get(ws);
	ASSERT(coll);
	return coll;
}

/**
 * Add a CFG to the collection.
 * @param cfg	Added CFG.
 */
void CFGCollection::add(CFG *cfg) {
	cfg->idx = cfgs.count();
	cfg->_offset = cfgs.isEmpty() ? 0 : cfgs[cfgs.length()-1]->offset() + cfgs[cfgs.length()-1]->count();
	cfg->_offset = cfgs.isEmpty() ? 0 : cfgs[cfgs.length()-1]->offset() + cfgs[cfgs.length()-1]->count();
	cfgs.add(cfg);
}

/**
 * Count the number of BB in the CFG collection
 * (sum of BB count of each CFG).
 * @return	Collection BB number.
 */
int CFGCollection::countBlocks(void) const {
	if(!cfgs)
		return 0;
	else
		return cfgs[count() - 1]->offset() + cfgs[count() - 1]->count();
}


/**
 * @class CFGCollector
 * This processor is used to collect all CFG implied in a computation.
 * It uses the @ref otawa::ENTRY_CFG or @ref otawa::TASK_ENTRY properties to find
 * the base CFG and explore in depth the CFG along subprograms calls.
 *
 * @par Configuration
 * @li @ref ENTRY_CFG -- CFG of the entry of the current task,
 * @li @ref TASK_ENTRY -- name if the entry function of the current task,
 * @li @ref CFGCollector::ADDED_CFG -- CFG to add to the collection.
 * @li @ref CFGCollector::ADDED_FUNCTION -- function name to add to the collection.
 *
 * @note @ref otawa::ENTRY_CFG is first looked on the @ref WorkSpace and then in the configuration properties.
 *
 * @par Provided Features
 * @ref COLLECTED_CFG_FEATURE
 *
 * @ingroup cfg
 */


/**
 */
CFGCollector::CFGCollector(p::declare& r):
	CFGProvider(r),
	builder(new AbstractCFGBuilder(*this))
{ }

/**
 * CFGCollector registration.
 */
p::declare CFGCollector::reg = p::init("otawa::CFGCollector", Version(2, 2, 0))
	.require(FLOW_FACTS_FEATURE)
	.require(TASK_INFO_FEATURE)
	.extend<CFGProvider>()
	.make<CFGCollector>();


/**
 */
void CFGCollector::setup(WorkSpace *ws) {
	added_cfgs[0] = TASK_INFO_FEATURE.get(ws)->entryInst()->address();

	// find address of label
	for(int i = 0; i < added_funs.count(); i++) {
		Address addr = ws->process()->program()->findLabel(added_funs[i]);
		if(addr.isNull())
			throw ProcessorException(*this, _ << "cannot find label \"" << added_funs[i] << "\"");
		Inst *inst = ws->findInstAt(addr);
		if(!inst)
			throw ProcessorException(*this, _ << "symbol \"" << added_funs[i] << "\" is not mapped in memory");
		builder->maker(inst);
	}

	// add instructions of CFGs to process
	for(int i = 0; i < added_cfgs.count(); i++) {
		Inst *inst = ws->findInstAt(added_cfgs[i]);
		if(!inst)
			throw ProcessorException(*this, _ << "address " << added_cfgs[i] << " is out of code segments.");
		builder->maker(inst);
	}
}


///
void CFGCollector::processWorkSpace(WorkSpace *ws) {

	// build the CFGs
	builder->process(ws);

	// build the CFG collection and clean markers
	auto coll = new CFGCollection();
	for(AbstractCFGBuilder::Iter m(*builder); m(); m++) {
		CFG *g = m->build();
		coll->add(g);
	}
	setCollection(coll);
	
	// if needed, set task name to the workspace
	if(ws->name() == "")
		ws->name(coll->entry()->name());

	// destroy builder
	delete builder;
	builder = nullptr;
}


///
void CFGCollector::configure(const PropList& props) {
	CFGProvider::configure(props);
	builder->configure(props);
	added_cfgs.add(Address::null);
	for(auto g: ADDED_CFG.all(props)) {
		added_cfgs.add(g);
		cout << "=======> " << g << endl;
	}
	for(auto f: ADDED_FUNCTION.all(props)) {
		added_funs.add(f);
	}
}


/**
 * This property is used to link the current computation involved CFG
 * on the framework.
 *
 * @par Hooks
 * @li @ref WorkSpace
 *
 * @ingroup cfg
 */
p::id<const CFGCollection *> INVOLVED_CFGS("otawa::INVOLVED_CFGS", 0);


/**
 * This property is used to get the entry CFG of the currently
 * processed task.
 *
 * @para Hooks
 * @li @ref WorkSpace
 *
 * @ingroup cfg
 */
p::id<CFG *> ENTRY_CFG("otawa::entry_cfg", 0);


/**
 * This properties are put on a CFG to get the list of edges calling it.
 *
 * @par Hooks
 * @li @ref CFG
 *
 * @ingroup cfg
 */
p::id<Edge *> CALLED_BY("otawa::CALLED_BY", 0);


/**
 * This feature asserts that all CFG involved in the current computation has
 * been collected and accessible thanks to @ref INVOLVED_CFGS property
 *
 * @par Configuration
 * @li @ref BB_BOUNDS
 * @li @ref ENTRY_CFG
 * @li @ref ADDED_CFG
 * @li @ref ADDED_FUNCTION
 *
 * @par Properties
 * @li @ref ENTRY_CFG (@ref WorkSpace).
 * @li @ref INVOLVED_CFGS (@ref WorkSpace).
 *
 * @ingroup cfg
 */
p::interfaced_feature<const CFGCollection> COLLECTED_CFG_FEATURE("otawa::COLLECTED_CFG_FEATURE", new Maker<CFGCollector>());


/**
 * This configuration property allows to add unlinked CFG to the used CFG
 * collection.
 *
 * @ingroup cfg
 */
p::id<Address> ADDED_CFG("otawa::ADDED_CFG", 0);


/**
 * This configuration property allows to add unlinked functions to the used CFG
 * collection.
 *
 * @ingroup cfg
 */
p::id<CString> ADDED_FUNCTION("otawa::ADDED_FUNCTION", 0);

#if 0
/**
 */
class AssemblyView: public view::View {
	friend class CFGCollector;
public:
	AssemblyView(void): view::View("assembly", "Assembly View") {
	}

	view::Viewer *explore(WorkSpace *ws, const Vector<view::PropertyType *>& types) override {
		return new AssemblyViewer(ws, types);
	}

	class AssemblyViewer: public view::Viewer {
	public:
		AssemblyViewer(WorkSpace *ws, const Vector<view::PropertyType *>& props)
			:	view::Viewer(ws, props) { }

		void start(Block *b) override {
			if(!b->isBasic())
				_i = BasicBlock::InstIter();
			else
				_i = b->toBasic()->insts();
		}

		void start(Edge *e) override {
			_i = BasicBlock::InstIter();
		}

		Address item(void) const override { return _i->address(); }
		void next(void) override { _i.next(); }
		bool ended(void) const override { return _i.ended(); }

		void print(io::Output& out) override {
			out << ot::address(_i->address()) << "  " << *_i << io::endl;
		}

	private:
		BasicBlock::InstIter _i;
	};

};

/**
 * View to display the program in assembly.
 * @ingroup view
 */
view::View& ASSEMBLY_VIEW = Single<AssemblyView>::_;

class RegistersProperty: public view::PropertyType {
public:
	RegistersProperty(void): view::PropertyType(ASSEMBLY_VIEW, "registers", "Registers used by an instruction.") { }

	bool isAvailable(WorkSpace *ws) override {
		return ws->isProvided(otawa::REGISTER_USAGE_FEATURE);
	}

	class Viewer: public view::PropertyViewer {
	public:
		Viewer(void): view::PropertyViewer(&REGISTERS_PROPERTY), doit(false) { }

		virtual void print(io::Output& out) override {

			// display read registers
			{
				RegSet rs;
				_i->readRegSet(rs);
				if(!rs.isEmpty()) {
					bool fst = true;
					out << "read: ";
					for(auto r: rs) {
						if(fst)
							fst = false;
						else
							out << ", ";
						out << r;
					}
				}
			}

			// display written registers
			{
				RegSet rs;
				_i->writeRegSet(rs);
				if(!rs.isEmpty()) {
					bool fst = true;
					out << "write: ";
					for(auto r: rs) {
						if(fst)
							fst = false;
						else
							out << ", ";
						out << r;
					}
				}
			}
}

	private:
		virtual void start(Block *b) {
			doit = b->isBasic();
			if(doit)
				_i = b->toBasic()->insts();
			else
				_i = BasicBlock::InstIter();
		}

		virtual void start(Edge *e) {
			doit = false;
			_i = BasicBlock::InstIter();
		}

		virtual void step(const Viewer& it) {
			if(doit)
				_i++;
		}

		bool doit;
		BasicBlock::InstIter _i;
	};

private:
	view::PropertyViewer *visit(void) override {
		return nullptr;
	}
};

view::PropertyType& REGISTERS_PROPERTY = Single<RegistersProperty>::_;
#endif

} // otawa

