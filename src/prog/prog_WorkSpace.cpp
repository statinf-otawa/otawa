/*
 *	WorkSpace class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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

#include <elm/data/List.h>
#include <elm/deprecated.h>
#include <elm/serial2/serial.h>
#include <elm/sys/System.h>
#include <elm/xom.h>

#include <config.h>
#include <otawa/ilp/System.h>
#include <otawa/manager.h>
#include <otawa/proc/ProcessorPlugin.h>
#include <otawa/proc/FeatureDependency.h>
#include <otawa/proc/Processor.h>
#include <otawa/proc/Registry.h>
#include <otawa/prog/File.h>
#include <otawa/prog/Loader.h>
#include <otawa/prog/Symbol.h>
#include <otawa/prog/WorkSpace.h>

// Trace
//#define FRAMEWORK_TRACE
#if !defined(NDEBUG) && defined(FRAMEWORK_TRACE)
#	define TRACE(str) cerr << __FILE__ << ':' << __LINE__ << ": " << str << '\n';
#else
#	define TRACE(str)
#endif


/**
 * @defgroup prog	Program Representation
 *
 * @dot
 * digraph {
 * 	node [shape = record, fontsize=10 ];
 * 	edge [ arrowhead = vee, fontsize=8 ];
 *	rankdir=TB;
 *
 *	Manager -> WorkSpace [ label="<<create>>",style=dashed  ];
 *  WorkSpace -> Process [ label="process", headlabel="1" ];
 *  File -> Process [ arrowhead = diamond, label="files", taillabel="1..*" ];
 *  Process -> File [ label="program", headlabel="1" ];
 *  Segment -> File [ arrowhead = diamond, label="segments", taillabel="1..*" ];
 *  ProgItem -> Segment [ arrowhead = diamond, label="items", taillabel="0..*" ];
 *  Symbol -> File[ arrowhead = diamond, label="symbols", taillabel="0..*" ];
 *  Inst -> ProgItem [ arrowhead = empty ];
 *
 *  Manager [ label="{Manager||}", URL="@ref otawa::Manager", root=true ]
 *  WorkSpace [ label="{WorkSpace||}", URL="@ref otawa::WorkSpace" ]
 *  Process [ label="{Process||}", URL="@ref otawa::Process" ]
 *  File [ label="{File||}", URL="@ref otawa::File" ]
 *  Segment [ label="{Segment||}", URL="@ref otawa::Segment" ]
 *  ProgItem [ label="{ProgItem||}", URL="@ref otawa::ProgItem" ]
 *  Inst [ label="{Inst||}", URL="@ref otawa::Inst" ]
 *  Symbol [ label="{Symbol||}", URL="@ref otawa::Symbol" ]
 * }
 * @enddot
 *
 * @section prog_desc	Description
 *
 * The program representation module of OTAWA is the main module providing all details
 * about the processed program. It provides a representation built from the program
 * binary form (@ref Process) and provides a workspace to perform analyses (@ref WorkSpace).
 * Usually, a workspace containing a process is provided by the manager (@ref otawa::Manager)
 * as a simple Manager::load() call:
 * @code
 * #include <otawa/otawa.h>
 *
 * using namespace elm;
 * using namespace otawa;
 *
 * try {
 *   PropList props;
 *   Processor::VERBOSE(props) = true;
 *   WorkSpace *ws = MANAGER.load("path_to_file", props);
 *   ...
 *   return 0;
 * }
 * catch(otawa::Exception& e) {
 *   cerr << "ERROR: " << e.message() << io::endl;
 *   return 1;
 * }
 * @endcode
 * The load may possibly fail and throw an @ref otawa::Exception exception.
 *
 * The @ref Process class describes the full program and its execution environment as
 * different items of description:
 * @li the @ref otawa::File represents a binary file involved in the building of the execution environments
 *   (each program has at least one file containing the main program and possibly
 *   other file for dynamically linked libraries -- most often zero in embedded systems),
 * @li the @ref otawa::Segment divides the program in different parts (code, data, etc),
 * @li the @ref otawa::ProgItem decompose each segment into code or data items,
 * @li the @ref otawa::Inst is an instance of @ref otawa::ProgItem that represents a single instruction,
 * @li the @ref otawa::DataItem is an instance of @ref otawa::ProgItem that represents a piece of data,
 * @li the @ref otawa::Symbol represents a named reference on some @ref ProgItem
 *
 * To load a program, the following properties may be useful:
 * @li @ref otawa::TASK_ENTRY -- name of the function used as entry point.
 * @li @ref otawa::LOADER_NAME -- name of loader to use.
 * @li @ref otawa::CACHE_CONFIG -- handle on cache configuration.
 * @li @ref otawa::CACHE_CONFIG_PATH -- path to cache configuration.
 * @li @ref otawa::MEMORY_OBJECT -- handle on memory configuration.
 * @li @ref otawa::MEMORY_PATH -- path to memory configuration.
 * @li @ref otawa::PROCESSOR -- handle on microprocessor configuration.
 * @li @ref otawa::PROCESSOR_PATH -- path to microprocessor configuration.
 * @li @ref otawa::LOAD_PARAM -- useful to pass a parameter to a loader in the form "ID=VALUE".
 *
 * Expert users can also use the following properties:
 * @li @ref otawa::LOADER -- handle of loader to use.
 * @li @ref otawa::ARGC -- count of arguments passed to task on the stack.
 * @li @ref otawa::ARGV -- null-terminated list of arguments passed to task on the stack.
 * @li @ref otawa::ENVP -- null-terminated list of environments to task on the stack.
 * @li @ref otawa::NO_SYSTEM -- no system is running the task.
 *
 * A commonly used propertu with IPET method is the property below:
 * @li @ref otawa::ipet::ILP_PLUGIN_NAME -- to select which to use to solve the IPET ILP system.
 *
 * Notice that most properties listed above may be used with @ref owcet command
 * or most OTAWA command using "--add-prop" option. Ask for command description
 * with "-h" option or look at @ref otawa::Application for more details.
 *
 *
 * @section prog_vliw	VLIW Support
 *
 * VLIW (Very Long Instruction Word) is a technology allowing to execute in parallel
 * several instructions, in order, without the logic needed to analyze dependencies
 * between instructions and to re-order instructions as found in out-of-order
 * architectures. Instruction are grouped into bundles that are guaranteed
 * by compiler to be executable in parallel.
 *
 * For instance, runned on a VLIW computer, the ARM instructions below perform
 * actually an exchange of register R0 and R1 (end of bundle is denoted by double semi-colon):
 * @code
 * 	MOV R0, R1;
 * 	MOV R1, R0;;
 * @endcode
 * In fact, when the bundle above is executed, registers R1 and R0 are read in parallel and the
 * assignment to R0, respectively to R1, is also performed in parallel.
 *
 * OTAWA provides a specific support for VLIW but the bundle-aware resources can be used as is
 * by non-VLIW instruction set: bundles will be composed of only one instruction in this case.
 * Whatever, using bundle-aware architecture allows adaptation for free of analyzes to VLIW and
 * non-VLIW  architectures. Following bundle facilities are available:
 * @li Inst::isBundleEnd() -- end of bundle detection,
 * @li Inst::semInsts(sem::Block&, int temp) -- semantic instruction generation with temporary re-basing,
 * @li Inst::semWriteBack() -- parallel write-back generation of registers from temporaries to actual registers,
 * @li BasicBlock::BundleIter -- iterator on the bundle composing a basic block.
 *
 * A special attention must be devoted to supporting semantic instruction. Model of execution of
 * semantic instruction is purely sequential. Hence, VLIW instruction set semantic cannot be preserved
 * if semantic instructions of machine are executed sequentially. Re-using the example of register
 * exchange above, the straight translation into semantics will only copy R1 to R0:
 * @code
 *	MOV R0, R1
 *		SET(R0, R1)
 *	MOV R1, R0
 *		SET(R1, R0)
 * @endcode
 *
 * A simple trick allows maintaining the current semantic instruction behavior and to adapt without effort
 * existing analysis to VLIW: just copy write-back registers into temporaries and delay write-back to
 * the end of execution of semantic instructions. Therefore, the semantic instructions implementing a
 * machine instructions of a bundle need only to be concatenated and ended by write-back operations.
 * uses these temporaries instead (our example continued):
 * @code
 * 	MOV R0, R1
 * 		SET(T1, R1)		// T1 refers to R0
 * 	MOV R1, R0			// T2 refers to R1
 * 		SET(T2, R0)
 *
 * 		SET(R0, T1)
 * 		SET(R1, R2)
 * @endcode
 *
 * This requires the help of the VLIW instructions to build such a sequence. Usually, the template
 * to translate into semantic instructions looks like:
 * @code
 *	MOV ri, rj
 *		SET(ri, rj)
 * @endcode
 *
 * For VLIW, this template must be re-defined to perform write-back on temporaries:
 * @code
 * 	MOV ri, rj
 * 		semantic
 * 			SET(temp, rj)
 * 		write-back (1 temporary used)
 * 			SET(ri, temp)
 * @endcode
 *
 * Write-back sequence is obtained by a call to Inst::semWriteBack() that returns also the number
 * of used temporaries and Inst::semInsts() allows generating the semantic instruction using
 * a specific temporary base. To detail the example, we get:
 * @code
 * 	Inst(MOV R0, R1).semInsts(block, T1) = 1
 * 		SET(T1, R1)
 * 	Inst(MOV R1, R0).semInsts(block, T2) = 1
 * 		SET(T2, R0)
 *	Inst(MOV R0, R1).semWriteBack(block, T1) = 1
 *		SET(R0, T1)
 *	Inst(MOV R1, R0).semWriteBack(block, T2) = 1
 *		SET(R1, T2)
 * @endcode
 */

