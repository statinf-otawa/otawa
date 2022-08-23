/*
 *	CFGProvider class interface.
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2022, IRIT UPS.
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
#ifndef OTAWA_CFG_CFGPROVIDER_H
#define OTAWA_CFG_CFGPROVIDER_H

#include <otawa/cfg/features.h>
#include <otawa/view/features.h>

namespace otawa {

class DisassemblyView;
class SourceView;
class KindView;
class RegisterView;
class SemView;
	
class CFGProvider: public Processor {
public:
	static p::declare reg;
	CFGProvider(p::declare& r = reg);
	void *interfaceFor(const AbstractFeature& feature) override;
protected:
	void commit(WorkSpace *ws) override;
	void destroy(WorkSpace *ws) override;
	inline CFGCollection *collection() { return coll; }
	void setCollection(CFGCollection *collection);
private:
	CFGCollection *coll;
	DisassemblyView *dview;
	SourceView *sview;
	KindView *kview;
	RegisterView *rview;
	SemView *seview;
};

}	// otawa

#endif	// OTAWA_CFG_CFGPROVIDER_H
