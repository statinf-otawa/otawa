/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/BHT.cpp -- implementation of BHT class.
 */

#include <otawa/prop/Identifier.h>
#include <otawa/hard/BHT.h>
#include <otawa/script/Script.h>
#include <otawa/prog/WorkSpace.h>
#include <elm/xom.h>
#include <elm/serial2/XOMUnserializer.h>

using namespace elm;

namespace otawa { namespace hard {

/**
 * Provide the description of the BHT (if not already available).
 * Mainly, either it keeps the current BHT_CONFIG, or it looks for other
 * way to get it:
 * @li mainly in the PLATFORM property put by the @ref Script processor.
 *
 * @par Configuration
 * @li @ref BHT_CONFIG -- BHT configuration to use
 *
 * @par Provided Features
 * @li @ref BHT_FEATURE
 */
class BHTGetter: public otawa::Processor {
public:
	static Registration<BHTGetter> reg;
	BHTGetter(void): Processor(reg), bht(0), pf(0) { }

protected:

	virtual void configure(const PropList& props) {
		Processor::configure(props);
		bht = BHT_CONFIG(props);
		if(!bht)
			pf = script::PLATFORM(props);
	}

	virtual void processWorkSpace(WorkSpace *ws) {
		if(!bht) {
			if(pf) {
				xom::Element *bht_elt = pf->getFirstChildElement("bht");
				if(bht_elt)
					loadXML(bht_elt);
			}
			if(!bht)
				throw ProcessorException(*this, "no configuration found for the BHT");
		}
		BHT_CONFIG(ws) = bht;
		this->track(BHT_FEATURE, BHT_CONFIG(ws));
	}


private:

	void loadXML(xom::Element *bht_elt) {
		elm::serial2::XOMUnserializer unserializer(bht_elt);
		bht = new BHT();
		try {
			unserializer >> *bht;
		}
		catch(elm::Exception& exn) {
			delete bht;
			bht = 0;
			throw exn;
		}
	}

	BHT *bht;
	xom::Element *pf;
};

Registration<BHTGetter> BHTGetter::reg(
		"otawa::hard::BHTGetter", Version(1, 0, 0),
		p::provide, &BHT_FEATURE,
		p::end
	);


/**
 * @class BHT
 * This class contains all information about the Branch History Predictor, that is,
 * @li the definition of the stored target a cache,
 * @li timing information for non-predicted conditional branch,
 * @li timing for predicted indirect branch,
 * @li timing for non-predicted indirect branch,
 * @li default prediction behavior.
 */


static SilentFeature::Maker<BHTGetter> BHT_MAKER;
/**
 * This feature ensures that the BHT description has been loaded.
 *
 * @par Properties
 * @li @ref BHT_CONFIG
 */
SilentFeature BHT_FEATURE("otawa::hard::BHT_FEATURE", BHT_MAKER);


/**
 * Gives the current BHT description.
 *
 * @par Hooks
 * @li configuration properties
 * @li @ref WorkSpace
 */
Identifier<BHT*> BHT_CONFIG("otawa::hard::BHT_CONFIG", 0);

} } // otawa::hard

// Serialization support
SERIALIZE(otawa::hard::BHT);

