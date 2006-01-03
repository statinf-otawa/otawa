/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/util/TrivialLBlockTime.h -- interface for TrivialLBlockTime class.
 */
#ifndef OTAWA_UTIL_TRIVIALLBLOCKTIME_H
#define OTAWA_UTIL_TRIVIALLBLOCKTIME_H

#include <otawa/proc/BBProcessor.h>

namespace otawa {

// TrivialBBLBlockTime class
class TrivialBBLBlockTime: public BBProcessor {
	int itime;
	void init(const PropList& props);
public:
	static Identifier ID_InstTime;

	TrivialBBLBlockTime(const PropList& props);

};

} // otawa


#endif // OTAWA_UTIL_TRIVIALLBLOCKTIME_H
