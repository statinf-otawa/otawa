/*
 *	$Id$
 *	BBProcessor class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-10, IRIT UPS.
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
#ifndef OTAWA_PROC_BBPROCESSOR_H
#define OTAWA_PROC_BBPROCESSOR_H

#include <otawa/cfg/BasicBlock.h>
#include <otawa/proc/CFGProcessor.h>

namespace otawa {

using namespace elm;

// BBCleaner class
class BBCleaner: public Cleaner {
public:
	BBCleaner(WorkSpace *_ws): ws(_ws) { }
	virtual void clean(void);
protected:
	virtual void clean(WorkSpace *ws, CFG *cfg, BasicBlock *bb) = 0;
private:
	WorkSpace *ws;
};


// BBRemover class
template <class T>
class BBRemover: public BBCleaner {
public:
	inline BBRemover(WorkSpace *ws, Identifier<T>& identifier)
		: BBCleaner(ws), id(identifier) { }
protected:
	Identifier<T>& id;
	virtual void clean(WorkSpace *ws, CFG *cfg, BasicBlock *bb)
		{ bb->removeProp(id); }
};


// BBDeletor class
template <class T>
class BBDeletor: public BBRemover<T> {
public:
	inline BBDeletor(WorkSpace *ws, Identifier<T *>& identifier)
		: BBRemover<T>(ws), id(identifier) { }
protected:
	Identifier<T *>& id;
	virtual void clean(WorkSpace *ws, CFG *cfg, BasicBlock *bb)
		{ T *p = id(bb); if(p) delete p; BBRemover<T>::clean(ws, cfg, bb); }
};


// BBProcessor class
class BBProcessor: public CFGProcessor {
protected:
	virtual void processCFG(WorkSpace *ws, CFG *cfg);
	virtual void processBB(WorkSpace *ws, CFG *cfd, BasicBlock *bb) = 0;
	virtual void cleanupCFG(WorkSpace *ws, CFG *cfg);
	virtual void cleanupBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb);
	
public:
	BBProcessor(void);
	inline BBProcessor(AbstractRegistration& reg): CFGProcessor(reg) { }
	inline BBProcessor(cstring name, const Version& version, AbstractRegistration& reg)
		: CFGProcessor(name, version, reg) { }
	BBProcessor(cstring name, elm::Version version = elm::Version::ZERO);

protected:
	template <class T> void trackBB(const AbstractFeature& feature, const Identifier<T *>& id)
		{ addCleaner(feature, new BBDeletor<T>(workspace(), id)); }
	template <class T> void trackBB(const AbstractFeature& feature, const Identifier<T>& id)
		{ addCleaner(feature, new BBRemover<T>(workspace(), id)); }
};

} // otawa

#endif	// OTAWA_PROC_BBPROCESSOR_H
