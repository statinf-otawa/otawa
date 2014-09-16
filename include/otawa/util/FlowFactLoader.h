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

#include <elm/string.h>
#include <elm/utility.h>
#include <elm/io.h>
#include <elm/system/Path.h>
#include <otawa/base.h>
#include <otawa/prop/Identifier.h>
#include <otawa/proc/Feature.h>
#include <elm/genstruct/Vector.h>
#include <otawa/prop/ContextualProperty.h>
#include <elm/types.h>
#include <otawa/dfa/State.h>

// Externals
namespace otawa  { class FlowFactLoader; }
namespace elm { namespace xom { class Element; class Node; } }
int util_fft_parse(otawa::FlowFactLoader *loader);
void util_fft_error(otawa::FlowFactLoader *loader, const char *msg);

namespace otawa {

using namespace elm;
using namespace elm::genstruct;

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
	MemArea addressOf(const string& file, int line) throw(ProcessorException);
	void onError(const string& message);
	void onWarning(const string& message);

	virtual void onCheckSum(const String& name, t::uint32 sum);
	virtual void onLoop(
		address_t addr,
		int count,
		int total,
		const ContextualPath& path);
	virtual void onReturn(address_t addr);
	virtual void onNoReturn(address_t addr);
	virtual void onNoReturn(String name);
	virtual void onNoCall(Address address);
	virtual void onIgnoreSeq(Address address);
	virtual void onIgnoreControl(Address address);
	virtual void onMultiBranch(Address control, const Vector<Address>& target);
	virtual void onMultiCall(Address control, const Vector<Address>& target);
	virtual void onPreserve(Address address);
	virtual void onIgnoreEntry(string name);

	virtual void onUnknownLoop(Address addr);
	virtual void onUnknownMultiBranch(Address control);
	virtual void onUnknownMultiCall(Address control);

	virtual void onMemoryAccess(Address iaddr, Address lo, Address hi, const ContextualPath& path);
	virtual void onRegSet(string name, const dfa::Value& value);
	virtual void onMemSet(Address addr, const Type *type, const dfa::Value& value);

	virtual void processWorkSpace(WorkSpace *ws);
	virtual void configure (const PropList &props);
	virtual void setup(WorkSpace *ws);

private:
	WorkSpace *_fw;
	bool checksummed;
	genstruct::Vector<Path> paths;
	genstruct::Vector<xom::Element *> nodes;
	Path current;
	bool mandatory;
	bool lines_available;
	dfa::State *state;

	// F4 support
	void loadF4(const string& path) throw(ProcessorException);

	// XML support
	void load(WorkSpace *ws, const Path& path);
	void loadXML(const string& path) throw(ProcessorException);
	void scanXLoop(xom::Element *element, ContextualPath& path) throw(ProcessorException);
	void scanXFun(xom::Element *element, ContextualPath& path) throw(ProcessorException);
	void scanXConditional(xom::Element *element, ContextualPath& path) throw(ProcessorException);
	MemArea scanAddress(xom::Element *element, ContextualPath& path) throw(ProcessorException);
	Option<long> scanInt(xom::Element *element, cstring name) throw(ProcessorException);
	Option<unsigned long> scanUInt(xom::Element *element, cstring name) throw(ProcessorException);
	Option<long> scanBound(xom::Element *element, cstring name) throw(ProcessorException);
	void scanXContent(xom::Element *element, ContextualPath& path) throw(ProcessorException);
	void scanXBody(xom::Element *element, ContextualPath& path) throw(ProcessorException);
	void scanXCall(xom::Element *element, ContextualPath& path) throw(ProcessorException);
	string xline(xom::Node *element);
	void scanIgnoreEntry(xom::Element *element);
	void scanMultiBranch(xom::Element *element, ContextualPath& cpath);
	void scanMultiCall(xom::Element *element, ContextualPath& cpath);
	dfa::Value scanValue(xom::Element *element);
	void scanMemAccess(xom::Element *element);
	void scanRegSet(xom::Element *element);
	void scanMemSet(xom::Element *element);
};

// Properties
extern Identifier<Path> FLOW_FACTS_PATH;
extern Identifier<xom::Element *> FLOW_FACTS_NODES;
extern Identifier<bool> FLOW_FACTS_MANDATORY;

// Features
extern Feature<FlowFactLoader> FLOW_FACTS_FEATURE;
extern Feature<FlowFactLoader> MKFF_PRESERVATION_FEATURE;

// Annotations
extern Identifier<bool> IS_RETURN;
extern Identifier<bool> NO_RETURN;
extern Identifier<int> MAX_ITERATION;
extern Identifier<int> MIN_ITERATION;
extern Identifier<int> TOTAL_ITERATION;
extern Identifier<bool> NO_CALL;
extern Identifier<bool> IGNORE_CONTROL;
extern Identifier<bool> IGNORE_SEQ;
extern Identifier<Address> BRANCH_TARGET;
extern Identifier<bool> PRESERVED;
extern Identifier<bool> IGNORE_ENTRY;
extern Identifier<Address> CALL_TARGET;
extern Identifier<Pair<Address, Address> > ACCESS_RANGE;

} // otawa

#endif	// OTAWA_UTIL_FLOW_FACT_READER_H
