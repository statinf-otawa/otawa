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


// Trace
//#define FRAMEWORK_TRACE
#if defined(NDEBUG) || !defined(FRAMEWORK_TRACE)
#	define TRACE(str)
#else
#	define TRACE(str) cerr << __FILE__ << ':' << __LINE__ << ": " << str << '\n';
#endif

namespace otawa {

/**
 * @class WorkSpace
 * A workspace represents a program, its run-time and all information about
 * WCET computation or any other analysis.
 */

	
/**
 * Build a new wokspace with the given process.
 * @param _proc	Process to use.
 */
WorkSpace::WorkSpace(Process *_proc): proc(_proc), featMap() {
	TRACE(this << ".WorkSpace::WorkSpace(" << _proc << ')');
	assert(_proc);
	//Manager *man = _proc->manager();
	//man->frameworks.add(this);
	proc->link(this);
}


/**
 * Build a new workspace on the same process as the given one.
 * @param ws	Workspace to get the process form.
 */
WorkSpace::WorkSpace(const WorkSpace *ws) {
	TRACE(this << ".WorkSpace::WorkSpace(" << ws << ')');
	assert(ws);
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
	for(feat_map_t::ItemIterator dep(featMap); dep; dep++)
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
 * Get the CFG of the project. If it does not exists, built it.
 */
CFGInfo *WorkSpace::getCFGInfo(void) {
	
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
 */
CFG *WorkSpace::getStartCFG(void) {
	
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
 */
ASTInfo *WorkSpace::getASTInfo(void) {
	DEPRECATED
	return 0;
}


/**
 */
const hard::CacheConfiguration& WorkSpace::cache(void) {
	return proc->cache();
}


/**
 * Build an ILP system with the default ILP engine.
 * @param max	True for a maximized system, false for a minimized.
 * @return		ILP system ready to use, NULL fi there is no support for ILP.
 */
ilp::System *WorkSpace::newILPSystem(bool max) {
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
	FeatureDependency *result = featMap.get(feature, NULL);
	ASSERT(result != NULL);
	ASSERT(!result->isInvalidated());
	return(result);
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
 	FeatureDependency *old = featMap.get(feature, NULL);
 	ASSERT(old);
 	featMap.remove(feature);
 	old->setInvalidated(true);
}

/**
 * Tests if the feature has a dependency graph associated with it, in the context of the present workspace
 * @param feature	Provided feature.
 */
bool WorkSpace::hasFeatDep(const AbstractFeature* feature) {
	ASSERT(!featMap.exists(feature) || !featMap.get(feature, NULL)->isInvalidated()); 
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
	
	// add dependencies
	if (required != NULL)
		for (int j = 0; j < required->count(); j++)
			if(required->get(j) != &feature && isProvided(*required->get(j)))
				getDependency(required->get(j))->addChild(getDependency(&feature));
}

/**
 * Invalidate a feature (removing its dependancies) 
 * @param feature	Provided feature.
 */
void WorkSpace::invalidate(const AbstractFeature& feature) {
	if (isProvided(feature)) {
		for (genstruct::DAGNode<FeatureDependency *>::Iterator iter(*getDependency(&feature)->graph); iter; iter++) {
			DAGNode<FeatureDependency *> *childNode = *iter;
			FeatureDependency *childDep = childNode->useValue();
			invalidate(*childDep->getFeature());
			getDependency(&feature)->removeChild(childDep);		
		}
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

} // otawa
