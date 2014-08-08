/*
 *	ASTLoader class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdarg.h>
#include <stdio.h>
#include <otawa/ast.h>
#include <otawa/ast/ASTLoader.h>
#include <elm/debug.h>
#include <otawa/ast/features.h>

using namespace elm;

extern FILE *ast_in;

namespace otawa { namespace ast {

/**
 * @class ASTLoader
 * This class may be used for loading AST from an external file using the
 * Heptane format (see doc/ directory for more details). The file containing
 * the AST description may be passed directly using @ref ID_ASTFile configuration
 * identifier or is built from executable path by appending ".ast" extension.
 *
 * @ingroup ast
 */


p::declare ASTLoader::reg = p::init("otawa::ast::ASTLoader", Version(1, 2, 0))
	.provide(FEATURE)
	.maker<ASTLoader>();

/**
 * Build a new AST loader.
 */
ASTLoader::ASTLoader(p::declare& r): Processor(r), ws(0), file(0) {
}


/**
 * This feature ensures that the AST structure of the binary has been loaded.
 *
 * @p Configuration
 * @li @ref PATH
 *
 * @p Properties
 * @li @ref FUN
 * @li @ref INFO
 *
 * @ingroup ast
 */
p::feature FEATURE("otawa::ast::FEATURE", new Maker<ASTLoader>());


/**
 * This identifier may be passed for specifying the path a file for loading
 * the AST.
 *
 * @p Feature
 * @li @ref otawa::ast::FEATURE
 *
 * @ingroup ast
 */
Identifier<String> PATH("otawa::ast::PATH", "");


/**
 */
void ASTLoader::configure(PropList& props) {
	path = PATH(props);
}


/**
 */
void ASTLoader::onError(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	out.format(fmt, args);
	va_end(args);
}


/**
 */
void ASTLoader::processWorkSpace(WorkSpace *ws) {
	ASSERT(ws);
	this->ws= ws;
	
	// Get a valid path
	if(!path) {
		file = ws->process()->program();
		ASSERT(file);
		elm::StringBuffer buffer;
		buffer << file->name() << ".ast";
		path = buffer.toString();
	}

	// Open the file
	ast_in = fopen(&path.toCString(), "r");
	if(!ast_in) {
		onError("ERROR: cannot open the constraint file \"%s\".", &path.toCString());
		return;
	}
	
	// Perform the parsing
	try {
		ast_parse(this);
	}
	catch(LoadException& e) {
		onError(&e.message());
	}
	
	// Close all
	fclose(ast_in);
}


/**
 * Build a block or a sequence of calls and block from the recorded calls
 * and the entry and exit of the block.
 * @param entry		Entry label.
 * @param exit		Exit label.
 * @return			Result AST.
 */
AST *ASTLoader::makeBlock(elm::CString entry, elm::CString exit) {
	
	// Retrieve entry instruction
	String entry_name(&entry, entry.length() - 1);
	Inst *entry_inst = ws->findInstAt(findLabel(entry_name));
	if(!entry_inst)
		throw LoadException(_ << "Cannot find instruction at \"" << entry << "\".");
	
	// Retrieve exit address
	String exit_name(&exit, exit.length() - 1);
	address_t exit_addr = findLabel(exit_name);
	
	// Any internal call
	if(!calls)
		return new BlockAST(entry_inst, exit_addr - entry_inst->address());
			
	// Resolve called labels
	genstruct::Vector<Inst *> call_insts;
	for(int i = 0; i < calls.length(); i++) {
		Inst *inst = ws->findInstAt(findLabel(calls[i].toCString()));
		if(!inst)
			throw LoadException(_ << "Cannot find instruction at \"" << calls[i] << "\".");
		else
			call_insts.add(inst);
	}
	
	// Find AST info
	ASTInfo *info = ASTInfo::getInfo(ws);
	
	// Build the matching sequence
	AST *ast = 0;
	Inst *start = entry_inst, *inst;
	int cnt = 0;
	for(inst = entry_inst; inst; inst = inst->nextInst()) {
		if(inst->isCall()) {
			
			// Find the function
			Inst *target = inst->target();
			FunAST *fun = info->getFunction(target);
			if(target != call_insts[cnt])
				continue;
				
			// Build the AST
			AST *call = new CallAST(start,
				inst->address() + inst->size() - start->address(), fun);
			if(!ast)
				ast = call;
			else
				ast = new SeqAST(ast, call);
					
			// Clean-up
			cnt++;
			start = inst->nextInst();
			if(cnt >= calls.length())
				break;
		}
		else if(inst->isReturn())
			throw LoadException(_ << "binary unconsistent with AST (" << entry << ".");
	}
	if(cnt < calls.length()) {
		throw LoadException(_ << "binary unconsistent with AST (" << entry << ").");
	}
	
	// Remaining block ?
	if(!start->prevInst() && start->address() != exit_addr)
		ast = new SeqAST(ast, new BlockAST(start, exit_addr - start->address()));
	
	// Clean-up
	calls.clear();
	return ast;
}


/**
 * Resolve an Heptane label to an address.
 * @param raw_label	Heptane label to resolve.
 * @return			Matching address or null.
 */
address_t ASTLoader::findLabel(elm::String raw_label) {

	// Retrieve the file
	if(!file)
		file = ws->process()->program();
	ASSERT(file);
	
	// Compute entry
	String label = raw_label.substring(1, raw_label.length() - 1);

	// Retrieve the labels
	address_t addr = file->findLabel(label);
	if(!addr)
		throw LoadException(_ << "Cannot resolve label \"" << label << "\".");
	return addr;
}

} } // otawa::ast
