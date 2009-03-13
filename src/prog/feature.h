/*
 *	$Id$
 *	Feature documentation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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

/**
 * @mainpage
 * OTAWA is logically split in different module representing different layers and usage of the
 * framework. In the following, you will find a guide to explore theses modules. Notice that
 * the OTAWA module splitting is not ever reflected by the header files and the libraries,
 * specially the kernel module of OTAWA.
 * 
 * OTAWA is logically split in different layer matching module:
 * @li @ref prog -- the main module and the root any work in OTAWA provides the abstract
 * representation of the program to handle.
 * @li @ref prop -- the property layer is built just above the program representation
 * to link properties or annotation to the program representation.
 * @li @ref proc -- this layer allows to make automatic the chaining and dependency
 * solving of analyses.
 * @li @ref hard -- this layer provides the description of the host platform where
 * the loaded program will run.
 * @li @ref dfa -- module devoted to generic classes to perform static analyses.
 * @li @ref ipet -- module containing class to implements the WCET computation by IPET.
 * @li @ref ff -- module providing support for flow fact information provided by user
 * or external tool.
 * @li @ref graph -- module providing classes to handle graphs.
 * @li @ref display -- module providing facilities to display graphs.
 * 
 * In addition, some utilities are provided:
 * @li @ref dumpcfg -- outputs CFG with different format (textual, .dot).
 * @li @ref oipet -- computes WCET according to different alternativs of the
 * IPET approach.
 * @li @ref opcg -- outputs the Program Call Graph (PCG),
 * @li @ref ostat -- computes some statistics about instructions of a program.
 */

/**
 * @defgroup features Available Features
 * 
 * @par Program Features
 * 
 * @li @ref FLOW_FACTS_FEATURE
 * 
 * @par CFG Features
 * 
 * @li @ref CFG_INFO_FEATURE
 * @li @ref COLLECTED_CFG_FEATURE
 * @li @ref CONTEXT_TREE_FEATURE
 * @li @ref DOMINANCE_FEATURE
 * @li @ref LOOP_HEADERS_FEATURE
 * @li @ref LOOP_INFO_FEATURE
 * @li @ref NORMALIZED_CFGS_FEATURE
 * @li @ref PCG_FEATURE
 * @li @ref UNROLLED_LOOPS_FEATURE
 * @li @ref VIRTUALIZED_CFG_FEATURE
 * 
 * @par IPET Features
 * 
 * @li @ref CONTROL_CONSTRAINTS_FEATURE
 * @li @ref FLOW_FACTS_CONSTRAINTS_FEATURE
 * @li @ref ILP_SYSTEM_FEATURE
 * @li @ref OBJECT_FUNCTION_FEATURE
 * @li @ref WCET_COUNT_RECORDED_FEATURE
 * @li @ref WCET_FEATURE
 * 
 * @par Pipeline timings features
 * 
 * @li @ref ASSIGNED_VARS_FEATURE
 * @li @ref BB_TIME_FEATURE
 * @li @ref DELTA_SEQUENCES_FEATURE
 * @li @ref EDGE_TIME_FEATURE
 * @li @ref ipet::FLOW_FACTS_FEATURE
 * @li @ref INTERBLOCK_SUPPORT_FEATURE
 * 
 * @par Cache specific features
 * 
 * @li @ref CCG_FEATURE
 * @li @ref COLLECTED_LBLOCKS_FEATURE
 * @li @ref DATA_CACHE_SUPPORT_FEATURE
 * @li @ref ICACHE_ACS_FEATURE
 * @li @ref ICACHE_ACS_MAY_FEATURE
 * @li @ref ICACHE_CATEGORY_FEATURE
 * @li @ref ICACHE_CATEGORY2_FEATURE
 * @li @ref ICACHE_FIRSTLAST_FEATURE
 * @li @ref INST_CACHE_SUPPORT_FEATURE
 * 
 * @par Loader Features
 * 
 * @li @ref CONTROL_DECODING_FEATURE
 * @li @ref FLOAT_MEMORY_ACCESS_FEATURE
 * @li @ref MEMORY_ACCESS_FEATURE
 * @li @ref REGISTER_USAGE_FEATURE
 * @li @ref SOURCE_LINE_FEATURE
 * @li @ref STACK_USAGE_FEATURE
 */

