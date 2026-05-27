CXX = g++
TARGET = arion_lexer
SRCS = src/core/main.cpp src/core/dfa_dinamis.cpp src/core/dfa_statis.cpp src/core/lexer.cpp src/core/parser_expr.cpp src/core/parser_stmt.cpp src/core/parser_decl.cpp src/core/ast_builder.cpp src/core/semantic_decl.cpp src/core/semantic_expr.cpp src/core/semantic_stmt.cpp src/core/semantic_printer.cpp src/core/icg_decl.cpp src/core/icg_expr.cpp src/core/icg_stmt.cpp src/core/interpreter_core.cpp src/core/interpreter_expr.cpp src/core/interpreter_flow.cpp src/core/interpreter_security.cpp

$(TARGET): $(SRCS)
	$(CXX) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)