namespace otawa {

/**
 * @class WorkSpace
 * A workspace represents a program, its run-time and all information about
 * WCET computation or any other analysis.
 * @ingroup prog
 */


/**
 * Build a new wokspace with the given process.
 * @param _proc	Process to use.
 */
WorkSpace::WorkSpace(Process *_proc): proc(_proc), cancelled(false) {
	TRACE(this << ".WorkSpace::WorkSpace(" << _proc << ')');
	ASSERT(_proc);
	const List<AbstractFeature *>& feats = _proc->features();
	for(List<AbstractFeature *>::Iter i(feats); i(); i++) {
		Dependency *dep = new Dependency();
		TRACE("added process feature " << dep << " for " << i->name());
		dep_map.put(*i, dep);
	}
}


/**
 * Build a new workspace on the same process as the given one.
 * @param ws	Workspace to get the process form.
 */
WorkSpace::WorkSpace(const WorkSpace *ws): cancelled(false) {
	TRACE(this << ".WorkSpace::WorkSpace(" << ws << ')');
	ASSERT(ws);
	proc = ws->process();
	const List<AbstractFeature *>& feats = ws->process()->features();
	for(List<AbstractFeature *>::Iter i(feats); i(); i++) {
		Dependency *dep = new Dependency();
		TRACE("added process feature " << dep << " for " << i->name());
		dep_map.put(*i, dep);
	}
}


/**
 * Delete the workspace and the associated process.
 */
WorkSpace::~WorkSpace(void) {
	TRACE(this << ".WorkSpace::~WorkSpace()");

	// collect root dependencies
	Vector<Dependency *> roots;
	for(dep_map_t::Iter i(dep_map); i(); i++)
		if((*i)->_used.isEmpty() && !roots.contains(*i)) {
			TRACE("ROOT: " << *i << " -> " << i->_proc << " for " << i.key()->name());
			roots.add(*i);
		}

	// invalidate dependencies
	for(Vector<Dependency *>::Iter i(roots); i(); i++)
		invalidate(*i);
}


/**
 * Format an address to make it human-readable.
 * First look for debugging information, else try to build the form label + offset
 * else fall back to address itself.
 * @param addr			Address to display.
 * @param with_address	Set to true to ever display the address.
 * @return				Formatted string.
 */
string WorkSpace::format(Address addr, bool with_address) {
	static string null_str = "<null>";
	StringBuffer buf;
	bool done = false;

	// null case
	if(addr.isNull())
		return null_str;

	// look in the debugging information
	Option<Pair<cstring, int> > line = process()->getSourceLine(addr);
	if(line) {
		done = true;
		buf << (*line).fst << ':' << (*line).snd;
	}

	// find the closer function symbol
	if(!done) {
		Inst *inst = findInstAt(addr);
		while(inst && !done) {
			for(Identifier<Symbol *>::Getter sym(inst, SYMBOL); sym(); sym++)
				/*if(sym->kind() == Symbol::FUNCTION)*/ {
					done = true;
					buf << '"' << sym->name() << '"';
					if(addr != inst->address())
						buf << " + 0x" << io::hex(addr - inst->address());
				}
			inst = inst->prevInst();
		}
	}

	// fall back to simple address
	if(!done) {
		buf << addr;
		with_address = false;
	}

	// if required, display the address
	if(with_address)
		buf << " (" << addr << ')';
	return buf.toString();
}



/**
 * Get the CFG of the project. If it does not exists, built it.
 * @deprecated
 */
CFGInfo *WorkSpace::getCFGInfo(void) {
	DEPRECATED;
	return 0;
#	if 0
	// Already built ?
	CFGInfo *info = CFGInfo::ID(this);
	if(info)
		return info;

	// Build it
	CFGBuilder builder;
	builder.process(this);
	return CFGInfo::ID(this);
#	endif
}


/**
 * Get the entry CFG of the program.
 * @return Entry CFG if any or null.
 * @deprecated
 */
CFG *WorkSpace::getStartCFG(void) {
	DEPRECATED;
	return 0;

#	if 0
	// Find the entry
	Inst *_start = start();
	if(!_start)
		return 0;

	// Get the CFG information
	CFGInfo *info = getCFGInfo();

	// Find CFG attached to the entry
	return info->findCFG(_start);
#	endif
}


/**
 * Get the AST of the project.
 * @deprecated
 */
ast::ASTInfo *WorkSpace::getASTInfo(void) {
	DEPRECATED
	return 0;
}


/**
 * Build an ILP system with the default ILP engine.
 * @param max	True for a maximized system, false for a minimized.
 * @return		ILP system ready to use, NULL fi there is no support for ILP.
 * @deprecated
 */
ilp::System *WorkSpace::newILPSystem(bool max) {
	DEPRECATED;
	return manager()->newILPSystem();
}


/**
 * Load the given configuration in the process.
 * @param path				Path to the XML configuration file.
 * @throw LoadException		If the file cannot be found or if it does not match
 * the OTAWA XML type.
 */
void WorkSpace::loadConfig(const elm::sys::Path& path) {
	xom::Builder builder;
	xom::Document *doc = builder.build(path.asSysString());
	if(!doc)
		throw LoadException(_ << "cannot load \"" << path << "\".");
	xom::Element *conf = doc->getRootElement();
	if(conf->getLocalName() != "otawa"
	|| conf->getNamespaceURI() != "")
		throw LoadException(_ << "bad file type in \"" << path << "\".");
	CONFIG_ELEMENT(this) = conf;
}


/**
 * Get the current configuration, if any, as an XML XOM element.
 * @return	Configuration XML element or null.
 */
xom::Element *WorkSpace::config(void) {
	xom::Element *conf = CONFIG_ELEMENT(this);
	if(!conf) {
		elm::sys::Path path = CONFIG_PATH(this);
		if(path) {
			loadConfig(path);
			conf = CONFIG_ELEMENT(this);
		}
	}
	return conf;
}


/**
 * Find and run the code processor corresponding to the given name
 * with the given configuration.
 * @param props	Property list to use (optional).
 * @return		The built processor.
 * @throw otawa::Exception -- if there is an error in the processor or
 * the processor cannot be found.
 */
Processor *WorkSpace::run(cstring name, const PropList& props) {
	Processor *p = otawa::ProcessorPlugin::getProcessor(name);
	if(p == nullptr)
		throw Exception(_ << "code processor " << name << " cannot be found!");
	run(p, props, true);
	return p;
}


/**
 * Processor *WorkSpace::run(const PropList& props);
 * Create a processor of type T and run it in the current workspace using the given
 * properties. As locally declared processor are no more allowed, this function is
 * the easier to call a processor which class is known.
 *
 * @param T			Class of the processor.
 * @param props		Configuration properties (optional).
 * @return			The built processor.
 * @throw otawa::Exception	As thrown by the code processor during its execution.
 */


/**
 * Run the given in the current workspace.
 *
 * Notice that, after this call, the workspace is the owner of the passed processor object
 * and, therefore, will be in charge of releasing it at the right time.
 *
 * @param proc		Processor to run.
 * @param props		Configuration to use.
 * @param del_proc	Set to true if the processor needs to be deleted when it is no more used.
 */
void WorkSpace::run(Processor *proc, const PropList& props, bool del_proc) {
	if(proc->isDone())
		return;

	// run the processor
	proc->configure(props);
	AbstractRegistration& reg = proc->registration();

	// build the list of required features
	Vector<const AbstractFeature *> required;
	for(FeatureIter fuse(reg); fuse(); fuse++)
		if(fuse->kind() == FeatureUsage::require && !required.contains(&fuse->feature()))
			required.add(&fuse->feature());

	// remove non-used invalidated features
	for(FeatureIter feature(reg); feature(); feature++)
		if(feature->kind() == FeatureUsage::invalidate
		&& !reg.uses(feature->feature())) {
			if(!proc->isQuiet() && proc->logFor(Processor::LOG_DEPS))
				proc->log << "INVALIDATED: " << feature->feature().name()
					<< " by " << reg.name() << io::endl;
			invalidate(feature->feature());
			required.remove(&feature->feature());
		}

	// Get used feature
	for(FeatureIter feature(reg); !isCancelled() && feature(); feature++)
		if(feature->kind() == FeatureUsage::require
		|| feature->kind() == FeatureUsage::use) {
			if(!proc->isQuiet() && proc->logFor(Processor::LOG_DEPS)) {
				cstring kind = "USED";
				if(feature->kind() == FeatureUsage::require)
					kind = "REQUIRED";
				proc->log << kind << ": " << feature->feature().name() << " by " << reg.name() << io::endl;
			}
			try {
				require(feature->feature(), props);
			}
			catch(NoProcessorException& e) {
				proc->log << "ERROR: no processor to implement " << feature->feature().name() << io::endl;
				throw UnavailableFeatureException(proc, feature->feature());
			}
		}
	if(isCancelled())
		return;

	// check before starting processor
	for(FeatureIter feature(reg); feature(); feature++)
		if((feature->kind() == FeatureUsage::require
		|| feature->kind() == FeatureUsage::use)
		&& !isProvided(feature->feature()))
			throw otawa::Exception(_ << "feature " << feature->feature().name()
				<< " is not provided after one cycle of requirements:\n"
				<< "stopping -- this may denotes circular dependencies.");

	// run the analysis
	proc->run(this);
	proc->flags |= Processor::IS_DONE;

	// cleanup used invalidated features
	for(FeatureIter feature(reg); feature(); feature++)
		if(feature->kind() == FeatureUsage::invalidate
		&& reg.uses(feature->feature())) {
			if(!proc->isQuiet() && proc->logFor(Processor::LOG_DEPS))
				proc->log << "INVALIDATED: " << feature->feature().name() << " by " << reg.name() << io::endl;
			invalidate(feature->feature());
			required.remove(&feature->feature());
		}

	// create the dependency
	proc->commit(this);
	int provides = 0;
	for(FeatureIter feature(reg); feature(); feature++)
		if(feature->kind() == FeatureUsage::provide) {
			provides++;
			if(!proc->isQuiet() && proc->logFor(Processor::LOG_DEPS))
				proc->log << "PROVIDED: " << feature->feature().name() << " by " << reg.name() << io::endl;
		}
	if(provides)
		add(proc, del_proc);
}


/**
 * Add a new dependency.
 * @param proc		Current processor.
 * @param del_proc	True if the processor has to be deleted.
 */
void WorkSpace::add(Processor *proc, bool del_proc) {
	ASSERT(proc);
	Dependency *d = new Dependency(proc, del_proc);
	TRACE("added " << d << " for " << proc->name());
	for(FeatureIter fu(proc->registration()); fu(); fu++)
		if(fu->kind() == FeatureUsage::provide) {
			dep_map.put(&fu->feature(), d);
			TRACE("\tfor " << fu->feature().name());
		}
		else if(fu->kind() == FeatureUsage::require) {
				Dependency *ud = dep_map.get(&fu->feature(), 0);
				if(!ud) {
					if(!proc->registration().invalidates(fu->feature()))
						throw otawa::Exception(_ << proc->name() << " requires " << fu->feature().name() << " but it seems it has invalidated it!");
				}
				else if(!ud->_users.contains(d)) {
					d->_used.add(ud);
					ud->_users.add(d);
				}
			}
	proc->flags |= Processor::IS_TIED;
}


/**
 * Remove a dependency.
 * @param dep	Dependency to remove.
 */
void WorkSpace::remove(Dependency *dep) {
	ASSERT(dep);
	TRACE("remove " << dep << " for " << dep->_proc->name());

	// remove provided features
	//for(List<FeatureUsage>::Iter fu(dep->_proc->registration().features()); fu; fu++)
	for(FeatureIter fu(dep->_proc->registration()); fu(); fu++)
		if(fu->kind() == FeatureUsage::provide)
			dep_map.remove(&fu->feature());

	// release its own dependencies
	for(List<Dependency *>::Iter i(dep->_used); i(); i++)
		i->_users.remove(dep);

	// free the memory
	dep->_proc->flags &= ~Processor::IS_TIED;
	if(dep->_del_proc)
		delete dep->_proc;
	delete dep;
}


/**
 * Invalidate a feature (removing its dependencies)
 * @param feature	Provided feature.
 */
void WorkSpace::invalidate(const AbstractFeature& feature) {
	Dependency *d = dep_map.get(&feature, 0);
	ASSERTP(d, "dependency " << feature.name() << " is not provided!");
	invalidate(d);
}


/**
 * Get the implementor of a feature (if provided).
 * @param feature	Feature for which the implementor is looked.
 * @return			Found implementor or null pointer.
 */
Processor *WorkSpace::getImpl(const AbstractFeature& feature) const {
	Dependency *d = dep_map.get(&feature, nullptr);
	if(d == nullptr)
		return nullptr;
	else
		return d->_proc;
}



/**
 * Invalidate a dependency (removing its dependencies)
 * @param dep	Dependency to invalidate.
 */
void WorkSpace::invalidate(Dependency *dep) {
	Vector<Dependency *> stack;
	stack.push(dep);

	while(stack) {
		Dependency *d = stack.top();

		// still dependencies to free
		if(!d->_users.isEmpty())
			stack.push(d->_users.pop());

		// free dependency: delete it!
		else {
			stack.pop();
			d->_proc->destroy(this);
			remove(d);
		}

	}
}


/**
 * Test if a feature is provided.
 * @param feature	Feature to test.
 * @return			True if it is provided, false else.
 */
bool WorkSpace::provides(const AbstractFeature& feature) {
	return dep_map.hasKey(&feature);
}


/**
 * Test if the named feature is provided.
 * @param name	Name of the look feature.
 * @return		True if the feature is available, false else.
 */
bool WorkSpace::provides(cstring name) {
	AbstractIdentifier *id = AbstractIdentifier::find(name);
	if(id == nullptr || !p::is_feature(id))
		return false;
	else
		return provides(*static_cast<const AbstractFeature *>(id));
}


/**
 * @fn bool WorkSpace::implements(const AbstractFeature& feature);
 * Test if a feature is provided.
 * @param feature	Feature to test.
 * @return			True if it is provided, false else.
 */


/**
 * @fn bool WorkSpace::isProvided(const AbstractFeature& feature);
 * Test if a feature is provided.
 * @param feature	Feature to test.
 * @return			True if it is provided, false else.
 * @deprecated		Use provides() instead.
 */


/**
 * Ensure that a feature is provided.
 * @param feature	Required feature.
 * @param props		Configuration properties (optional).
 */
void WorkSpace::require(const AbstractFeature& feature, const PropList& props) {
	if(!isProvided(feature))
		feature.process(this, props);
}


/**
 * @fn void WorkSpace::clearCancellation(void);
 * Reset the cancellation bit. This function must be called before
 * starting a long running time computation.
 */


/**
 * @fn void WorkSpace::cancel(void);
 * Informs the current computation to stop as soon as possible.
 * This feature has not direct on the workspace but is used by code processor
 * to change their behaviour. It may be used to stop a computation
 * in a parallel or a GUI context.
 */


/**
 * @fn bool WorkSpace::isCancelled(void) const;
 * Test if the cancel() method has been called on the workspace.
 * This method is usually called by code processor to know if
 * the computation has been canceled.
 * @return	True if the computation has been cancelled, false else.
 */


/**
 * Serialize the workspace to the current serializer.
 * @param serializer	Serializer to serialize to.
 */
void WorkSpace::serialize(elm::serial2::Serializer& serializer) {

	// link to the loader
	serializer.beginField("loader");
	serializer << process()->loader()->path().toString();
	serializer.endField();

	// save the feature instances

	// save the program representation

}


/**
 * Unserialize the workspace to the current unserializer.
 * @param unserializer	Unserializer to unserialize to.
 */
void WorkSpace::unserialize(elm::serial2::Unserializer& unserializer) {
	// do nothing for now
}


#ifdef OTAWA_CONC
#	define CONC_DEBUG(x)	//cerr << x
	class RCURunnable: public sys::Runnable {
		friend class WorkSpace;
	public:

