/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	oshell.h -- OShell cursor classes implementation.
 */

#include <stdlib.h>
#include <elm/genstruct/DLList.h>
#include <elm/genstruct/SortedBinTree.h>
#include <elm/debug.h>
#include <otawa/cfg.h>
#include "oshell.h"
#include "FrameWorkCursor.h"


namespace otawa {

// BasicBlock comparator
class BasicBlockComparator: public elm::Comparator<BasicBlock *> {
public:
	virtual int compare(BasicBlock *v1, BasicBlock *v2) {
		return v1->address() - v2->address();
	}
	static BasicBlockComparator comp;
};
BasicBlockComparator BasicBlockComparator::comp;
Comparator<BasicBlock *>& Comparator<BasicBlock *>::def = otawa::BasicBlockComparator::comp;
	
/**
 * @class GoException
 * Exception thrown when a "go" action is attempted and the target item
 * does not exists.
 */

// Overloaded
String GoException::message(void) {
	return "Target item does not exist.";
}


/**
 * @class BackException
 * Exception thrown when a "back" action is performed on a cursor already
 * at the top-level.
 */

// Overloaded
String BackException::message(void) {
	return "Current cursor is already at the top level.";
}


/**
 * @class PerformException
 * Exception thrown when a performed command does not exists in the current
 * context or there is a syntax error.
 */

/**
 * @fn PerformException::PerformException(CString message): msg(message);
 * Build a new perform exception.
 * @param message	Message of the exception.
 */


/**
 * @class Cursor
 * Abstract interface for objects providing navigation in Otawa system.
 */

/**
 * Get the cursor associated with the given framework.
 * @param fw	Framework to get the cursor for.
 * @return			Cursor on this framework.
 */
Cursor *Cursor::get(FrameWork *fw) {
	return new FrameWorkCursor(fw);
}

/**
 * @fn Cursor::~Cursor(void);
 * Virtual destructor.
 */

/**
 * @fn void Cursor::path(Output& out);
 * Display the path of the current cursor.
 * @param out	Output for displaying the path.
 */

/**
 * @fn void Cursor::info(Output& out);
 * @param out	Output for displaying the path.
 * Display information about the current item.
 */

/**
 * @fn void Cursor::list(Output& out);
 * @param out	Output for displaying the path.
 * List the content of the current item.
 */

/**
 * @fn Cursor *Cursor::go(CString name);
 * Go in the named sub-item of the current item.
 * @param name	Name of the item to go in.
 * @return				New cursor.
 * @throw	GoException	If the named item does not exists.
 */

/**
 * @fn Cursor *Cursor::back(void);
 * Go back to the container item.
 * @return	Cursor of the container item.
 * @throw BackException	If the cursor is already the top-level cursor.
 */

/**
 * @fn Cursor *Cursor::perform(Output& out, int argc, CString argv[]);
 * Perform a specific action.
 * @param out	Standard output for any output.
 * @param argc	Argument count (including the command name at index 0).
 * @param argv	Command arguments.
 * @return			Eventually, a new cursor.
 * @throw	PerformException	Thrown if the command is not known or
 * if the syntax is bad.
 */

/**
 * @fn void Cursor::help(Output& out)
 * Display the local help of the current cursor.
 * @param out	Used output.
 */


/**
 * @class FrameWorkCursor
 * This class implement the Cursor interface applied on the framework.
 */

}	// otawa
