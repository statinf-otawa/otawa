/*
 *	WorkSpace class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007-8, IRIT UPS.
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
 *	This file is part of OTAWA
 *	Copyright (c) 2007-08, IRIT UPS.
 */
#ifndef OTAWA_PROG_WORK_SPACE_H
#define OTAWA_PROG_WORK_SPACE_H

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
namespace ast {
	class AST;
	class ASTInfo;
}
class CFG;
class CFGInfo;
class File;
class Inst;
class Loader;
class Manager;
class Process;
namespace hard {
	class CacheConfiguration;
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
public:
	WorkSpace(Process *_proc);
	WorkSpace(const WorkSpace *ws);
	~WorkSpace(void);
	inline Process *process(void) const { return &proc; };

	// Process overload
	virtual hard::Platform *platform(void) { return proc->platform(); };
	virtual Manager *manager(void) { return proc->manager(); };
	virtual Inst *start(void) { return proc->start(); };
	virtual Inst *findInstAt(address_t addr) { return proc->findInstAt(addr); };
	string format(Address addr, bool with_address = true);

	// Configuration services
	elm::xom::Element *config(void);
	void loadConfig(const elm::system::Path& path);

	// Feature management
	void require(const AbstractFeature& feature, const PropList& props = PropList::EMPTY);
	void provide(const AbstractFeature& feature, const Vector<const AbstractFeature*> *required = NULL);
	bool isProvided(const AbstractFeature& feature);
	void remove(const AbstractFeature& feature);
	void invalidate(const AbstractFeature& feature);
	FeatureDependency* getDependency(const AbstractFeature* feature);

	// cancellation management
	inline void clearCancellation(void) { cancelled = false; }
	inline void cancel(void) { cancelled = true; }
	inline bool isCancelled(void) const { return cancelled; }

	// serialization
	virtual void serialize(elm::serial2::Serializer& serializer);
	virtual void unserialize(elm::serial2::Unserializer& unserializer);

	// deprecated
	ast::ASTInfo *getASTInfo(void);
	ilp::System *newILPSystem(bool max = true);
	CFGInfo *getCFGInfo(void);
	CFG *getStartCFG(void);
	virtual const hard::CacheConfiguration& cache(void);

private:
	void newFeatDep(const AbstractFeature* feature);
	bool hasFeatDep(const AbstractFeature* feature);
	void delFeatDep(const AbstractFeature* feature);

private:
	AutoPtr<Process> proc;
	typedef HashTable<const AbstractFeature*, FeatureDependency*> feat_map_t;
	feat_map_t featMap;
	bool cancelled;
};

};	// otawa

#endif	// OTAWA_PROG_WORK_SPACE_H
