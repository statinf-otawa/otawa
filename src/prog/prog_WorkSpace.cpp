/*
 *	$Id$
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

#include <otawa/manager.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/ast/ASTInfo.h>
#include <otawa/ilp/System.h>
#include <otawa/cfg/CFGBuilder.h>
#include <config.h>
#include <elm/xom.h>
#include <otawa/proc/FeatureDependency.h>
#include <elm/deprecated.h>
 #include <elm/serial2/serial.h>
#include <otawa/prog/Loader.h>

// Trace
//#define FRAMEWORK_TRACE
#if defined(NDEBUG) || !defined(FRAMEWORK_TRACE)
#	define TRACE(str)
#else
#	define TRACE(str) cerr << __FILE__ << ':' << __LINE__ << ": " << str << '\n';
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
 *  CodeItem -> Segment [ arrowhead = diamond, label="items", taillabel="0..*" ];
 *  Symbol -> File[ arrowhead = diamond, label="symbols", taillabel="0..*" ];
 *  Inst -> CodeItem [ arrowhead = empty ];
 *
 *  Manager [ label="{Manager||}", URL="@ref otawa::Manager", root=true ]
 *  WorkSpace [ label="{WorkSpace||}", URL="@ref otawa::WorkSpace" ]
 *  Process [ label="{Process||}", URL="@ref otawa::Process" ]
 *  File [ label="{File||}", URL="@ref otawa::File" ]
 *  Segment [ label="{Segment||}", URL="@ref otawa::Segment" ]
 *  CodeItem [ label="{CodeItem||}", URL="@ref otawa::CodeItem" ]
 *  Inst [ label="{Inst||}", URL="@ref otawa::Inst" ]
 *  Symbol [ label="{Symbol||}", URL="@ref otawa::Symbol" ]
 * }
 * @enddot
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
 * @li the @ref otawa::Segment divides the program in different parts (code, date, etc),
 * @li the @ref otawa::ProgItem decompose each segment into code or data items,
 * @li the @ref otawa::Inst is an instance of @ref otawa::ProgItem that represents a single instruction,
 * @li the @ref otawa::DataItem is an instance of @ref otawa::ProgItem that represents a piece of data,
 * @li the @ref otawa::Symbol represents a named reference on some @ref ProgItem
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
WorkSpace::WorkSpace(Process *_proc): proc(_proc), featMap(), cancelled(false) {
	TRACE(this << ".WorkSpace::WorkSpace(" << _proc << ')');
	ASSERT(_proc);
	//Manager *man = _proc->manager();
	//man->frameworks.add(this);
	proc->link(this);
}


/**
 * Build a new workspace on the same process as the given one.
 * @param ws	Workspace to get the process form.
 */
WorkSpace::WorkSpace(const WorkSpace *ws): cancelled(false) {
	TRACE(this << ".WorkSpace::WorkSpace(" << ws << ')');
	ASSERT(ws);
	proc = ws->process();
	//Manager *man = proc->manager();
	//man->frameworks.add(this);
	proc->link(this);
}


/**
 * Delete the workspace and the associated process.
 */
WorkSpace::~WorkSpace(void) {
	TRACE(this << ".WorkSpace::~WorkSpace()");

	// clean-up
	Vector<const AbstractFeature *> deps;
	for(feat_map_t::Iterator dep(featMap); dep; dep++)
		deps.add(dep->getFeature());
	for(int i = 0; i < deps.length(); i++)
		if(isProvided(*deps[i]))
			invalidate(*deps[i]);
	clearProps();

	// removal from manager and process
	//Manager *man = proc->manager();
	//man->frameworks.remove(this);
	proc->unlink(this);
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
			for(Identifier<Symbol *>::Getter sym(inst, Symbol::ID); sym; sym++)
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

	// Already built ?
	CFGInfo *info = CFGInfo::ID(this);
	if(info)
		return info;

	// Build it
	CFGBuilder builder;
	builder.process(this);
	return CFGInfo::ID(this);
}


/**
 * Get the entry CFG of the program.
 * @return Entry CFG if any or null.
 * @deprecated
 */