		RCURunnable(void): to_run(0) {
			sys::Thread::make(*this);
		}

		inline bool isRoot(void) const { return this == &root; }

		virtual void run(void) {
			ASSERT(to_run);
			CONC_DEBUG("DEBUG: running " << (void *)this << " on " << (void *)to_run << io::endl);

			// launch the runnable
			to_run->run();

			// end launching
			mutex->lock();
			running.remove(this);
			avail.add(this);
			check();
			mutex->unlock();
		}

		void clean(void) {
			for(List<Property *>::iter i(to_free); i; i++)
				delete *i;
			to_free.clear();
		}

		static void init(void) {
			static bool initialized = false;
			if(!initialized) {
				initialized = true;
				mutex = sys::Mutex::make();
				core_count = sys::System::coreCount();
			}
		}

		static void check(void) {
			if(!running) {
				for(List<RCURunnable *>::iter i(avail); i; i++)
					i->clean();
				root.clean();
			}
		}

		static void runInRoot(sys::Runnable *runnable) {
			runnable->run();
			mutex->lock();
			check();
			mutex->unlock();
		}

		static sys::Thread *run(sys::Runnable *runnable) {

			// lock
			init();
			mutex->lock();

			// look for a free thread
			if(avail) {
				RCURunnable *t = avail.pop();
				running.add(t);
				CONC_DEBUG("DEBUG: " << (void *)t << " has to run " << (void *)runnable << io::endl);
				t->to_run = runnable;
				t->thread()->start();
				mutex->unlock();
				return t->thread();
			}

			// add a new one if any core is remaining
			else if(running.count() < core_count - 1) {
				RCURunnable *t = new RCURunnable();
				CONC_DEBUG(cerr << "DEBUG: created " << (void *)t << io::endl);
				running.add(t);
				CONC_DEBUG(cerr << "DEBUG: " << (void *)t << " has to run " << (void *)runnable << io::endl);
				t->to_run = runnable;
				t->thread()->start();
				mutex->unlock();
				return t->thread();
			}

			// only main core remains: run in main core
			else {
				mutex->unlock();
				runInRoot(runnable);
				return 0;
			}
		}

