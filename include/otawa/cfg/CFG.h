/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/cfg/CFG.h -- interface of CFG class.
 */
#ifndef OTAWA_CFG_CFG_H
#define OTAWA_CFG_CFG_H

#include <assert.h>
#include <elm/Collection.h>
#include <elm/genstruct/Vector.h>

namespace otawa {

// Classes
class BasicBlock;
class Code;

	
// CFG class
class CFG: public ProgObject, private elm::Collection<BasicBlock *> {
	Code *_code;
	BasicBlock *ent;

	virtual elm::IteratorInst<BasicBlock *> *visit(void) const;
	virtual elm::MutableCollection<BasicBlock *> *empty(void) const;

public:
	static id_t ID;
	
	// Iterator
	class BBIterator: public elm::IteratorInst<BasicBlock *> {
		elm::genstruct::Vector<BasicBlock *> bbs;
		int pos;
	public:
		inline BBIterator(const CFG *cfg);
		virtual bool ended(void) const;
		virtual BasicBlock *item(void) const;
		virtual void next(void);
	};
	
	// Methods
	CFG(Code *code, BasicBlock *entry);
	inline BasicBlock *entry(void) const;
	inline Code *code(void) const;
	String label(void);
	address_t address(void);
	inline elm::Collection<BasicBlock *>& bbs(void);
};


// Inlines
inline elm::Collection<BasicBlock *>& CFG::bbs(void) {
	return *this;
}

inline BasicBlock *CFG::entry(void) const {
	return ent;
}

inline Code *CFG::code(void) const {
	return _code;
};

inline CFG::BBIterator::BBIterator(const CFG *cfg) {
	assert(cfg);
	bbs.add(cfg->entry());
	pos = 0;
}

} // otawa

#endif // OTAWA_CFG_CFG_H
