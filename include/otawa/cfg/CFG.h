/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/cfg/CFG.h -- interface of CFG class.
 */
#ifndef OTAWA_CFG_CFG_H
#define OTAWA_CFG_CFG_H

namespace otawa {

// Classes
class BasicBlock;
class Code;

	
// CFG class
class CFG: public ProgObject {
	Code *_code;
	AutoPtr<BasicBlock> ent;
public:
	static id_t ID;
	CFG(Code *code, AutoPtr<BasicBlock> entry);
	inline AutoPtr<BasicBlock> entry(void) const { return ent; };
	inline Code *code(void) const { return _code; };
};

} // otawa

#endif // OTAWA_CFG_CFG_H
