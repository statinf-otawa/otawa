/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	oshell.h -- OShell cursor classes implementation.
 */

#include <stdlib.h>
#include "oshell.h"

namespace otawa {


// CodeCursor class
class CodeCursor: public Cursor {
	Code *_code;
public:
	CodeCursor(Cursor *back, Code *code): Cursor(back), _code(code) {
	};
	
	virtual void path(Output& out) {
		back()->path(out);
		out << '/' << _code->name();
	};

	virtual void info(Output& out) {
		out << "[Code]\n";
		out << "name=" << _code->name() << '\n';
		out << "address=" << _code->address() << '\n';
		out << "size=" << (int)_code->size() << '\n';
	};	

	virtual void list(Output& out) {
		out << "Instructions \n";
		Inst *inst;
		for(inst = _code->first(); !inst->atEnd(); inst = inst->next()) {
			out << '\t' << inst->address() << ' ';
			inst->dump(out);
			out << '\n';
		}
	};		
};


// SegmentCursor class
class SegmentCursor: public Cursor {
	Segment *seg;
public:
	SegmentCursor(Cursor *back, Segment *segment): Cursor(back), seg(segment) {
	}
	
	virtual void path(Output& out) {
		back()->path(out);
		out << '#' << seg->name();
	};

	virtual void info(Output& out) {
		out << "[Segment]\n";
		out << "name=" << seg->name() << '\n';
		out << "address=" << seg->address() << '\n';
		out << "size=" << (int)seg->size() << '\n';
		out << "flags=";
		if(seg->flags() & Segment::EXECUTABLE)
			out << " EXEC";
		if(seg->flags() & Segment::WRITABLE)
			out << " WRITE";
		out << '\n';
	};	

	virtual void list(Output& out) {
		out << "Items: \n";
		int i = 0;
		for(Iterator<ProgItem *> iter = seg->items(); iter; iter++, i++) {
			out << '\t' << i << ':' << iter->name() << '(';
			out << (iter->toCode() ? "code" : "data");
			out << ") [" << iter->address() << ':' << (int)iter->size() << "]\n";
		}
		if(!i)
			out << "<none>\n";
	};	
	
	virtual Cursor *go(CString name) {
		int num = atoi(&name);
		for(Iterator<ProgItem *> iter = seg->items(); iter; iter++, num--)
			if(!num) {
				Code *code = iter->toCode();
				if(!code)
					break;
				else
					return new CodeCursor(this, code);
			}
		throw GoException();
	};
};


// FileCursor class
class FileCursor: public Cursor {
	File *_file;
public:
	FileCursor(Cursor *back, File *file): Cursor(back), _file(file) {
	};

	virtual void path(Output& out) {
		out << _file->name();
	};

	virtual void info(Output& out) {
		out << "[File]\n";
		out << "path=" << _file->name() << '\n';
	};	

	virtual void list(Output& out) {
		out << "Segments: \n";
		int i = 0;
		for(Iterator<Segment *> iter = _file->segments(); iter; iter++, i++) {
			out << '\t' << i << ':' << iter->name()
				 << " [" << iter->address()
				 << ':' << (int)iter->size() << ']';
			if(iter->flags() & Segment::EXECUTABLE)
				out << " EXEC";
			if(iter->flags() & Segment::WRITABLE)
				out << " WRITE";
			out << '\n';
		}
		if(!i)
			out << "<none>\n";
	};
	
	virtual Cursor *go(CString name) {
		int num = atoi(&name);
		for(Iterator<Segment *> iter = _file->segments(); iter; iter++, num--)
			if(!num)
				return new SegmentCursor(this, *iter);
		throw GoException();
	};
};

// FrameWorkCursor class
class FrameWorkCursor: public Cursor {
	FrameWork *fw;
public:
	inline FrameWorkCursor(FrameWork *_fw): fw(_fw) { };
	
	virtual void path(Output& out) {
	};
	
	virtual void info(Output& out) {
		out << "[FrameWork]\n";
	};
	
	virtual void list(Output& out) {
		out << "Files: \n";
		int i = 0;
		for(Iterator<File *> iter = fw->files(); iter; iter++, i++)
			out << '\t' << i << ": " << iter->name() << '\n';
		if(!i)
			out << "<none>\n";
	};
	
	virtual Cursor *go(CString name) {
		int num = atoi(&name);
		for(Iterator<File *> iter = fw->files(); iter; iter++, num--)
			if(!num) {
				return new FileCursor(this, *iter);
			}
		throw GoException();
	};
	
	virtual Locked<Cursor> back(void) {
		throw BackException();
	};
	
	virtual void perform(Output& out, int argc, CString argv[]) {
		if(argv[0] == "load") {
			if(argc != 2) {
				out << "ERROR: " << "'load' require one argument.\n";
				return;
			}
			File *file = fw->loadFile(argv[1]);
			if(!file)
				out << "ERROR: " << "during the load.\n";
			else
				out << "SUCCESS.\n";
		}
		else
			Cursor::perform(out, argc, argv);
	};
	
	virtual void help(Output& out) {
		out << "load <path>: load the given executable path in the current framework.\n";
	};
};


/**
 * @class GoException
 * Exception thrown when a "go" action is attempted and the target item
 * does not exists.
 */

// Overloaded
CString GoException::message(void) {
	return "Target item does not exist.";
}


/**
 * @class BackException
 * Exception thrown when a "back" action is performed on a cursor already
 * at the top-level.
 */

// Overloaded
CString BackException::message(void) {
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

}	// otawa
