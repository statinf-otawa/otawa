set(CMAKE_INSTALL_RPATH "\\$ORIGIN/../../..")
set(CMAKE_MACOSX_RPATH true)

set(ast_la_SOURCES
	"ast_ASTLoader.cpp"
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

BISON_TARGET(ast_parser ast_parser.yy ${CMAKE_CURRENT_BINARY_DIR}/ast_parser.cpp HEADER ${CMAKE_CURRENT_BINARY_DIR}/ast_parser.h)
FLEX_TARGET(ast_lexer ast_lexer.ll  ${CMAKE_CURRENT_BINARY_DIR}/ast_lexer.cc)
ADD_FLEX_BISON_DEPENDENCY(ast_lexer ast_parser)
list(APPEND ast_la_SOURCES
	${BISON_ast_parser_OUTPUTS}
	${FLEX_ast_lexer_OUTPUTS})

include_directories("../../include")


add_library(ast SHARED ${ast_la_SOURCES})
set_property(TARGET ast PROPERTY PREFIX "")
set(SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
target_link_libraries(ast otawa)
install(TARGETS ast DESTINATION "${PROCDIR}/otawa")
