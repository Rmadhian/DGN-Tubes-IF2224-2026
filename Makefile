CXX = g++
TARGET = arion_lexer
SRCS = src/main.cpp src/dfa_dinamis.cpp src/dfa_statis.cpp src/lexer.cpp src/parser_expr.cpp src/parser_stmt.cpp src/parser_decl.cpp

$(TARGET): $(SRCS)
	$(CXX) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)