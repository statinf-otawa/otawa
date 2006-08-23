/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/display/graphviz.h -- graphviz display module interface.
 */
#ifndef OTAWA_DISPLAY_GRAPHVIZ_H
#define OTAWA_DISPLAY_GRAPHVIZ_H

#include <otawa/display/Driver.h>

namespace otawa { namespace display {
	
// GraphViz driver
extern Driver& graphviz_driver;


// Layout property
typedef enum graphviz_layout_t {
	LAYOUT_DOT,
	LAYOUT_RADIAL,
	LAYOUT_CIRCULAR,
	LAYOUT_UNDIRECTED_NEATO,
	LAYOUT_UNDIRECTED_FDP
} graphviz_layout_t;
extern GenericIdentifier<int> GRAPHVIZ_LAYOUT;


// Output property
typedef enum graphviz_output_t {
	OUTPUT_TEXT,
	OUTPUT_PS,
	OUTPUT_PNG,
	OUTPUT_SVG
} graphviz_output_t;
extern GenericIdentifier<int> GRAPHVIZ_OUTPUT;


// File property
extern GenericIdentifier<elm::CString> GRAPHVIZ_FILE;

} } // otawa::display

#endif // OTAWA_DISPLAY_GRAPHVIZ_H
