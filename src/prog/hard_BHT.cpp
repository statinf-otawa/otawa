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
 * @li @ref BHT_CONFIG
 * @li @ref BHT_ELEMENT
 * @li @ref BHT_PATH
 *
 * @par Provided Features
 * @li @ref BHT_FEATURE
 *
 * @ingroup hard
 */
class BHTGetter: public otawa::Processor {
public:
	static p::declare reg;
	BHTGetter(void): Processor(reg), bht(0), element(0) { }

protected:

	virtual void configure(const PropList& props) {
		Processor::configure(props);
		bht = BHT_CONFIG(props);
		if(!bht) {
			element = BHT_ELEMENT(props);
			if(!element) {
				path = BHT_PATH(props);
			}
		}
	}

	virtual void processWorkSpace(WorkSpace *ws) {
		if(!bht) {

			try {
				bht = new BHT();

				// from XML element
				if(element) {
					elm::serial2::XOMUnserializer unserializer(element);
					unserializer >> *bht;
				}

				// from XML file
				else if(path) {
					elm::serial2::XOMUnserializer unserializer(path);
					unserializer >> *bht;
				}

				// no way
				else {
					delete bht;
					bht = 0;
					throw ProcessorException(*this, "no way to get the BHT description");
				}

			}
			catch(elm::Exception& exn) {
				delete bht;
				bht = 0;
				throw ProcessorException(*this, exn.message());
			}
		}
		BHT_CONFIG(ws) = bht;
		this->track(BHT_FEATURE, BHT_CONFIG(ws));
	}


private:
	BHT *bht;
	xom::Element *element;
	sys::Path path;
};

p::declare BHTGetter::reg = p::init("otawa::hard::BHTGetter", Version(1, 1, 0))
	.provide(BHT_FEATURE)
	.maker<BHTGetter>();


/**
 * @class BHT
 * This class contains all information about the Branch History Predictor, that is,
 * @li the definition of the stored target a cache,
 * @li timing information for non-predicted conditional branch,
 * @li timing for predicted indirect branch,
 * @li timing for non-predicted indirect branch,
 * @li default prediction behavior.
 *
 * @ingroup hard
 */

/**
 */
BHT::BHT(void)
:	cond_penalty(10),
	indirect_penalty(10),
	cond_indirect_penalty(10),
	def_predict(PREDICT_NOT_TAKEN)
{
}

/**
 */
BHT::~BHT(void) {
}

/**
 * Compute the actual default prediction. PREDICT_TAKEN and PREDICT_NOT_TAKEN
 * default branch are returned as is. PREDICT_DIRECT uses the arguments to
 * determine the default branch direction:
 * @li branch address >= target address (maybe a loop) -- predicted taken.
 * @li branch address < target address -- predicted not-taken.
 * @param branch	Branch address.
 * @param target	Target address.
 * @return			Any prediction except PREDICT_DIRECT.
 */
int BHT::actualDefaultPrediction(Address branch, Address target) {
	if(def_predict != PREDICT_DIRECT)
		return def_predict;
	else if(branch >= target)
		return PREDICT_TAKEN;
	else
		return PREDICT_NOT_TAKEN;
}


/**
 * This feature ensures that the BHT description has been loaded.
 *
 * @par Properties
 * @li @ref BHT_CONFIG
 *
 * @par Configuration
 * @li @ref BHT_CONFIG
 * @li @ref BHT_ELEMENT
 * @li @ref BHT_PATH
 *
 * @ingroup hard
 */
p::feature BHT_FEATURE("otawa::hard::BHT_FEATURE", new Maker<BHTGetter>());


/**
 * Gives the current BHT description.
 *
 * @par Hooks
 * @li configuration properties (@ref BHT_FEATURE)
 * @li @ref WorkSpace
 *
 * @ingroup hard
 */
Identifier<BHT*> BHT_CONFIG("otawa::hard::BHT_CONFIG", 0);


/**
 * Used as a configuration property for @ref BHT_FEATURE, provides an XML element
 * describing a BHT.
 *
 * @ingroup hard
 */
Identifier<xom::Element *> BHT_ELEMENT("otawa::hard::BHT_ELEMENT", 0);


/**
 * Used as a configuration property @ref BHT_FEATURE, provides the path to an XML file
 * describing a BHT.
 *
 * @ingroup hard
 */
Identifier<elm::sys::Path> BHT_PATH("otawa::hard::BHT_PATH");

} } // otawa::hard

ENUM_BEGIN(otawa::hard::predict_t)
	VALUE(otawa::hard::PREDICT_TAKEN),
	VALUE(otawa::hard::PREDICT_NOT_TAKEN),
	VALUE(otawa::hard::PREDICT_DIRECT)
ENUM_END

// Serialization support
SERIALIZE(otawa::hard::BHT);

