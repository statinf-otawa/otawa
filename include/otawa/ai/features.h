/*
 *	features of otawa::ai module
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2018, IRIT UPS.
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
#ifndef INCLUDE_OTAWA_AI_FEATURES_H_
#define INCLUDE_OTAWA_AI_FEATURES_H_

#include <otawa/proc/Feature.h>

namespace otawa {

class Block;

namespace ai {

// CFG ranking
class CFGRanking {
public:
	virtual ~CFGRanking();
	virtual int rankOf(Block *v) = 0;
};
extern p::interfaced_feature<CFGRanking> CFG_RANKING_FEATURE;


// deprecated
class PropertyRanking {
public:
	int rankOf(const PropList& props);
	inline int rankOf(const PropList *props) { return rankOf(*props); }
};

extern p::id<int> RANK_OF;
extern p::feature RANKING_FEATURE;

} }		// otawa::ai

#endif /* INCLUDE_OTAWA_AI_FEATURES_H_ */
