/*
 * $Id$
 * Copyright (c) 2005, IRIT-UPS
 *
 * otawa/util/FlowFactLoader.h -- interface of FlowFactReader class.
 */
#ifndef OTAWA_UTIL_FLOW_FACT_LOADER_H
#define OTAWA_UTIL_FLOW_FACT_LOADER_H

#include <elm/string.h>
#include <elm/utility.h>
#include <elm/io.h>
#include <otawa/base.h>

// Externals
namespace otawa  {
	class FlowFactLoader;
} // otawa
int util_fft_parse(otawa::FlowFactLoader *loader);
void util_fft_error(otawa::FlowFactLoader *loader, const char *msg);

namespace otawa {

// Extern class
class FrameWork;

// FlowFactLoader abstract class
class FlowFactLoader {
	friend int ::util_fft_parse(FlowFactLoader *loader);
	friend void ::util_fft_error(otawa::FlowFactLoader *loader, const char *msg);
protected:
	virtual void onError(const char *fmt, ...) = 0;
	virtual void onLoop(address_t addr, int count) = 0;
public:
	void run(FrameWork *fw);
};

} // otawa

#endif	// OTAWA_UTIL_FLOW_FACT_READER_H