		static void runAll(Runnable *runnable) {
			CONC_DEBUG("DEBUG: RCU::runAll()\n");

			// count available threads
			init();
			mutex->lock();
			int c = RCURunnable::core_count - running.count() - 1;
			mutex->unlock();
			CONC_DEBUG("DEBUG: " << c << " threads available (" << core_count << ").\n");

			// run available threads
			Vector<sys::Thread *> to_wait;
			for(int i = 0; i < c; i++) {
				to_wait.add(run(runnable));
				CONC_DEBUG("DEBUG: launched thread " << (void *)to_wait.top() << io::endl);
			}
			runInRoot(runnable);		// root thread

			// wait for end of used threads
			for(int i = 0; i < to_wait.count(); i++)
				if(to_wait[i]) {
					CONC_DEBUG("DEBUG: waiting for thread " << (void *)to_wait[i] << io::endl);
					to_wait[i]->join();
					CONC_DEBUG("DEBUG: ended for thread " << (void *)to_wait[i] << io::endl);
				}
		}

		static void remove(Property *prop) {
			if(!running)
				delete prop;
			else
				static_cast<RCURunnable&>(current()).to_free.add(prop);
		}

	private:
		sys::Runnable *to_run;
		List<Property *> to_free;

		static RCURunnable root;
		static sys::Mutex *mutex;
		static List<RCURunnable *> avail, running;
		static int core_count;
	};

