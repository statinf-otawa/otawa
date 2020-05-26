/*
 *	$Id$
 *	FlowFactLoader class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-09, IRIT UPS.
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
#ifndef OTAWA_UTIL_FLOW_FACT_LOADER_H
#define OTAWA_UTIL_FLOW_FACT_LOADER_H
#include <otawa/flowfact/conflict.h>
#include <elm/data/Vector.h>
#include <elm/io.h>
#include <elm/string.h>
#include <elm/sys/Path.h>
#include <elm/types.h>
#include <elm/utility.h>
#include <otawa/base.h>
#include <otawa/dfa/State.h>
#include <otawa/flowfact/features.h>
#include <otawa/proc/Feature.h>
#include <otawa/prop/ContextualProperty.h>
#include <otawa/prop/Identifier.h>

// Externals
namespace otawa  { class FlowFactLoader; }
namespace elm { namespace xom { class Element; class Node; } }
int util_fft_parse(otawa::FlowFactLoader *loader);
void util_fft_error(otawa::FlowFactLoader *loader, const char *msg);

namespace otawa {

using namespace elm;

// Extern class
class ContextualPath;
class File;
class WorkSpace;

// FlowFactLoader abstract class
class FlowFactLoader: public Processor {
	friend int ::util_fft_parse(FlowFactLoader *loader);
	friend void ::util_fft_error(otawa::FlowFactLoader *loader, const char *msg);
public:
	static p::declare reg;
	FlowFactLoader(p::declare& r = reg);

protected:
	inline WorkSpace *workSpace(void) const { return _fw; }

	Address addressOf(const string& label);
	MemArea addressOf(const string& file, int line);
	void onError(const string& message);
	void onWarning(const string& message);

	virtual void onCheckSum(const String& name, t::uint32 sum);
	virtual void onLibrary(void);
	virtual void onLoop(address_t addr, int count, int total, int min, const ContextualPath& path);
	virtual void onReturn(address_t addr);
	virtual void onNoReturn(address_t addr);
	virtual void onNoReturn(String name);
	virtual void onNoCall(Address address);
	virtual void onNoInline(Address address, bool no_inline, const ContextualPath& path);
	virtual void onIgnoreSeq(Address address);
	virtual void onIgnoreControl(Address address);
	virtual void onMultiBranch(Address control, const Vector<Address>& target);
	virtual void onMultiCall(Address control, const Vector<Address>& target);
	virtual void onPreserve(Address address);
	virtual void onIgnoreEntry(string name);
	virtual void onInfeasablePath( address_t addr,const ContextualPath& path);  
    virtual int containsNotALL(xom::Element *element); 
	
	virtual void onForceBranch(Address address);
	virtual void onForceCall(Address address);

	virtual void onUnknownLoop(Address addr);
	virtual void onUnknownMultiBranch(Address control);
	virtual void onUnknownMultiCall(Address control);

	virtual void onMemoryAccess(Address iaddr, Address lo, Address hi, const ContextualPath& path);
	virtual void onRegSet(string name, const dfa::Value& value);
	virtual void onMemSet(Address addr, const Type *type, const dfa::Value& value);
	virtual void onRegSet(dfa::State* state, string name, const dfa::Value& value);
	virtual void onMemSet(dfa::State* state, Address addr, const Type *type, const dfa::Value& value);
	virtual void onSetInlining(Address address, bool policy, const ContextualPath& path);

	virtual void processWorkSpace(WorkSpace *ws);
	virtual void configure (const PropList &props);
	virtual void setup(WorkSpace *ws);

private:
	WorkSpace *_fw;
	bool checksummed;
	Vector<Path> paths;
	Vector<xom::Element *> nodes;
	Path current;
	bool mandatory;
	bool lines_available;
	dfa::State *state;
	bool lib;
	Vector<int> numListOfUnclosedPath; 
	int currentCteNum; 
	int numOfEdgeIntoCurrentCte; 
	bool intoConflictPath; 


	// F4 support
	void loadF4(const string& path);

	// XML support
	void load(WorkSpace *ws, const Path& path);
	void loadXML(const string& path);
	void scanXState(xom::Element *element);
	void scanXState(xom::Element *element, ContextualPath& path);
	void scanXLoop(xom::Element *element, ContextualPath& path);
	void scanXFun(xom::Element *element, ContextualPath& path);
	void scanXConditional(xom::Element *element, ContextualPath& path);
	MemArea scanAddress(xom::Element *element, ContextualPath& path, bool call = false);
	int findCall(cstring file, int line, Address& r);
	Option<long> scanInt(xom::Element *element, cstring name);
	Option<unsigned long> scanUInt(xom::Element *element, cstring name);
	Option<long> scanBound(xom::Element *element, cstring name);
	void scanXContent(xom::Element *element, ContextualPath& path);
	void scanXBody(xom::Element *element, ContextualPath& path);
	void scanXCall(xom::Element *element, ContextualPath& path);
	string xline(xom::Node *element);
	void scanNoInline(xom::Element *element, ContextualPath& cpath, bool no_inline);
	void scanIgnoreEntry(xom::Element *element);
	void scanMultiBranch(xom::Element *element, ContextualPath& cpath);
	void scanMultiCall(xom::Element *element, ContextualPath& cpath);
	void scanIgnoreControl(xom::Element *element, ContextualPath& cpath);
	void scanIgnoreSeq(xom::Element *element, ContextualPath& cpath);
	dfa::Value scanValue(xom::Element *element);
	void scanXNotAll(xom::Element *element, ContextualPath& cpath);  
	void scanEdge(xom::Element* edge,  ContextualPath& cpath  ); 
	void getQualifierAnd(xom::Element * element, Inst *inst, int *nbPath, bool nextLoop, ContextualPath& path, Vector< LoopOfConflict >  & infoLoop); 
	void scanMemAccess(xom::Element *element);
	void scanRegSet(xom::Element *element, dfa::State* state);
	void scanMemSet(xom::Element *element, dfa::State* state);
	void scanSetInlining(xom::Element *element, ContextualPath& cpath, bool policy);
};
} // otawa

#endif	// OTAWA_UTIL_FLOW_FACT_READER_H
