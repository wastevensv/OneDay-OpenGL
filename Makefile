TARGET=combine-test
CXXFLAGS=-std=c++11 -lGL -lglfw -lGLEW -lSOIL `pkg-config opencv --cflags --libs`
DEBUGFLAGS=-ggdb
compile: $(TARGET).cpp
	g++ $(CXXFLAGS) $(TARGET).cpp -o $(TARGET)
debug: $(TARGET).cpp
	g++ $(CXXFLAGS) $(DEBUGFLAGS) $(TARGET).cpp -o $(TARGET)
valgrind: compile
	valgrind ./$(TARGET)
run: compile
	./$(TARGET)
