/*
 *	$Id$
 *	Copyright (c) 2006 IRIT - UPS.
 *
 *	otawa/cfg/CollectCFG.h -- interface of CFGCollector class.
 */
#ifndef OTAWA_CFG_CFG_COLLECTOR_H
#define OTAWA_CFG_CFG_COLLECTOR_H

#include <elm/genstruct/FragTable.h>
#include <otawa/proc/Processor.h>
#include <otawa/proc/Feature.h>

namespace otawa {

// External Classes
class CFG;

// CFGCollection Class
class CFGCollection {
	friend class CFGCollector;
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
protected:
	void processFrameWork(FrameWork *fw);
public:
	CFGCollector(void);
};

// Features
extern Feature<CFGCollector> COLLECTED_CFG_FEATURE;

// Properties
extern GenericIdentifier<CFGCollection *> INVOLVED_CFGS;


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
