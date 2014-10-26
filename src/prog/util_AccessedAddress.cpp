/*
 * AccessedAddress.cpp
 *
 *  Created on: 2 juil. 2009
 *      Author: casse
 */

#include <otawa/prog/Inst.h>
#include <otawa/stack/AccessedAddress.h>

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


/**
 * @class AddressStats: public BBProcessor
 * This processor is used to collect stacks about the found
 * accessed addresses. Statistics are returned in a structure
 * passed to @ref otawa::AddressStat::STATS .
 *
 * @par Required Features
 * @li @ref otawa::ADDRESS_ANALYSIS_FEATURE
 *
 * @par Configuration
 * @par @ref otawa::AddressStats::DISPLAY
 * @par @ref otawa::AddressStats::STATS
 */

/**
 * Configuration identifier for the @ref otawa::AddressStats processor.
 */
Identifier<AddressStats::stat_t *> AddressStats::STATS("otawa::AddressStats::STATS", 0);


/**
 * Configuration displayer for activating / non-display of address statistics
 * by the @ref otawa::AddressStats processor.
 */
Identifier<bool> AddressStats::DISPLAY(" otawa::AddressStats::DISPLAY", true);


/**
 */
AddressStats::AddressStats(void): BBProcessor("otawa::AddressStats", Version(1, 0, 0)) {
	require(ADDRESS_ANALYSIS_FEATURE);
}


/**
 */
void AddressStats::configure(const PropList &props) {
	BBProcessor::configure(props);
	stats = STATS(props);
	if(!stats)
		stats = &istats;
	display = DISPLAY(props);
}


/**
 */
void AddressStats::setup (WorkSpace *fw) {
	stats->all = 0;
	stats->abs = 0;
	stats->sprel = 0;
	stats->total = 0;
}


/**
 */
void AddressStats::cleanup (WorkSpace *fw) {
	if(display)  {
		out << "ADDRESS STATICTICS\n";
		cout << "unknown:\t" << stats->all << " (" << (stats->total ? (stats->all * 100. / stats->total) : 0) << "%)\n";
		cout << "SP-relative:\t" << stats->sprel << " (" << (stats->total ? (stats->sprel * 100. / stats->total) : 0) << "%)\n";
		cout << "absolute:\t" << stats->abs << " (" << (stats->total ? (stats->abs * 100. / stats->total) : 0) << "%)\n";
		cout << "total:\t" << stats->total << " (100%)\n";
	}
}


/**
 */
void AddressStats::processBB (WorkSpace *fw, CFG *cfd, BasicBlock *bb) {

	// get addresses
	AccessedAddresses *addrs = ADDRESSES(bb);
	if(!addrs)
		return;

	// do statistics
	stats->total += addrs->size();
	for(int i = 0; i < addrs->size(); i++)
		switch(addrs->get(i)->kind()) {
		case AccessedAddress::ANY: stats->all++; break;
		case AccessedAddress::SP: stats->sprel++; break;
		case AccessedAddress::ABS: stats->abs++; break;
		default: ASSERT(false);
		}
}

}	// otawa
