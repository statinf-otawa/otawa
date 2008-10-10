otawaprocdir=$(libdir)/otawa/proc/otawa

AM_CXXFLAGS = @OTAWA_CXXFLAGS@ @GEL_CXXFLAGS@

otawaproc_LTLIBRARIES = ast.la

ast_la_LDFLAGS = -module
ast_la_LIBADD = @OTAWA_LIBS@
ast_la_DEPENDENCIES = @OTAWA_DEPS@

ast_la_SOURCES = \
	ast_ASTLoader.cpp \
	ast_parser.yy \
	ast_lexer.ll \
	AST.cpp \
	ASTInfo.cpp \
	BlockAST.cpp \
	CallAST.cpp \
	DoWhileAST.cpp \
	ForAST.cpp \
	FunAST.cpp \
	proc_FunProcessor.cpp \
	IfAST.cpp \
	proc_ASTProcessor.cpp \
	SeqAST.cpp \
	WhileAST.cpp
