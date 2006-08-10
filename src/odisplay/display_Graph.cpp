/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	src/odisplay/display_Graph.cpp -- display::Graph class implementation.
 */

#include <otawa/display/Graph.h>

/**
 * @page odisplay ODisplay Library
 * 
 * This library is delivered with OTAWA to provide graphical display
 * of managed graphs.
 * 
 * The main principle is to provides the same interface
 * to each graph application whatever the required final output using a
 * display driver system.
 * 
 * Using abstract graph description using classes
 * @ref otawa::display::Graph, @ref otawa::display::Node or
 * @ref otawa::display::Edge, the applications describes the graph and the
 * driver is responsible for outputting the graph in the user-chosen
 * form (vector graphics, bitmap, screen display and so on).
 */
