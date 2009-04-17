/*
 *	$Id$
 *	Copyright (c) 2006, IRIT - UPS.
 *
 *	otawa/hard_Processor.cpp -- processor description interface.
 */

#include <otawa/hard/Processor.h>

namespace otawa { namespace hard {

/**
 * @class Processor
 * Description of a processor pipeline. A pipeline is viewed as a collection
 * of stages (@ref otawa::hard::Stage) separated by instructions queues
 * (@ref otawa::hard::Queue).
 * @ingroup hard
 */

/**
 * @class Stage
 * @ingroup hard
 * 
 * This class represents a stage in a pipeline. The stages have a type:
 * @li @ref otawa::hard::Stage::FETCH -- a stage loading instruction from memory
 * (usually the initial stage of a pipeline).
 * @li @ref otawa::hard::Stage::LAZY -- a simple stage that is traversed by instructions,
 * @li @ref otawa::hard::Stage::EXEC -- a stage that performs execution of instructions
 * (it contains functionnal units that performs the work of the instructions),
 * @li @ref otawa::hard::Stage::COMMIT -- a stage that writes result of an instruction
 * in the processor state (usually the end of the traversal of the instructions).
 * 
 * The stages are described by the following attributes:
 * @li the name,
 * @li the width (number of instructions traversing in parallel the stage),
 * @li the latency (number of cycles taken to traverse the stage),
 * @li the order (is the instructions traversal perform in program order or not),
 * @li for the EXEC stage only, a functionnal unit list (@ref otawa::hard::FunctionalUnit)
 * and dispatch list (@ref otawa::hard::Dispatch).
 */

/**
 * @class Queue
 * @ingroup hard
 * 
 * The instructions queues stores instruction that come from one stage to another one.
 * Possibly, an instruction queue may have an internal execution stage that uses
 * the stored instruction to compute their result.
 */

/**
 * @class FunctionalUnit
 * @ingroup hard
 * 
 * A functional unit is specialized in the computation of some kinds of instructions.
 * They are contained in a stage (@ref otawa::hard::Stage) of type EXEC. 
 * The selection of a functional unit is performed thanks to the @ref otawa::hard::Dispatch
 * objects provided in the execution stage.
 * 
 * A functional unit is defined by:
 * @li its latency (computation time in cycles),
 * @li its width (number of instructions traversing the functional unit in parallel),
 * @li pipeline property indicating that the function unit is pipelined or not
 * (that is it may chain instruction execution cycles after cycles).
 */

/**
 * @class Dispatch
 * @ingroup hard
 * 
 * A dispatch object allows to map some kinds of instructions to a functional unit.
 * To find if the dispatch match an instruction, an AND is performed between the
 * instruction kind and the dispatch type and if the result is equal to the dispatch
 * type, this dispatch functional unit is selected. This means that the dispatched
 * functional unit only process instructions that meets the all properties found
 * in the dispatch kind.
 */

} } // otawa::hard

ENUM_BEGIN(otawa::hard::Stage::type_t)
VALUE(otawa::hard::Stage::FETCH),
  VALUE(otawa::hard::Stage::LAZY),
  VALUE(otawa::hard::Stage::EXEC),
  VALUE(otawa::hard::Stage::COMMIT),
  VALUE(otawa::hard::Stage::DECOMP)
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
	  throw io::IOException(_ << "unknown symbol \"" << item << "\".");
      }
    }

    void __serialize(Serializer& s, otawa::hard::Dispatch::type_t v) {
      assert(0);
    }

  } } // elm::serial2
