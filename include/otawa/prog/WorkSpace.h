/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS.
 *
 *	WorkSpace class interface
 */
#ifndef OTAWA_PROG_WORK_SPACE_H
#define OTAWA_PROG_WORK_SPACE_H

#include <elm/Collection.h>
#include <elm/system/Path.h>
#include <elm/genstruct/Vector.h>
#include <otawa/properties.h>
#include <otawa/prog/Process.h>

namespace elm { namespace xom {
	class Element;
} } // elm::xom

namespace otawa {

using namespace elm;
using namespace elm::genstruct;

// Classes
class AbstractFeature;
class FeatureDependency;
class AST;
class ASTInfo;
class CFG;
class CFGInfo;
class File;
class Inst;
class Loader;
class Manager;
namespace hard {
	class Platform;
	class Processor;
}
namespace ilp {
	class System;
}
namespace sim {
	class Simulator;
}

// WorkSpace class
class WorkSpace: public PropList {
	Process *proc;
	Vector<const AbstractFeature *> features;
	HashTable<const AbstractFeature*, FeatureDependency*> featMap;
protected:
	virtual Property *getDeep(const AbstractIdentifier *id)
		{ return proc->getProp(id); };
public:
	WorkSpace(Process *_proc);
	WorkSpace(const WorkSpace *ws);
	~WorkSpace(void);
	inline Process *process(void) const { return proc; };
	
	// Process overload
	virtual hard::Platform *platform(void) { return proc->platform(); };
	virtual Manager *manager(void) { return proc->manager(); };
	virtual Inst *start(void) { return proc->start(); };
	virtual Inst *findInstAt(address_t addr) { return proc->findInstAt(addr); };
	virtual const hard::CacheConfiguration& cache(void);
	
	// CFG Management
	CFGInfo *getCFGInfo(void);
	CFG *getStartCFG(void);
	
	// AST Management
	ASTInfo *getASTInfo(void);
	
	// ILP support
	ilp::System *newILPSystem(bool max = true);

	// Configuration services
	elm::xom::Element *config(void);
	void loadConfig(const elm::system::Path& path);
	
	// Feature management
	void require(const AbstractFeature& feature, const PropList& props = PropList::EMPTY);
	void provide(const AbstractFeature& feature, const Vector<const AbstractFeature*> *required = NULL);
	bool isProvided(const AbstractFeature& feature);
	void remove(const AbstractFeature& feature);
	void invalidate(const AbstractFeature& feature);
	
	// Feature dependency graph management
	FeatureDependency* getGraph(const AbstractFeature* feature);
	void newGraph(const AbstractFeature* feature);
	bool hasGraph(const AbstractFeature* feature);
	void delGraph(const AbstractFeature* feature);
};

};	// otawa

#endif	// OTAWA_PROG_WORK_SPACE_H
