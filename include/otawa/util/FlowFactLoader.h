/*
 * $Id$
 * Copyright (c) 2005-07, IRIT-UPS
 *
 * FlowFactReader class interface
 */
#ifndef OTAWA_UTIL_FLOW_FACT_LOADER_H
#define OTAWA_UTIL_FLOW_FACT_LOADER_H

#include <elm/string.h>
#include <elm/utility.h>
#include <elm/io.h>
#include <elm/system/Path.h>
#include <otawa/base.h>
#include <otawa/prop/Identifier.h>

// Externals
namespace otawa  {
	class FlowFactLoader;
} // otawa
int util_fft_parse(otawa::FlowFactLoader *loader);
void util_fft_error(otawa::FlowFactLoader *loader, const char *msg);

namespace otawa {

using namespace elm;
using namespace elm;

// Extern class
class File;
class WorkSpace;

// FlowFactLoader abstract class
class FlowFactLoader {
	friend int ::util_fft_parse(FlowFactLoader *loader);
	friend void ::util_fft_error(otawa::FlowFactLoader *loader, const char *msg);
	WorkSpace *_fw;
	bool _verbose;
	bool checksummed;
	void onCheckSum(const String& name, unsigned long sum);
protected:
	FlowFactLoader(void);
	virtual void onError(const char *fmt, ...) = 0;
	virtual void onWarning(const char *fmt, ...) = 0;
	virtual void onLoop(address_t addr, int count) = 0;
public:
	void run(WorkSpace *fw, elm::system::Path path = "", bool verbose = false);
};

// Properties
extern Identifier<elm::system::Path> FLOW_FACTS_PATH;

} // otawa

#endif	// OTAWA_UTIL_FLOW_FACT_READER_H
