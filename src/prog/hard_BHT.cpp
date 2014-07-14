/*
 *	BHT support implementation
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
			throw ProcessorException(*this, exn.message());
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


/**
 * This feature ensures that the BHT description has been loaded.
 *
 * @par Properties
 * @li @ref BHT_CONFIG
 */
p::feature BHT_FEATURE("otawa::hard::BHT_FEATURE", new Maker<BHTGetter>());


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

