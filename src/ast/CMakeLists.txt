

set(ast_la_SOURCES
	"ast_ASTLoader.cpp"
#	"ast_parser.yy"
#	"ast_lexer.ll"
	"AST.cpp"
	"ASTInfo.cpp"
	"BlockAST.cpp"
	"CallAST.cpp"
	"DoWhileAST.cpp"
	"ForAST.cpp"
	"FunAST.cpp"
	"proc_FunProcessor.cpp"
	"IfAST.cpp"
	"proc_ASTProcessor.cpp"
	"SeqAST.cpp"
	"WhileAST.cpp"
	)

include_directories(".")

if(WIN32)
	BISON_FILE("ast_parser.yy")
	list(APPEND ast_la_SOURCES ${OUTFILE})
	FLEX_FILE("ast_lexer.ll")
	list(APPEND ast_la_SOURCES lex.${PREFIX}.c lex.${PREFIX}.h)
elseif(UNIX)
	
	
endif(WIN32)


include_directories("../../include")


add_library(ast SHARED ${ast_la_SOURCES})
install(TARGETS ast DESTINATION ${LIBDIR})
target_link_libraries(ast libotawa)