CFG *WorkSpace::getStartCFG(void) {
	DEPRECATED;

	// Find the entry
	Inst *_start = start();
	if(!_start)
		return 0;

	// Get the CFG information
	CFGInfo *info = getCFGInfo();

	// Find CFG attached to the entry
	return info->findCFG(_start);
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
 * @deprecated
 */
const hard::CacheConfiguration& WorkSpace::cache(void) {
	DEPRECATED;
	return proc->cache();
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
void WorkSpace::loadConfig(const elm::system::Path& path) {
	xom::Builder builder;
	xom::Document *doc = builder.build(&path);
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
		elm::system::Path path = CONFIG_PATH(this);
		if(path) {
			loadConfig(path);
			conf = CONFIG_ELEMENT(this);
		}
	}
	return conf;
}

/**
 * Get the dependency graph node associated with the feature and workspace
 * @param feature	Provided feature.
 */
FeatureDependency* WorkSpace::getDependency(const AbstractFeature* feature) {
	return featMap.get(feature, NULL);
}

/**
 * Create a new FeatureDependency associated with the feature.
 * @param feature	Provided feature.
 */
 void WorkSpace::newFeatDep(const AbstractFeature* feature) {
 	ASSERT(featMap.get(feature, NULL) == NULL);
 	featMap.put(feature, new FeatureDependency(feature));
}

/**
 * Invalidates and delete the FeatureDependency associated w/ this feature and workspace
 * It has to be freed by the user.
 * @param feature	Provided feature.
 */
 void WorkSpace::delFeatDep(const AbstractFeature* feature) {
 	FeatureDependency *dep = featMap.get(feature, NULL);
 	ASSERT(dep);
 	featMap.remove(feature);
 	delete dep;
}

/**
 * Tests if the feature has a dependency graph associated with it, in the context of the present workspace
 * @param feature	Provided feature.
 */
bool WorkSpace::hasFeatDep(const AbstractFeature* feature) {
	return (featMap.exists(feature));
}

/**
 * Record in the workspace that a feature is provided.
 * Also update the feature dependency graph
 * @param feature	Provided feature.
 */
void WorkSpace::provide(const AbstractFeature& feature, const Vector<const AbstractFeature*> *required) {
	if(isProvided(feature))
		return;

	// record the providing of the feature
	ASSERT(!hasFeatDep(&feature));
	newFeatDep(&feature);
	FeatureDependency *pdep = getDependency(&feature);
	ASSERT(pdep);

	// add dependencies
	if(required != NULL) {
		ASSERTP(!required->contains(&feature), feature.name() << " provided and required!");
		for(int j = 0; j < required->count(); j++) {
			const AbstractFeature *rfeature = required->get(j);
			ASSERTP(isProvided(*rfeature), rfeature->name() << " is not provided as it should be !");
			pdep->add(getDependency(rfeature));
		}
	}
}

/**
 * Invalidate a feature (removing its dependencies)
 * @param feature	Provided feature.
 */
void WorkSpace::invalidate(const AbstractFeature& feature) {
	if (isProvided(feature)) {
		FeatureDependency *fdep = getDependency(&feature);

		// get dependents
		genstruct::Vector<FeatureDependency *> to_inv;
		for(FeatureDependency::Dependent dep(fdep); dep; dep++)
			to_inv.add(dep);

		// invalidate them
		for(int i = 0; i < to_inv.count(); i++)
			invalidate(*to_inv[i]->getFeature());

		delFeatDep(&feature);
		remove(feature);
	}
}


/**
 * Test if a feature is provided.
 * @param feature	Feature to test.
 * @return			True if it is provided, false else.
 */
bool WorkSpace::isProvided(const AbstractFeature& feature) {
	return featMap.get(&feature, 0);
}


/**
 * Remove a feature. It is usually called by processor whose actions remove
 * some properties from the workspace.
 * @param feature	Feature to remove.
 */
void WorkSpace::remove(const AbstractFeature& feature) {
	feature.clean(this);
}


/**
 * Ensure that a feature is provided.
 * @param feature	Required feature.
 * @param props		Configuration properties (optional).
 */
void WorkSpace::require(const AbstractFeature& feature, const PropList& props) {
	if(!isProvided(feature)) {
		feature.process(this, props);
	}
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

} // otawa
