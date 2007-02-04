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
#include <otawa/cfg/BasicBlock.h>

namespace otawa {

// Classes
class BasicBlock;
class CodeItem;
class CFG;

// Properties
extern Identifier<CFG *> ENTRY;
extern Identifier<int> INDEX;

// CFG class
class CFG: public ProgObject, private elm::Collection<BasicBlock *> {
	CodeItem *_code;
	BasicBlock *ent;

	virtual elm::IteratorInst<BasicBlock *> *visit(void);
	virtual elm::MutableCollection<BasicBlock *> *empty(void);
protected:
	unsigned long flags;
	EndBasicBlock _entry, _exit;
	static const unsigned long FLAG_Scanned = 0x01;
	static const unsigned long FLAG_Virtual = 0x02;
	static const unsigned long FLAG_Inlined = 0x04;
	genstruct::Vector<BasicBlock *> _bbs;
	virtual void scan(void);
public:
	//static Identifier ID_Dom;
	
	// Iterator
	class BBIterator: public elm::PreIterator<BBIterator, BasicBlock *> {
		elm::genstruct::Vector<BasicBlock *>& bbs;
		int pos;
	public:
		inline BBIterator(CFG *cfg);
		inline bool ended(void) const;
		inline BasicBlock *item(void) const;
		inline void next(void);
	};
	
	// Methods
	CFG(void);
	CFG(CodeItem *code, BasicBlock *entry);
	inline CodeItem *code(void) const;
	String label(void);
	address_t address(void);
	inline elm::Collection<BasicBlock *>& bbs(void);
	inline BasicBlock *entry(void);
	inline BasicBlock *exit(void);
	inline int countBB(void);
	bool dominates(BasicBlock *bb1, BasicBlock *bb2);
	inline bool isVirtual(void) const;
	inline bool isInlined(void) const;
	void numberBB(void);
};


// CFG inlines
inline elm::Collection<BasicBlock *>& CFG::bbs(void) {
	return *this;
}

inline CodeItem *CFG::code(void) const {
	return _code;
};

inline BasicBlock *CFG::entry(void) {
	if(!(flags & FLAG_Scanned))
		scan();
	return &_entry;
}

inline BasicBlock *CFG::exit(void) {
	if(!(flags & FLAG_Scanned))
		scan();
	return &_exit;
}
inline int CFG::countBB(void) {
	if(!(flags & FLAG_Scanned))
		scan();
	return _bbs.length();
}
inline bool CFG::isVirtual(void) const {
	return flags & FLAG_Virtual;
}
inline bool CFG::isInlined(void) const {
	return flags & FLAG_Inlined;
}

// BBIterator inlines
inline CFG::BBIterator::BBIterator(CFG *cfg): bbs(cfg->_bbs), pos(0) {
	if(!(cfg->flags & FLAG_Scanned))
		cfg->scan();
};
inline bool CFG::BBIterator::ended(void) const {
	return pos >= bbs.length();
};
inline BasicBlock *CFG::BBIterator::item(void) const {
	return bbs[pos];
};
inline void CFG::BBIterator::next(void) {
	pos++;
};

// Property display
template <>
void Identifier<CFG *>::print(elm::io::Output& out, const Property& prop) const;

} // otawa

#endif // OTAWA_CFG_CFG_H
