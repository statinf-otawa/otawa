/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/oshell/FrameWorkCursor.cpp -- implementation of FrameWorkCursor class.
 */

#include <stdlib.h>
#include <otawa/ast.h>
#include <otawa/ast/ASTLoader.h>
#include "FrameWorkCursor.h"
#include "CFGCursor.h"
#include "FileCursor.h"
#include "FunCursor.h"

namespace otawa {

// Cursor overload
void FrameWorkCursor::path(Output& out) {
}


// Cursor overload
void FrameWorkCursor::info(Output& out) {
		out << "[FrameWork]\n";
}


// Cursor overload
void FrameWorkCursor::list(Output& out) {
	
	// Display files
	out << "Files: \n";
	int i = 0;
	for(Iterator<File *> iter(*fw->files()); iter; iter++, i++)
		out << '\t' << i << ": " << iter->name() << '\n';
	if(!i)
		out << "<none>\n";
	out << '\n';
	
	// Display CFG if any
	CFGInfo *info = fw->get<CFGInfo *>(CFGInfo::ID, 0);
	if(info) {
		out << "CFG:\n";
		i = 0;
		for(Iterator<CFG *> cfg(info->cfgs()); cfg; cfg++, i++) {
			out << "\tC" << i << ':' << cfg->entry()->address();
			Option<String> label = cfg->get<String>(File::ID_Label);
			if(label)
				out << " [@" << *label << ']';
			out << '\n';
		}
	}
	
	// Display the functions if any
	ASTInfo *ainfo = ASTInfo::getInfo(fw);
	if(ainfo) {
		out << "Function AST:" << ainfo << "\n";
		int i = 0;
		for(Iterator< FunAST *> fun(ainfo->functions()); fun; fun++) {
			out << "\tF" << i;
			if(fun->name())
				out << " (&" << fun->name() << ')';
			out << '\n';
			i++;
		}
		out << '\n';
	}
}

// Cursor overload	
Cursor *FrameWorkCursor::go(CString name) {
	
	// CFG traversal by address
	if(name[0] == 'C') {
		CFGInfo *info = fw->get< CFGInfo *>(CFGInfo::ID, 0);
		if(!info)
			throw GoException();
		int num = atoi(&name + 1);
		for(Iterator<CFG *> cfg(info->cfgs()); cfg; cfg++, num--)
			if(!num)
				return new CFGCursor(this, *cfg);
		throw GoException();
	}
	
	// CFG traversal by name
	else if(name[0] == '@') {
		if(name[1] == '\0') {
			CFG *cfg = fw->getStartCFG();
			if(cfg)
				return new CFGCursor(this, cfg);
		}
		else {
			CFGInfo *info = fw->get<CFGInfo *>(CFGInfo::ID, 0);
			if(!info)
				throw GoException();
			for(Iterator<CFG *> cfg(info->cfgs()); cfg; cfg++) {
				Option<String> label = cfg->get<String>(File::ID_Label);
				if(label && *label == name.chars() + 1)
					return new CFGCursor(this, *cfg);
			}
		}
	}
	
	// Fun traversal by index
	else if(name[0] == 'F') {
		int i = atoi(name.chars() + 1);
		ASTInfo *ainfo = ASTInfo::getInfo(fw);
		if(ainfo) {
			for(Iterator< FunAST *> fun(ainfo->functions()); fun;
			fun++, i--)
				if(!i)
					return new FunCursor(this, *fun);
			throw GoException();
		}
	}
	
	// Fun traversal by name
	else if(name[0] == '&') {
		String fname = name.chars() + 1;
		ASTInfo *ainfo = ASTInfo::getInfo(fw);
		if(ainfo) {
			Option< FunAST *> fun = ainfo->map().get(
				String(name.chars() + 1));
			if(fun)
				return new FunCursor(this, *fun);
			else
				throw GoException();
		}	
	}
	
	// Segment traversal
	else {
		int num = atoi(&name);
		for(Iterator<File *> iter(*fw->files()); iter; iter++, num--)
			if(!num) {
				return new FileCursor(this, *iter);
			}
	}
	
	// Pass to parent
	return back()->go(name);
}


// Cursor overload
AutoPtr<Cursor> FrameWorkCursor::back(void) {
	throw BackException();
}


/**
 * Load a file.
 * @param out	Output.
 * @param path	Path of the file to load.
 */
void FrameWorkCursor::load(Output& out, CString path) {
	try {
		File *file = fw->loadFile(path);
		out << "SUCCESS.\n";
	} catch(Exception& e) {
		out << "ERROR: " << e.message() << "\n";
	}
}


/**
 * Build the CFG.
 * @param out	Output.
 */
void FrameWorkCursor::cfg(Output& out) {
	fw->getCFGInfo();
	out << "CFG Built.\n";
}


// Cursor overload
void FrameWorkCursor::perform(Output& out, int argc, CString argv[]) {
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
	else if(argv[0] == "ast") {
		if(argc > 2)
			throw new PerformException("Too many arguments.");
		ast(argc == 2 ? argv[1] : "");
	}
	else
		Cursor::perform(out, argc, argv);
}

// Cursor overload
void FrameWorkCursor::help(Output& out) {
	out << "load <path>: load the given executable path in the current framework.\n";
	out << "cfg: build the CFG of the program.\n";
	out << "ast [path]: load the AST [from the given file].\n";
}


/**
 * Load the AST information from the default file or the given one.
 * @param path	Path to load information from (may be null for default file).
 */
void FrameWorkCursor::ast(CString path) {
	PropList props;
	if(path)
		props.add<String>(ASTLoader::ID_ASTFile, path);
	ASTLoader loader;
	loader.configure(props);
	loader.processFrameWork(fw);
}

} // otawa
