/*
 *	AlternativeProcessor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2015, IRIT UPS.
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
#ifndef OTAWA_ALTERNATIVE_PROCESSOR_H
#define OTAWA_ALTERNATIVE_PROCESSOR_H

#include <elm/data/Vector.h>
#include <otawa/proc/Processor.h>

namespace otawa {

class AlternativeProcessor: public Processor {
public:

	class Alternative {
	public:
		inline Alternative& require(const AbstractFeature& f) { feats.add(FeatureUsage(FeatureUsage::require, f)); return *this; }
		inline Alternative& use(const AbstractFeature& f) { feats.add(FeatureUsage(FeatureUsage::use, f)); return *this; }
		inline const Vector<FeatureUsage> features(void) const { return feats; }
	private:
		Vector<FeatureUsage> feats;
	};

	class AlternativeSet: public Vector<const Alternative *> {
	public:
		inline AlternativeSet& add(const Alternative& alt) { Vector<const Alternative *>::add(&alt); return *this; };
	};

	AlternativeProcessor(AlternativeSet& alts, AbstractRegistration& r);

protected:
	virtual const Alternative& select(WorkSpace *ws) = 0;

private:
	AlternativeSet set;
};

namespace p {
	inline AlternativeProcessor::Alternative alt(void) { return AlternativeProcessor::Alternative(); }
	inline AlternativeProcessor::AlternativeSet altset(void) { return AlternativeProcessor::AlternativeSet(); }
}	// p

}	// otawa

#endif // OTAWA_ALTERNATIVE_PROCESSOR_H
