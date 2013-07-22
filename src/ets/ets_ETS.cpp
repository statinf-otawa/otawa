/*
 *	ETS module
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
#include <otawa/ets/features.h>
#include <otawa/ets/ACSComputation.h>
#include <otawa/ets/CacheFirstMissComputation.h>
#include <otawa/ets/CacheHitComputation.h>
#include <otawa/ets/CacheMissComputation.h>
#include <otawa/ets/FlowFactLoader.h>
#include <otawa/ets/TrivialAstBlockTime.h>
#include <otawa/ets/WCETComputation.h>
 
namespace otawa { namespace ets {

/**
 * @defgroup ets Extended Timing Schema
 * This WCET computation approach is based on AST representation.
 * WCET is computed from leaves AST to the root by applying a computation
 * depending on the type of the AST node.
 */


static SilentFeature::Maker<ACSComputation> acs_maker;
/**
 * Compute the Abstract Cache State at entry of each AST for L1 instruction cache.
 *
 * @p Properties
 * @li @ref ACS
 *
 * @p Default Processor
 * @li @ref ACSComputation
 *
 * @ingroup ets
 */
SilentFeature ACS_FEATURE("otawa::ets::ACS_FEATURE", acs_maker);

/**
 * This identifier (AbstractCacheState *) is used for storing the abstract cache
 * line for each node.
 *
 * @ingroup ets
 */
Identifier<AbstractCacheState *> ACS("otawa::ets::ACS", 0);


static SilentFeature::Maker<CacheFirstMissComputation> fmiss_maker;
/**
 * Provide the computation of occurrences of first-misses of instruction cache.
 *
 * @p Properties
 * @li @ref FIRST_MISSES
 *
 * @p Default Process
 * @li @ref CacheFirstMissComputation
 *
 * @ingroup ets
 */
SilentFeature CACHE_FIRST_MISS_FEATURE("otawa::ets::CACHE_FIRST_MISS_FEATURE", fmiss_maker);


/**
 * This identifier (int) is used for storing the number of first miss accesses
 * for each node.
 *
 * @ingroup ets
 */
Identifier<int> FIRST_MISSES("otawa::ets::FIRST_MISSES", 0);


static SilentFeature::Maker<CacheHitComputation> hit_maker;
/**
 * Provided the computation of the number of hits for the L1 instruction cache.
 *
 * @p Properties
 * @li @ref HITS
 *
 * @p Default Processor
 * @li @ref CacheHitComputation
 *
 * @ingroup ets
 */
SilentFeature CACHE_HIT_FEATURE("otawa::ets::CACHE_HIT_FEATURE", hit_maker);


/**
 * This identifier (int) is used for storing the number of hit accesses for each
 * node.
 *
 * @ingroup ets
 */
Identifier<int> HITS("otawa::ets::HITS", -1);


static SilentFeature::Maker<CacheMissComputation> miss_maker;
/**
 * Provide the number of misses for a L1 instruction cache.
 *
 * @p Properties
 * @li @ref MISSES
 *
 * @p Default Processors
 * @li @ref CacheMissComputation
 *
 * @ingroup ets
 */
SilentFeature CACHE_MISS_FEATURE("otawa::ets::CACHE_MISS_FEATURE", miss_maker);


/**
 * This identifier (int) is used for storing the number of miss accesses for each
 * node.
 *
 * @ingroup ets
 */
Identifier<int> MISSES("otawa::ets::misses", 0);


static SilentFeature::Maker<FlowFactLoader> ff_maker;
/**
 * Provide flow facts in the AST.
 *
 * @p Properties
 * @li @ref LOOP_COUNT
 *
 * @p Default Processor
 * @li @ref FlowFactLoader
 *
 * @ingroup ets
 */
SilentFeature FLOWFACT_FEATURE("otawa::ets::FLOWFACT_FEATURE", ff_maker);


/**
 * This identifier (int) is used for storing the number of iteration for each loop
 * of the program.
 *
 * @ingroup ets
 */
Identifier<int> LOOP_COUNT("otawa::ets::LOOP_COUNT", -1);


static SilentFeature::Maker<TrivialAstBlockTime> bt_maker;
/**
 * Provide WCET for each AST.
 *
 * @p Properties
 * @li @ref WCET
 *
 * @p Default Processor
 * @li @ref  TrivialAstBlockTime
 *
 * @ingroup ets
 */
SilentFeature BLOCK_TIME_FEATURE("otawa::ets::BLOCK_TIME_FEATURE", bt_maker);


static SilentFeature::Maker<WCETComputation> w_maker;
/**
 * Provide the WCET of a task using the ETS approach.
 *
 * @p Properties
 * @li @ref WCET
 *
 * @p Default Processor
 * @li @ref  WCETComputation
 *
 * @ingroup ets
 */
SilentFeature WCET_FEATURE("otawa::ets::WCET_FEATURE", w_maker);


/**
 * Identifier of annotation (int) used for storing the WCET value
 * in the ETS of the computed function.
 *
 * @ingroup ets
 */
Identifier<int> WCET("otawa::ets::wcet", -1);


/**
 * This identifier (int) is used for storing the number of conflict accesses for
 * each node.
 */
Identifier<int> CONFLICTS("otawa::ets::conflicts", 0);

} }// otawa::ets
