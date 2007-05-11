/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS.
 *
 *	WorkSpace class implementation
 */

#include <otawa/manager.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/ast/ASTInfo.h>
#include <otawa/ilp/System.h>
#include <otawa/cfg/CFGBuilder.h>
#include <config.h>
#include <elm/xom.h>


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
WorkSpace::WorkSpace(Process *_proc): proc(_proc) {
	TRACE(this << ".WorkSpace::WorkSpace(" << _proc << ')');
	assert(_proc);
	addProps(*_proc);
	Manager *man = _proc->manager();
	man->frameworks.add(this);
}


/**
 * Delete the workspace and the associated process.
 */
WorkSpace::~WorkSpace(void) {
	TRACE(this << ".WorkSpace::~WorkSpace()");
	clearProps();
	Manager *man = proc->manager();
	man->frameworks.remove(this);
	delete proc;
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
	return ASTInfo::getInfo(this);
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
		throw LoadException("cannot load \"%s\".", &path);
	xom::Element *conf = doc->getRootElement();
	if(conf->getLocalName() != "otawa"
	|| conf->getNamespaceURI() != "")
		throw LoadException("bad file type in \"%s\".", &path);
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
 * Record in the workspace that a feature is provided.
 * @param feature	Provided feature.
 */
void WorkSpace::provide(const AbstractFeature& feature) {
	if(!isProvided(feature))
		features.add(&feature);
}


/**
 * Test if a feature is provided.
 * @param feature	Feature to test.
 * @return			True if it is provided, false else.
 */
bool WorkSpace::isProvided(const AbstractFeature& feature) {
	return features.contains(&feature);
}


/**
 * Remove a feature. It is usually called by processor whose actions remove
 * some properties from the workspace.
 * @param feature	Feature to remove.
 */
void WorkSpace::remove(const AbstractFeature& feature) {
	features.remove(&feature);
}


/**
 * Ensure that a feature is provided.
 * @param feature	Required feature.
 * @param props		Configuration properties (optional).
 */
void WorkSpace::require(const AbstractFeature& feature, const PropList& props) {
	if(!isProvided(feature))
		feature.process(this, props);
}

} // otawa
