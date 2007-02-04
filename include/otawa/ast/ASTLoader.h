/*
 *	$Id$
 *	Copyright (c) 2005, IRIT-UPS.
 *
 *	otawa/ast/ASTLoader.h -- interface for ASTLoader class.
 */
#ifndef OTAWA_AST_AST_LOADER_H
#define OTAWA_AST_AWT_LOADER_H

#include <elm/genstruct/Vector.h>
#include <otawa/proc/Processor.h>

// Externals
namespace otawa {
	class ASTLoader;
	class File;
}
int ast_parse(otawa::ASTLoader *loader);
void ast_error(otawa::ASTLoader *loader, const char *msg);

namespace otawa {

// ASTLoader class
class ASTLoader: public Processor {
	friend int ::ast_parse(ASTLoader *loader);
	friend void ::ast_error(ASTLoader *loader, const char *msg);
	
	elm::String path;
	elm::genstruct::Vector<String> calls;
	FrameWork *fw;
	File *file;
	
	void onError(const char *fmt, ...);
	AST *makeBlock(elm::CString entry, elm::CString exit);
	address_t findLabel(elm::String raw_label);
public:
	static Identifier<String> PATH;
	
	// Constructors
	ASTLoader(void);
	
	// Processor overload
	virtual void configure(PropList& props);
	virtual void processFrameWork(FrameWork *fw);
};

} // otawa

#endif // OTAWA_AST_AST_LOADER_H
