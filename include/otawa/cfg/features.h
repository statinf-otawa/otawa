/*
 *	$Id$
 *	features of module cfg
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#ifndef OTAWA_CFG_FEATURES_H_
#define OTAWA_CFG_FEATURES_H_

#include <otawa/proc/SilentFeature.h>
#include <elm/genstruct/FragTable.h>

namespace elm { namespace genstruct { template <class T> class Tree; } }

namespace otawa {

// Pre-declarations
class BasicBlock;
class CFG;
class CFGCollector;
class CFGInfo;
class Edge;
class LoopUnroller;
namespace pfg { class PFG; class BB; }
class SESERegion;
typedef elm::genstruct::Tree<SESERegion*> PSTree;

// CFGCollection Class
class CFGCollection {
	friend class CFGCollector;
	friend class LoopUnroller;
	elm::genstruct::FragTable<CFG *> cfgs;
public:
	inline int count(void) const { return cfgs.length(); }
	inline CFG *get(int index) const { return cfgs[index]; }
	inline CFG *operator[](int index) const { return cfgs[index]; }

	class Iterator: public elm::genstruct::FragTable<CFG *>::Iterator {
	public:
		inline Iterator(const CFGCollection *cfgs)
			: elm::genstruct::FragTable<CFG *>::Iterator(cfgs->cfgs) { }
		inline Iterator(const CFGCollection& cfgs)
			: elm::genstruct::FragTable<CFG *>::Iterator(cfgs.cfgs) { }
	};
};

// PFG_FEATURE
extern SilentFeature PFG_FEATURE;
extern Identifier<pfg::PFG *> PFG;
extern Identifier<pfg::BB *> PFG_BB;

// COLLECTED_CFG_FEATURE
extern SilentFeature COLLECTED_CFG_FEATURE;
extern Identifier<CFGCollection *> INVOLVED_CFGS;

// CFGInfoFeature
extern SilentFeature CFG_INFO_FEATURE;
extern Identifier<CFGInfo *> CFG_INFO;

// REDUCED_LOOPS_FEATURE
extern SilentFeature REDUCED_LOOPS_FEATURE;

// UNROLLED_LOOPS_FEATURE
extern SilentFeature UNROLLED_LOOPS_FEATURE;
extern Identifier<BasicBlock*> UNROLLED_FROM;

// PST_FEATURE
extern SilentFeature PST_FEATURE;
extern Identifier<BasicBlock*> FROM_BB;
extern Identifier<Edge *> FROM_EDGE;
extern Identifier<PSTree *> PROGRAM_STRUCTURE_TREE;

// VIRTUALIZED_CFG_FEATURE
extern SilentFeature VIRTUALIZED_CFG_FEATURE;
extern Identifier<bool> VIRTUAL_INLINING;
extern Identifier<BasicBlock*> VIRTUAL_RETURN_BLOCK;

// CFG_CHECKSUM_FEATURE
extern SilentFeature CFG_CHECKSUM_FEATURE;
extern Identifier<unsigned long > CHECKSUM;

} // otawa

#endif /* OTAWA_CFG_FEATURES_H_ */
