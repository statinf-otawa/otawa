/*
 * AccessedAddress.cpp
 *
 *  Created on: 2 juil. 2009
 *      Author: casse
 */

#include <otawa/prog/Inst.h>
#include <otawa/util/AccessedAddress.h>

namespace otawa {

void AccessedAddress::print(io::Output& out) const {
	out << "\t" << inst->address() << "\t";
	out << "\t";
	if(store)
		out << "store ";
	else
		out << "load ";
	switch(knd) {
	case ANY: out << "T"; break;
	case ABS: out << ((const AbsAddress *)this)->address();
	case SP: out << "SP+" << ((const SPAddress *)this)->offset();
	}
	out << io::endl;
}

Feature<NoProcessor> ADDRESS_ANALYSIS_FEATURE("otawa::ADDRESS_ANALYSIS_FEATURE");
Identifier<AccessedAddresses *> ADDRESSES("otawa::ADDRESSES", 0);

}	// otawa
