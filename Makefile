CXX = g++
TARGET = arion_lexer
SRCS = src/main.cpp src/dfa_dinamis.cpp src/dfa_statis.cpp

$(TARGET): $(SRCS)
	$(CXX) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)