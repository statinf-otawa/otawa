/*
 *	$Id$
 *	Copyright (c) 2006 IRIT - UPS.
 *
 *	otawa/cfg/CollectCFG.h -- interface of CFGCollector class.
 */
#ifndef OTAWA_CFG_CFG_COLLECTOR_H
#define OTAWA_CFG_CFG_COLLECTOR_H

#include <elm/genstruct/FragTable.h>
#include <elm/genstruct/Vector.h>
#include <otawa/proc/Processor.h>
#include <otawa/proc/Feature.h>

namespace otawa {

// External Classes
class CFG;

// CFGCollection Class
class CFGCollection {
	friend class CFGCollector;
	friend class LoopUnroller;
	elm::genstruct::FragTable<CFG *> cfgs;
public:
	inline int count(void) const;
	inline CFG *get(int index) const;
	inline CFG *operator[](int index) const;
	
	class Iterator: public elm::genstruct::FragTable<CFG *>::Iterator {
	public:
		inline Iterator(const CFGCollection *cfgs);
		inline Iterator(const CFGCollection& cfgs);
	};
};

// CFGCollector Class
class CFGCollector: public Processor {
	elm::genstruct::Vector<CFG *> added_cfgs;
	elm::genstruct::Vector<CString> added_funs;
public:
	CFGCollector(void);
	virtual void configure(const PropList& props);
	virtual void cleanup(WorkSpace *ws);

	// Configuration
	static Identifier<CFG *> ADDED_CFG;
	static Identifier<CString> ADDED_FUNCTION;

protected:
	void processWorkSpace(WorkSpace *fw);
private:
	CString name;
	CFG *entry;
	bool rec;
};

// Features
extern Feature<CFGCollector> COLLECTED_CFG_FEATURE;

// Properties
extern Identifier<CFGCollection *> INVOLVED_CFGS;


// CFGCollection Inlines
inline int CFGCollection::count(void) const {
	return cfgs.length();
}

inline CFG *CFGCollection::get(int index) const {
	return cfgs[index];
}

inline CFG *CFGCollection::operator[](int index) const {
	return cfgs[index];
}


// CFGCollection::Iterator Inlines	
inline CFGCollection::Iterator::Iterator(const CFGCollection *cfgs)
: elm::genstruct::FragTable<CFG *>::Iterator(cfgs->cfgs) {
}

inline CFGCollection::Iterator::Iterator(const CFGCollection& cfgs)
: elm::genstruct::FragTable<CFG *>::Iterator(cfgs.cfgs) {
}

} // otawa

#endif // OTAWA_CFG_CFG_COLLECTOR_H
