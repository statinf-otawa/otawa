/*
 *	$Id$
 *	CFG class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-07, IRIT UPS.
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
#ifndef OTAWA_CFG_CFG_H
#define OTAWA_CFG_CFG_H

#include <assert.h>
#include <elm/Collection.h>
#include <elm/genstruct/FragTable.h>
#include <otawa/cfg/BasicBlock.h>

namespace otawa {

// Classes
class BasicBlock;
class CodeItem;
class CFG;
class CFGInfo;

// Properties
extern Identifier<CFG *> ENTRY;
extern Identifier<int> INDEX;

// CFG class
class CFG: public PropList, private elm::Collection<BasicBlock *> {
	Segment *_seg;
	BasicBlock *ent;

	virtual elm::IteratorInst<BasicBlock *> *visit(void);
	virtual elm::MutableCollection<BasicBlock *> *empty(void);
	typedef genstruct::FragTable<BasicBlock *> bbs_t;
protected:
	friend class CFGInfo;
	virtual ~CFG(void);
	unsigned long flags;
	EndBasicBlock _entry, _exit;
	static const unsigned long FLAG_Scanned = 0x01;
	static const unsigned long FLAG_Virtual = 0x02;
	static const unsigned long FLAG_Inlined = 0x04;
	bbs_t _bbs;
	virtual void scan(void);
	inline const bbs_t& __bbs(void) {
		if(!(flags & FLAG_Scanned))
			scan();
		return _bbs;
	}
public:
	//static Identifier ID_Dom;
	
	// Iterator
	class BBIterator: public bbs_t::Iterator {
	public:
		inline BBIterator(CFG *cfg): bbs_t::Iterator(cfg->__bbs()) { }
		inline BBIterator(const BBIterator& iter): bbs_t::Iterator(iter) { }
	};
	
	// Methods
	CFG(void);
	CFG(Segment *seg, BasicBlock *entry);
	inline Segment *segment(void) const;
	String label(void);
	inline int number(void);
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

inline Segment *CFG::segment(void) const {
	return _seg;
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

inline int CFG::number(void) {
	return(INDEX(this));
}

inline bool CFG::isVirtual(void) const {
	return flags & FLAG_Virtual;
}
inline bool CFG::isInlined(void) const {
	return flags & FLAG_Inlined;
}

// Property display
template <>
void Identifier<CFG *>::print(elm::io::Output& out, const Property& prop) const;

} // otawa

#endif // OTAWA_CFG_CFG_H
