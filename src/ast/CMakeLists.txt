set(CMAKE_INSTALL_RPATH "\\$ORIGIN")

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
elseif(UNIX OR APPLE)
	BISON_TARGET(ast_parser ast_parser.yy ${CMAKE_CURRENT_BINARY_DIR}/ast_parser.cpp COMPILE_FLAGS --defines=ast_parser.h)
	FLEX_TARGET(ast_lexer ast_lexer.ll  ${CMAKE_CURRENT_BINARY_DIR}/ast_lexer.cc)
	ADD_FLEX_BISON_DEPENDENCY(ast_lexer ast_parser)
	list(APPEND ast_la_SOURCES
		${BISON_ast_parser_OUTPUTS}
		${FLEX_ast_lexer_OUTPUTS})
	
	
endif(WIN32)


include_directories("../../include")


add_library(ast SHARED ${ast_la_SOURCES})
install(TARGETS ast DESTINATION ${LIBDIR})
target_link_libraries(ast otawa)
