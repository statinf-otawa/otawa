/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/prog/FrameWork.cpp -- implementation for FrameWork class.
 */

#include <otawa/manager.h>
#include <otawa/prog/FrameWork.h>
#include <otawa/ast/ASTInfo.h>
#include <otawa/ilp/System.h>
#include <otawa/cfg/CFGBuilder.h>
#include <config.h>


// Trace
//#define FRAMEWORK_TRACE
#if defined(NDEBUG) || !defined(FRAMEWORK_TRACE)
#	define TRACE(str)
#else
#	define TRACE(str) cerr << __FILE__ << ':' << __LINE__ << ": " << str << '\n';
#endif

namespace otawa {

/**
 * @class FrameWork
 * A framework represents a program, its run-time and all information about
 * WCET computation or any other analysis.
 */

	
/**
 * Build a new framework with the given process.
 * @param _proc	Process to use.
 */
FrameWork::FrameWork(Process *_proc): proc(_proc) {
	TRACE(this << ".FrameWork::FrameWork(" << _proc << ')');
	assert(_proc);
	addProps(*_proc);
	Manager *man = _proc->manager();
	man->frameworks.add(this);
}


/**
 * Delete the framework and the associated process.
 */
FrameWork::~FrameWork(void) {
	TRACE(this << ".FrameWork::~FrameWork()");
	clearProps();
	Manager *man = proc->manager();
	man->frameworks.remove(this);
	delete proc;
}


/**
 * Get the CFG of the project. If it does not exists, built it.
 */
CFGInfo *FrameWork::getCFGInfo(void) {
	
	// Already built ?
	CFGInfo *info = get<CFGInfo *>(CFGInfo::ID, 0);
	if(info)
		return info;
	
	// Build it
	CFGBuilder builder;
	builder.processFrameWork(this);
	return use<CFGInfo *>(CFGInfo::ID);
}


/**
 * Get the entry CFG of the program.
 * @return Entry CFG if any or null.
 */
CFG *FrameWork::getStartCFG(void) {
	
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
ASTInfo *FrameWork::getASTInfo(void) {
	return ASTInfo::getInfo(this);
}


/**
 */
const hard::CacheConfiguration& FrameWork::cache(void) {
	return proc->cache();
}


/**
 * Build an ILP system with the default ILP engine.
 * @param max	True for a maximized system, false for a minimized.
 * @return		ILP system ready to use, NULL fi there is no support for ILP.
 */
ilp::System *FrameWork::newILPSystem(bool max) {
	return manager()->newILPSystem();
}


/**
 * This property, passed to the load configuration, gives the name of the
 * entry function of the current task.
 */
GenericIdentifier<CString> TASK_ENTRY("otawa.task_entry", "main");


/**
 * Identifier of the property indicating the name (CString) of the platform to use.
 */	
GenericIdentifier<CString> PLATFORM_NAME("otawa.platform_name", "");


/**
 * Identifier of the property indicating a name (CString) of the loader to use..
 */	
GenericIdentifier<CString> LOADER_NAME("otawa.laoder_name", "");


/**
 * Identifier of the property indicating a platform (Platform *) to use.
 */	
GenericIdentifier<hard::Platform *> PLATFORM("otawa.platform", 0);


/**
 * Identifier of the property indicating the loader to use.
 */	
GenericIdentifier<Loader *> LOADER("otawa.loader", 0);


/**
 * Identifier of the property indicating the identifier (PlatformId) of the loader to use.
 */	
GenericIdentifier<hard::Platform::Identification *> PLATFORM_IDENTFIER("otawa.platform_identifier", 0);


/**
 * Argument count as passed to the program (int).
 */	
GenericIdentifier<int> ARGC("otawa.argc", -1);


/**
 * Argument values as passed to the program (char **).
 */	
GenericIdentifier<char **> ARGV("otawa.argv", 0);


/**
 * Argument values as passed to the program (char **).
 */	
GenericIdentifier<char **> ENVP("otawa.envp", 0);


/**
 * This property defines the used the used simulator when a simulator is
 * needed to perform simulation.
 */
GenericIdentifier<sim::Simulator *> SIMULATOR("otawa.simulator", 0);


/**
 * This property is used to pass the cache configuration directly to the
 * platform.
 */
GenericIdentifier<hard::CacheConfiguration *> CACHE_CONFIG("otawa.cache_config", 0);


/**
 * This property is a hint to have an estimation of the pipeline depth.
 * It is better to look to the processor configuration in the patform.
 */
GenericIdentifier<int> PIPELINE_DEPTH("otawa.pipeline_depth", -1);

} // otawa
