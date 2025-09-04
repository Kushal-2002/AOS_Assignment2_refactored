CXX = g++
CXXFLAGS = -Wall -g
OBJS = main.o shell_core.o autocomplete.o builtins.o history.o io.o pipeline.o utils.o signals.o
TARGET = shell

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp shell.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
