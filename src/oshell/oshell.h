/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	oshell.h -- OShell cursor classes interface.
 */
#ifndef OTAWA_OSHELL_H
#define OTAWA_OSHELL_H

#include <elm/utility.h>
#include <elm/io/io.h>
#include <otawa/manager.h>
using namespace elm::io;

namespace otawa {

// GoException class
class GoException: public Exception {
public:
	virtual String message(void);
};

// BackException class
class BackException: public Exception {
public:
	virtual String message(void);
};

// PerformException class
class PerformException: public Exception {
	CString msg;
public:
	inline PerformException(CString message): msg(message) { };
	virtual String message(void) { return msg; };	
};

// Cursor abstract class
class Cursor: public Lock {
protected:
	AutoPtr<Cursor> bck;
	inline Cursor(void) { };
	inline Cursor(Cursor *back): bck(back) { };
	inline Cursor *back(void) const { return &bck; };
public:
	static Cursor *get(FrameWork *fw);
	virtual ~Cursor(void) { };
	virtual void path(Output& out) { };
	virtual void info(Output& out) { };
	virtual void list(Output& out) { };
	virtual void display(Output& out) { };
	virtual Cursor *go(CString name) {
		if(bck)
			bck->go(name);
		else
			throw GoException();
	};
	virtual AutoPtr<Cursor> back(void) {
		if(bck.isNull())
			throw BackException();
		else
			return bck;
	};
	virtual void perform(Output& out, int count, CString argv[]) {
		if(bck)
			bck->perform(out, count, argv);
		else
			throw PerformException("Undefined command.");
	};
	virtual void help(Output& out) {
		if(bck)
			bck->help(out);
	};
};
	
}

#endif	// OTAWA_OSHELL_H
