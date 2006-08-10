/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/display/Driver.h -- display::Driver class.
 */
#ifndef OTAWA_DISPLAY_DRIVER_H
#define OTAWA_DISPLAY_DRIVER_H

#include <otawa/display/Graph.h>

namespace otawa { namespace display {

// Driver class
class Driver {
public:
	Graph *newGraph(const PropList& props) const = 0;
};

} } // otawa::display

#endif // OTAWA_DISPLAY_DRIVER_H
