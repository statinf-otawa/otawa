/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	oshell.h -- OShell cursor classes implementation.
 */

#include <stdlib.h>
#include <elm/genstruct/DLList.h>
#include <elm/genstruct/SortedBinTree.h>
#include <otawa/cfg.h>
#include "oshell.h"


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
 * Cursor used for examining CFG.
 */
class CFGCursor: public Cursor {
	CFG *cfg;
	bool built;
	genstruct::SortedBinTree<BasicBlock *> bbs;
	void build(void);
	static id_t ID_Number;

	// Visitor for numbering basic blocks
	class BasicBlockVisitor: public genstruct::SortedBinTree<BasicBlock *>::Visitor {
		int cnt;
	public:
		inline BasicBlockVisitor(void): cnt(0) { };
		int process(BasicBlock *bb) {
			bb->set<int>(ID_Number, cnt++);
			return 1;
		}
	};
	
	// Visitor for listing the CFG
	class ListVisitor: public genstruct::SortedBinTree<BasicBlock *>::Visitor {
		Output& out;
	public:
		inline ListVisitor(Output& _out): out(_out) { };
		int process(BasicBlock *bb);
	};
public:
	CFGCursor(Cursor *back, CFG *_cfg): Cursor(back), cfg(_cfg), built(false) {
	};
	virtual void path(Output& out) {
		back()->path(out);
		out << "/CFG" << cfg->entry()->address();
	};	
	virtual void list(Output& out);
};


// InstCursor class
class InstCursor: public Cursor {
	Inst *_inst;
public:
	InstCursor(Cursor *back, Inst *inst): Cursor(back), _inst(inst) {
	};
	
	virtual void path(Output& out) {
		back()->path(out);
		out << '/' << _inst->address();
	};
	virtual void info(Output& out) {
		out << "[INSTRUCTION]\n"
			 << "\taddress = " << _inst->address() << '\n';
		out << "\tasm = ";
		_inst->dump(out);
		out << '\n';
	}	
};

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
			if(inst->isControl())
				out << "\tCONTROL";
			out << '\n';
		}
	};		

	virtual Cursor *go(CString name) {
		address_t addr = (address_t)strtol(&name, 0, 16);
		for(Inst *inst = _code->first(); !inst->atEnd(); inst = inst->next())
			if(addr == inst->address())
				return new InstCursor(this, inst);
		throw GoException();
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
	
	virtual void list(Output& out);
	
	virtual Cursor *go(CString name);
	
	virtual Locked<Cursor> back(void) {
		throw BackException();
	};
	
	void load(Output& out, CString path);
	void cfg(Output& out);
	
	virtual void perform(Output& out, int argc, CString argv[]) {
		if(argv[0] == "load") {
			if(argc != 2) {
				out << "ERROR: " << "'load' require one argument.\n";
				return;
			}
			load(out, argv[1]);
		}	
		else if(argv[0] == "cfg") {
			if(argc != 1)
				throw new PerformException("Too many arguments.");
			cfg(out);			
		}
		else
			Cursor::perform(out, argc, argv);
	};
	
	virtual void help(Output& out) {
		out << "load <path>: load the given executable path in the current framework.\n";
		out << "cfg: build the CFG of the program.\n";
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


/**
 * @class FrameWorkCursor
 * This class implement the Cursor interface applied on the framework.
 */

/**
 * Load a file.
 * @param out	Output.
 * @param path	Path of the file to load.
 */
void FrameWorkCursor::load(Output& out, CString path) {
			File *file = fw->loadFile(path);
			if(!file)
				out << "ERROR: " << "during the load.\n";
			else
				out << "SUCCESS.\n";
}

/**
 * Build the CFG.
 * @param out	Output.
 */
void FrameWorkCursor::cfg(Output& out) {
	fw->getCFG();
	out << "CFG Built.\n";
}

// Overloaded
void FrameWorkCursor::list(Output& out) {
	
	// Display files
	out << "Files: \n";
	int i = 0;
	for(Iterator<File *> iter = fw->files(); iter; iter++, i++)
		out << '\t' << i << ": " << iter->name() << '\n';
	if(!i)
		out << "<none>\n";
	
	// Display CFG if any
	CFGInfo *info = fw->get<CFGInfo *>(CFGInfo::ID, 0);
	if(info) {
		out << "CFG:\n";
		i = 0;
		for(Iterator<CFG *> cfg = info->cfgs(); cfg; cfg++, i++) {
			out << "\tC" << i << ':' << cfg->entry()->address() << '\n';
		}
	}
};

// Overloaded	
Cursor *FrameWorkCursor::go(CString name) {
	
	// CFG traversal
	if(name[0] == 'C') {
		CFGInfo *info = fw->get<CFGInfo *>(CFGInfo::ID, 0);
		if(!info)
			throw GoException();
		int num = atoi(&name + 1);
		for(Iterator<CFG *> cfg = info->cfgs(); cfg; cfg++, num--)
			if(!num) {
				return new CFGCursor(this, *cfg);
			}
		throw GoException();
	}
	
	// Segment traversal
	else {
		int num = atoi(&name);
		for(Iterator<File *> iter = fw->files(); iter; iter++, num--)
			if(!num) {
				return new FileCursor(this, *iter);
			}
		throw GoException();
	}
}


/**
 * Used for storing numbers in basic blocks.
 */
id_t CFGCursor::ID_Number = Property::getID("OShell.Number");

/**
 * Build the CFG representation.
 */
void CFGCursor::build(void) {
	built = true;
	genstruct::DLList<BasicBlock *> remain;
	remain.addLast(cfg->entry());	
	
	// Find all basic blocks
	for(int num = 0; !remain.isEmpty(); num++) {
		
		// Get the current BB
		BasicBlock *bb = remain.first();
		remain.removeFirst();
		if(bbs.contains(bb))
			continue;
		bbs.insert(bb);
		
		// Display not-taken
		BasicBlock *target = bb->getNotTaken();
		if(target && !bbs.contains(target))
			remain.addLast(target);

		// Display taken
		target = bb->getTaken();
		if(target && !bbs.contains(target))
			remain.addLast(target);
	}

	// Give number to basic blocks
	BasicBlockVisitor visitor;
	bbs.visit(&visitor);
}

/**
 * Display the graph of the CFG.
 */
void CFGCursor::list(Output& out) {
	
	// Build
	if(!built)
		build();
	
	// Visit the bbs
	ListVisitor visitor(out);
	bbs.visit(&visitor);
}
int CFGCursor::ListVisitor::process(BasicBlock *bb) {
	
	// Display header
	out << "BB " << bb->use<int>(ID_Number) << ": ";
	BasicBlock *target = bb->getNotTaken();
	if(target)
		out << " NT(" << target->use<int>(ID_Number) << ')';
	target = bb->getTaken();
	if(target)
		out << " T(" << target->use<int>(ID_Number) << ')';
	out << '\n';

	// Disassemble basic block
	PseudoInst *pseudo;
	for(Inst *inst = bb->head()->next(); !inst->atEnd(); inst = inst->next()) {
		if((pseudo = inst->toPseudo()) && pseudo->id() == BasicBlock::ID)
			break;
		else {
			out << '\t' << inst->address() << ' ';
			inst->dump(out);
			out << '\n';
		}
	}
}

}	// otawa
