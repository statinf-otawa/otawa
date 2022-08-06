set(CMAKE_INSTALL_RPATH "${ORIGIN}")
set(CMAKE_MACOSX_RPATH true)
set(CMAKE_CXX_FLAGS "-Wall" )

# TODO
#	"cfg_PSTBuilder.cpp"
#	"dfa_FastState.cpp"
#	"ai.cpp"

# sources
set(libotawa_SOURCES

#   core module
	"feature.h"
	"base.cpp"
	"type.cpp"
	"Loader.cpp"

	"dfa_MemorySet.cpp"

#    CFG module
	"CFG.cpp"
	"cfg_CFGCollector.cpp"
	"pcg_PCG.cpp"
	"pcg_PCGBuilder.cpp"
	"cfg_AbstractCFGBuilder.cpp"
	"cfg_CFGAdapter.cpp"
	"cfg_CFGBuilder.cpp"
	"cfg_CFGChecker.cpp"
	"cfg_CFGCheckSummer.cpp"
	"cfg_CFGInfo.cpp"
	"cfg_CFGTransformer.cpp"
	"cfg_ConditionalRestructurer.cpp"
	"cfg_DelayedBuilder.cpp"
	"cfg_Dominance.cpp"
	"cfg_interproc.cpp"
	"cfg_Loop.cpp"
	"cfg_LoopIdentifier.cpp"
	"cfg_LoopInfoBuilder.cpp"
	"cfg_LoopReductor.cpp"
	"cfg_LoopUnroller.cpp"
	"cfg_PostDominance.cpp"
	"cfg_SubCFGBuilder.cpp"
	"cfg_Virtualizer.cpp"
	"cfg_Weighter.cpp"
    "cfgio_Input.cpp"
	"cfgio_Output.cpp"
	"events.cpp"
	"BBRatioDisplayer.cpp"
	"app_Application.cpp"
	"app_AddressOption.cpp"
	"app_CFGApplication.cpp"
	"app_Test.cpp"

	"icache.cpp"

	"ilp_AbstractSystem.cpp"
	"ilp_Constraint.cpp"
	"ilp_Expression.cpp"
	"ilp_ILPPlugin.cpp"
	"ilp_impl.cpp"
	"ilp_System.cpp"
	"ilp_Var.cpp"

#   proc module
	"proc_AlternativeProcessor.cpp"
	"proc_Processor.cpp"
	"proc_CFGProcessor.cpp"
	"proc_ConcurrentCFGProcessor.cpp"
	"proc_ContextualProcessor.cpp"
	"proc_DynFeature.cpp"
	"proc_DynProcessor.cpp"
	"proc_EdgeProcessor.cpp"
	"proc_Feature.cpp"
	"proc_FeatureRequirer.cpp"
	"proc_LBlockProcessor.cpp"
	"proc_BBProcessor.cpp"
	"proc_Monitor.cpp"
	"proc_ProcessorException.cpp"
	"proc_ProcessorPlugin.cpp"
	"proc_Registry.cpp"
	"stats.cpp"
	"stats_BBStatCollector.cpp"
	"stat_StatsDumper.cpp"

#    property module
	"prop_AbstractIdentifier.cpp"
	"prop_ContextualProperty.cpp"
	"prop_DeletableProperty.cpp"
	"prop_Identifier.cpp"
	"prop_Property.cpp"
	"prop_PropList.cpp"

#   ipet module
	"ipet_BasicConstraintsBuilder.cpp"
	"ipet_BasicObjectFunctionBuilder.cpp"
	"ipet_ConstraintLoader.cpp"
	"ipet_FlowFactLoader.cpp"
	"ipet_FlowFactConstraintBuilder.cpp"
	"ipet_FlowFactConflictConstraintBuilder.cpp"
	"ipet_ILPSystemGetter.cpp"
	"ipet_IPET.cpp"
	"ipet_VarAssignment.cpp"
	"ipet_WCETComputation.cpp"
	"ipet_WCETCountRecorder.cpp"
	"ilp_Output.cpp"

#   data-flow analysis module
	"dfa_BitSet.cpp"
	"dfa_Debug.cpp"
	"dfa_hai_DefaultFixPoint.cpp"
	"dfa_hai_DefaultListener.cpp"
	"dfa_hai_FirstUnrollingFixPoint.cpp"
	"dfa_hai_HalfAbsInt.cpp"
	"dfa_hai_UnrollingListener.cpp"
	"dfa_hai_WideningFixPoint.cpp"
	"dfa_hai_WideningListener.cpp"
	"dfa_IterativeDFA.cpp"
	"dfa_XCFGVisitor.cpp"
	"dfa_XIterativeDFA.cpp"
	"util_AccessedAddress.cpp"
	"util_StackAnalysis.cpp"
	"util_SymAddress.cpp"
	"dfa_State.cpp"
	"dfa_Lexicon.cpp"

#    abstract interpretation module
	"ai.cpp"
	"ai_CFGAnalyzer.cpp"
	"ai_FlowAwareRanking.cpp"
	"ai_PseudoTopoOrder.cpp"

#    utility module
	"util_CFGNormalizer.cpp"
	"util_MemBlockMap.cpp"
	"util_ContextTree.cpp"
	"util_Bag.cpp"
	"util_Node.cpp"
	"util_Edge.cpp"
	"ograph.cpp"
	"view.cpp"

#	rtti
	"rtti.cpp"

#	flow facts
	"flowfact_ContextualLoopBound.cpp"
	"flowfact_FlowFactLoader.cpp"

#   script module
	"util_XSLTScript.cpp"
	"script_NamedObject.cpp"
	"script_Script.cpp"

#    hardware module
	"hard_Dumper.cpp"
	"hard_BHT.cpp"
	"hard_Processor.cpp"
	"hardware_Cache.cpp"
	"hardware_CacheConfiguration.cpp"
	"hardware_Platform.cpp"
	"hardware_PureCache.cpp"
	"hard_Register.cpp"
	"hard_Memory.cpp"

#    instruction cache module
	"cache_ACSBuilder.cpp"
	"cache_ACSMayBuilder.cpp"
	"cache_categories.cpp"
	"cache_CAT2Builder.cpp"
	"cache_CAT2NCBuilder.cpp"
	"cache_CAT2ConstraintBuilder.cpp"
	"cache_CachePenalty.cpp"
	"cache_MUSTProblem.cpp"
	"cache_MAYProblem.cpp"
 	"cache_PERSProblem.cpp"
	"cache_MUSTPERS.cpp"
	"cache_LinkedBlocksDetector.cpp"
	"cache_LBlock.cpp"
	"cache_FirstLastBuilder.cpp"
	"cache_LBlockSet.cpp"
	"cache_CachePenaltiesObjectFunctionBuilder.cpp"
	"cache_LBlockBuilder.cpp"

#   program representation module
	"prog_Bundle.cpp"
	"prog_File.cpp"
	"prog_FixedTextDecoder.cpp"
	"prog_Inst.cpp"
	"prog_Label.cpp"
	"prog_Manager.cpp"
	"prog_ProgItem.cpp"
	"prog_Process.cpp"
	"prog_Segment.cpp"
	"prog_Symbol.cpp"
	"prog_TaskInfoService.cpp"
	"prog_TextDecoder.cpp"
	"prog_VarTextDecoder.cpp"
	"prog_VirtualInst.cpp"
	"prog_WorkSpace.cpp"
	"sem.cpp"

#   execution graph module
	"parexegraph_ParExeProc.cpp"
	"parexegraph_Resource.cpp"
	"parexegraph_ParExeGraph.cpp"
	"parexegraph_GraphBBTime.cpp"
	"util_Trace.cpp"

#    sgraph module
    "sgraph_DiGraph.cpp"
    "sgraph_LoopIdentifier.cpp"

#	sim module
	"sim_Simulator.cpp"
	"sim_State.cpp"
	"sim_AbstractCacheDriver.cpp"
	"sim_CacheDriver.cpp"
	"sim_TrivialSimulator.cpp"
	"sim_Driver.cpp"
	"sim_BasicBlockDriver.cpp"
	
#	tsim module
	"BBTimeSimulator.cpp"

	"loader_arm.cpp"

)



