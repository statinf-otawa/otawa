/*
 *	CFGNormalizer class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-07, IRIT UPS.
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
#ifndef OTAWA_UTIL_CFG_NORMALIZER_H
#define OTAWA_UTIL_CFG_NORMALIZER_H

#include <otawa/cfg/CFGTransformer.h>
#include <otawa/proc/Feature.h>

namespace otawa {
	
// CFGNormalizer class
class CFGNormalizer: public CFGTransformer {
public:

	static p::declare reg;
	static Identifier<bool> FORCE;

	CFGNormalizer(p::declare& r = reg);
	virtual void configure(const PropList& props = PropList::EMPTY);

protected:
	virtual void transform(CFG *g, CFGMaker& m);

private:
	bool force;
};

// Feature
extern p::feature NORMALIZED_CFGS_FEATURE;

} // otawa

#endif	// OTAWA_UTIL_CFG_NORMALIZER_H
