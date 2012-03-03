/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/hard/BHT.h -- interface for BHT class.
 */
#ifndef OTAWA_HARD_BHT_H
#define OTAWA_HARD_BHT_H

#include <elm/io.h>
#include <elm/serial2/macros.h>
#include <elm/genstruct/Vector.h>
#include <otawa/base.h>
#include <otawa/hard/PureCache.h>
#include <otawa/proc/SilentFeature.h>

namespace otawa { namespace hard {

// default prediction type
const static int PREDICT_NONE = 0,
				 PREDICT_TAKEN = 1,
				 PREDICT_NOT_TAKEN = 2,
				 PREDICT_DIRECT = 3,
				 PREDICT_UNKNOWN = 4;

// BHT class
class BHT: public PureCache {
private:
	int cond_penalty;
	int indirect_penalty;
	int cond_indirect_penalty;
	int def_predict;
	
	SERIALIZABLE(BHT, ELM_BASE(PureCache) &
		DFIELD(cond_penalty, 10) &
		DFIELD(indirect_penalty, 10) &
		DFIELD(cond_indirect_penalty, 10) &
		DFIELD(def_predict, PREDICT_TAKEN)
	);

public:
	inline BHT(void) { }
	virtual ~BHT(void) { }
	
	// Modifiers
	inline void setCondPenalty(int time) { cond_penalty = time; }
	inline void setIndirectPenalty(int time) { indirect_penalty = time; }
	inline void setCondIndirectPenalty(int time) { cond_indirect_penalty = time; }
	inline void setDefaultPrediction(int def) { def_predict = def; }
	
	// Accessors
	inline int getCondPenalty(void) const { return cond_penalty; }
	inline int getIndirectPenalty(void) const { return indirect_penalty; }
	inline int getCondIndirectPenalty(void) const { return cond_indirect_penalty; }
	inline int getDefaultPrediction(void) const { return def_predict; }
};

extern SilentFeature BHT_FEATURE;
extern Identifier<BHT*> BHT_CONFIG;

} } // otawa::hard

#endif // OTAWA_HARD_BHT_H