	RCURunnable RCURunnable::root;
	sys::Mutex *RCURunnable::mutex = 0;
	List<RCURunnable *> RCURunnable::avail, RCURunnable::running;
	int RCURunnable::core_count = 0;
#endif


/**
 * Launch in a different thread the given runnable.
 * If no more thread is available, launch the runnable in the
 * current thread.
 *
 * Using this method to start a thread allows the
 * management of property in thread-safe way.
 *
 * @param runnable	Runnable to launch.
 * @return			Started thread or null if the runnable has been launched
 * in the current thread.
 */
sys::Thread *WorkSpace::run(sys::Runnable& runnable) {
#	ifdef OTAWA_CONC
		return RCURunnable::run(&runnable);
#	else
		runnable.run();
		return 0;
#	endif
}

/**
 * Run all threads with the given runnable. Return only
 * when all threads has stopped.
 *
 * Using this method to start a thread allows the
 * management of property in thread-safe way.
 *
 * @param runnable	Runnable to launch.
 */
void WorkSpace::runAll(sys::Runnable& runnable) {
#	ifdef OTAWA_CONC
		RCURunnable::runAll(&runnable);
#	else
		runnable.run();
#	endif
}

/**
 * Remove a property in a thread-safe way.
 * @param prop	Property to remove.
 */
void WorkSpace::remove(Property *prop) {
#	ifdef OTAWA_CONC
		RCURunnable::remove(prop);
#	else
		delete prop;
#	endif
}


/**
 */
WorkSpace::Dependency::Dependency( Processor *proc, bool del_proc)
: _proc(proc), _del_proc(del_proc) {
}


/**
 */
WorkSpace::Dependency::Dependency(void): _proc(&Processor::null), _del_proc(false) {
}


/**
 * Get the platform used by the process.
 * @return	Process platform.
 */
hard::Platform *WorkSpace::platform(void) {
	return proc->platform();
}

/**
 * Get the manager of the workspace.
 * @return	Workspace manager.
 */
Manager *WorkSpace::manager(void) {
	return proc->manager();
}

/**
 * Get the start instruction of the program.
 * @return	Program start instruciton.
 */
Inst *WorkSpace::start(void) {
	return proc->start();
}

/**
 * Find an instruction at the given address.
 * @param addr	Looked address.
 * @return		Found instruction or null (bad instruction or not in code memory).
 */
Inst *WorkSpace::findInstAt(address_t addr) {
	return proc->findInstAt(addr);
}

/**
 * @fn string WorkSpace::name() const;
 * Get the name of the workspace (if any) or an empty string.
 * @return	Workspace name.
 */

/**
 * @fn WorkSpace& WorkSpace::name(string name);
 * Set the name of the workspace.
 * @param name	New name of the workspace.
 */

/**
 * Get the working directory. If the binary has for path DIR/FILE.EXT
 * and if the workspace has for name NAME, the working directory is:
 *
 * DIR/NAME-otawa
 *
 * If the workspace has no name, the working directory is:
 *
 * DIR/FILE-otwaa
 *
 * @return	Working directory path.
 */
sys::Path WorkSpace::workDir() {
	if(wdir.isEmpty()) {
		string task;
		if(_name != "")
			task = _name;
		else
			task = sys::Path(process()->program_name()).withoutExt().namePart();
		wdir = sys::Path(process()->program_name()).parent() / (task + "-otawa");
	}
	return wdir;
}

/**
 * Build and return the working directory.
 * @return	Working directory path.
 * @throw elm::sys::SystemException		If there is an IO error.
 */
sys::Path WorkSpace::makeWorkDir() {
	sys::Path p = workDir();
	if(!p.isDir()) {
		if(p.exists())
			p.remove();
		p.makeDirs();
	}
	return p;
}

/**
 * @fn WorkSpace& WorkSpace::workDir(sys::Path path);
 * Set the path of the working directory.
 * @param path	new path to the working directory.
 */

} // otawa
