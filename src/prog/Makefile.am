lib_LIBRARIES=libprog.a

libprog_a_SOURCES = \
	BlockAST.cpp \
	FunAST.cpp \
	ASTInfo.cpp \
	CallAST.cpp \
	SeqAST.cpp \
	IfAST.cpp \
	WhileAST.cpp \
	DoWhileAST.cpp \
	ForAST.cpp \
	AST.cpp \
	base.cpp \
	program.cpp \
	manager.cpp \
	platform.cpp \
	instruction.cpp \
	type.cpp \
	Loader.cpp \
	FrameWork.cpp \
	BasicBlock.cpp \
	CFGInfo.cpp \
	CFG.cpp \
	Edge.cpp \
	prop_Identifier.cpp \
	prop_PropList.cpp \
	prop_Property.cpp \
	prop_DeletableProperty.cpp \
	ilp_Var.cpp \
	ilp_Constraint.cpp \
	ilp_System.cpp \
	proc_Processor.cpp \
	proc_CFGProcessor.cpp \
	proc_FunProcessor.cpp \
	proc_ASTProcessor.cpp \
	proc_BBProcessor.cpp \
	ipet_IPET.cpp \
	ipet_TrivialBBTime.cpp \
	ipet_VarAssignment.cpp \
	Process.cpp \
	ipet_BasicConstraintsBuilder.cpp \
	ipet_WCETComputation.cpp \
	util_DFA.cpp \
	util_DFABitSet.cpp \
	ets_ETS.cpp \
	ets_WCETComputation.cpp \
	ets_TrivialAstBlockTime.cpp \
	ets_FlowFactReader.cpp \
	ets_FunReader.cpp \
	util_Dominance.cpp \
	util_ContextTree.cpp \
	cfg_VirtualBasicBlock.cpp \
	cfg_VirtualCFG.cpp \
	util_FlowFactLoader.cpp \
	util_fft_parser.yy \
	util_fft_lexer.ll \
	ipet_FlowFactLoader.cpp
	
INCLUDES=\
	-I../../include \
	@ELM_CFLAGS@
	
AM_CXXFLAGS=\
	-g3 \
	@LP_SOLVE_DEF@

AM_YFLAGS=-dv

#	ipet_parser.yy 
#	ipet_lexer.ll 
#	ipet_ConstraintLoader.cpp
