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
extern GenericIdentifier<int> GRAPHVIZ_LAYOUT;
typedef enum graphviz_layout_t {
	LAYOUT_DOT,
	LAYOUT_RADIAL,
	LAYOUT_CIRCULAR
} graphviz_layout_t;


// Output property
extern GenericIdentifier<int> GRAPHVIZ_OUTPUT;
typedef enum graphviz_output_t {
	OUTPUT_TEXT,
	OUTPUT_PS,
	OUTPUT_PNG,
	OUTPUT_SVG,
	OUTPUT_PDF
} graphviz_output_t;


// File property
extern GenericIdentifier<String&> GRAPHVIZ_FILE;

} } // otawa::display

#endif // OTAWA_DISPLAY_GRAPHVIZ_H
