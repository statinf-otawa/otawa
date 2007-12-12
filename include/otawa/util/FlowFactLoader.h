/*
 *	$Id$
 *	FlowFactLoader class interface
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

// Externals
namespace otawa  {
	class FlowFactLoader;
} // otawa
int util_fft_parse(otawa::FlowFactLoader *loader);
void util_fft_error(otawa::FlowFactLoader *loader, const char *msg);

namespace otawa {

using namespace elm;
using namespace elm::genstruct;

// Extern class
class File;
class WorkSpace;

// FlowFactLoader abstract class
class FlowFactLoader: public Processor {
	friend int ::util_fft_parse(FlowFactLoader *loader);
	friend void ::util_fft_error(otawa::FlowFactLoader *loader, const char *msg);
	WorkSpace *_fw;
	bool checksummed;
	String path;
	bool mandatory;
protected:
	inline WorkSpace *workSpace(void) const { return _fw; }
	
	Address addressOf(const string& label);
	void onError(const string& message);
	void onWarning(const string& message);
	
	virtual void onCheckSum(const String& name, unsigned long sum);
	virtual void onLoop(address_t addr, int count);
	virtual void onReturn(address_t addr);
	virtual void onNoReturn(address_t addr);
	virtual void onNoReturn(String name);
	virtual void onNoCall(Address address);
	virtual void onIgnoreControl(Address address);
	virtual void onMultiBranch(Address control, const Vector<Address>& target);
	virtual void onPreserve(Address address);
	
	virtual void onUnknownLoop(Address addr);
	virtual void onUnknownMultiBranch(Address control);
	
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void configure (const PropList &props);
	
	FlowFactLoader(const string& name, const Version& version);
public:
	FlowFactLoader(void);
};

// Properties
extern Identifier<elm::system::Path> FLOW_FACTS_PATH;
extern Identifier<bool> FLOW_FACTS_MANDATORY;

// Features
extern Feature<FlowFactLoader> FLOW_FACTS_FEATURE;
extern Feature<FlowFactLoader> MKFF_PRESERVATION_FEATURE;

// Annotations
extern Identifier<bool> IS_RETURN;
extern Identifier<bool> NO_RETURN;
extern Identifier<int> MAX_ITERATION;
extern Identifier<bool> NO_CALL;
extern Identifier<bool> IGNORE_CONTROL;
extern Identifier<Address> BRANCH_TARGET;
extern Identifier<bool> PRESERVED;

} // otawa

#endif	// OTAWA_UTIL_FLOW_FACT_READER_H