include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(".")

BISON_TARGET(ipet_parser ipet_parser.yy ${CMAKE_CURRENT_BINARY_DIR}/ipet_parser.cpp HEADER ${CMAKE_CURRENT_BINARY_DIR}/ipet_parser.h)
FLEX_TARGET(ipet_lexer ipet_lexer.ll  ${CMAKE_CURRENT_BINARY_DIR}/ipet_lexer.cc)
ADD_FLEX_BISON_DEPENDENCY(ipet_lexer ipet_parser)
BISON_TARGET(util_fft_parser util_fft_parser.yy ${CMAKE_CURRENT_BINARY_DIR}/util_fft_parser.cpp HEADER ${CMAKE_CURRENT_BINARY_DIR}/util_fft_parser.h)
FLEX_TARGET(util_fft_lexer util_fft_lexer.ll  ${CMAKE_CURRENT_BINARY_DIR}/util_fft_lexer.cc)
ADD_FLEX_BISON_DEPENDENCY(util_fft_lexer util_fft_parser)
list(APPEND libotawa_SOURCES
	${BISON_ipet_parser_OUTPUTS}
	${FLEX_ipet_lexer_OUTPUTS}
	${BISON_util_fft_parser_OUTPUTS}
	${FLEX_util_fft_lexer_OUTPUTS})

# build
add_library(otawa SHARED ${libotawa_SOURCES})
target_link_libraries(otawa "${LIBELM}")
target_link_libraries(otawa "${LIBGEL}")
target_link_libraries(otawa "${LIBGEL_DWARF}")

# installation
install(TARGETS otawa DESTINATION ${LIBDIR})
