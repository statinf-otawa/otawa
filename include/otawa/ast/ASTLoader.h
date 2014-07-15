/*
 *	ASTLoader class interface
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
#ifndef OTAWA_AST_AST_LOADER_H
#define OTAWA_AST_AST_LOADER_H

#include <elm/genstruct/Vector.h>
#include <otawa/proc/Processor.h>

// Externals
namespace otawa {
	namespace ast { class ASTLoader; }
	class File;
}
int ast_parse(otawa::ast::ASTLoader *loader);
void ast_error(otawa::ast::ASTLoader *loader, const char *msg);

namespace otawa { namespace ast {

// ASTLoader class
class ASTLoader: public Processor {
	friend int ::ast_parse(ASTLoader *loader);
	friend void ::ast_error(ASTLoader *loader, const char *msg);
	
	elm::String path;
	elm::genstruct::Vector<String> calls;
	WorkSpace *ws;
	File *file;
	
	void onError(const char *fmt, ...);
	AST *makeBlock(elm::CString entry, elm::CString exit);
	address_t findLabel(elm::String raw_label);
public:
	static p::declare reg;
	
	// Constructors
	ASTLoader(p::declare& r = reg);
	
	// Processor overload
	virtual void configure(PropList& props);
	virtual void processWorkSpace(WorkSpace *ws);
};

} } // otawa::ast

#endif // OTAWA_AST_AST_LOADER_H
