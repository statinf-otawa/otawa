/*
 *	$Id$
 *	Copyright (c) 2006, IRIT - UPS.
 *
 *	otawa/hard_Processor.cpp -- processor description interface.
 */

#include <otawa/hard/Processor.h>

ENUM_BEGIN(otawa::hard::Stage::type_t)
	VALUE(otawa::hard::Stage::FETCH),
	VALUE(otawa::hard::Stage::LAZY),
	VALUE(otawa::hard::Stage::EXEC),
	VALUE(otawa::hard::Stage::COMMIT)
ENUM_END

SERIALIZE(otawa::hard::FunctionalUnit);
SERIALIZE(otawa::hard::Dispatch);
SERIALIZE(otawa::hard::Stage);
SERIALIZE(otawa::hard::Queue);
SERIALIZE(otawa::hard::Processor);

namespace elm { namespace serial2 {

void __unserialize(Unserializer& s, otawa::hard::Dispatch::type_t& v) {
	
	// List  of identifiers
	static elm::value_t values[] = {
		VALUE(otawa::Inst::IS_COND),
		VALUE(otawa::Inst::IS_CONTROL),
		VALUE(otawa::Inst::IS_CALL),
		VALUE(otawa::Inst::IS_RETURN),
		VALUE(otawa::Inst::IS_MEM),
		VALUE(otawa::Inst::IS_LOAD),
		VALUE(otawa::Inst::IS_STORE),
		VALUE(otawa::Inst::IS_INT),
		VALUE(otawa::Inst::IS_FLOAT),
		VALUE(otawa::Inst::IS_ALU),
		VALUE(otawa::Inst::IS_MUL),
		VALUE(otawa::Inst::IS_DIV),
		VALUE(otawa::Inst::IS_SHIFT),
		VALUE(otawa::Inst::IS_TRAP),
		VALUE(otawa::Inst::IS_INTERN),
		value("", 0)
	};
	
	// Build the type
	v = 0;
	String text;
	__unserialize(s, text);
	while(text) {
		
		// Get the component
		int pos = text.indexOf('|');
		String item;
		if(pos < 0) {
			item = text;
			text = "";
		}
		else { 
			item = text.substring(0, pos);
			text = text.substring(pos + 1);
		}
		
		// Find the constant
		bool done = false;
		for(int i = 0; values[i].name(); i++) {
			CString cst = values[i].name();
			if(item == cst ||
			(cst.endsWith(item) && cst[cst.length() - item.length() - 1] == ':')) {
				done = true; 
				v |= values[i].value();
				break;
			}
		}
		if(!done)
			throw io::IOException("unknown symbol \"%s\".", &item);
	}
}

void __serialize(Serializer& s, otawa::hard::Dispatch::type_t v) {
	assert(0);
}

} } // elm::serial2
