/*
 *	$Id$
 *	Copyright (c) 2006, IRIT - UPS.
 *
 *	otawa/hard_Processor.cpp -- processor description interface.
 */

#include <elm/serial/implement.h>
#include <otawa/hard/Processor.h>
#include <elm/serial/Unserializer.h>

SERIALIZE_ENUM(otawa::hard::Stage::type_t,
	ENUM_VALUE(otawa::hard::Stage::FETCH),
	ENUM_VALUE(otawa::hard::Stage::LAZY),
	ENUM_VALUE(otawa::hard::Stage::EXEC),
	ENUM_VALUE(otawa::hard::Stage::COMMIT));

//SERIALIZE_ENUM(otawa::Inst::kind_t,

SERIALIZE(otawa::hard::FunctionalUnit,
	FIELD(name);
	FIELD(width);
	FIELD(latency);
	FIELD(pipelined));

namespace elm { namespace serial {

template <> void Unserializer::read<otawa::hard::Dispatch::type_t>(
	otawa::hard::Dispatch::type_t& type
) {
	
	// List  of identifiers
	static elm::Pair<elm::CString, int> values[] = {
		ENUM_VALUE(otawa::Inst::IS_COND),
		ENUM_VALUE(otawa::Inst::IS_CONTROL),
		ENUM_VALUE(otawa::Inst::IS_CALL),
		ENUM_VALUE(otawa::Inst::IS_RETURN),
		ENUM_VALUE(otawa::Inst::IS_MEM),
		ENUM_VALUE(otawa::Inst::IS_LOAD),
		ENUM_VALUE(otawa::Inst::IS_STORE),
		ENUM_VALUE(otawa::Inst::IS_INT),
		ENUM_VALUE(otawa::Inst::IS_FLOAT),
		ENUM_VALUE(otawa::Inst::IS_ALU),
		ENUM_VALUE(otawa::Inst::IS_MUL),
		ENUM_VALUE(otawa::Inst::IS_DIV),
		ENUM_VALUE(otawa::Inst::IS_SHIFT),
		ENUM_VALUE(otawa::Inst::IS_TRAP),
		ENUM_VALUE(otawa::Inst::IS_INTERN),
		elm::Pair<elm::CString, int>("", 0)
	};
	
	// Build the type
	type = 0;
	String text;
	read(text);
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
		for(int i = 0; values[i].fst; i++) {
			CString cst = values[i].fst;
			if(item == cst ||
			(cst.endsWith(item) && cst[cst.length() - item.length() - 1] == ':')) {
				done = true; 
				type |= values[i].snd;
				break;
			}
		}
		if(!done)
			throw io::IOException("unknown symbol \"%s\".", &item);
	}
}

} } // elm::serial

SERIALIZE(otawa::hard::Dispatch,
	FIELD(fu);
	FIELD(type));

SERIALIZE(otawa::hard::Stage,
	FIELD(name);
	FIELD(type);
	FIELD(width);
	FIELD(latency);
	FIELD(fus);
	FIELD(dispatch);
	FIELD(ordered));

SERIALIZE(otawa::hard::Queue,
	FIELD(name);
	FIELD(size);
	FIELD(input);
	FIELD(output);
	FIELD(intern));
	
SERIALIZE(otawa::hard::Processor,
	FIELD(arch);
	FIELD(model);
	FIELD(builder);
	FIELD(stages);
	FIELD(queues));
