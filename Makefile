TARGET=combine-test
CXXFLAGS=-std=c++11 -lGL -lglfw -lGLEW -lSOIL -lopencv_core -lopencv_highgui -lopencv_video
DEBUGFLAGS=-ggdb
compile: $(TARGET).cpp
	g++ $(CXXFLAGS) $(TARGET).cpp -o $(TARGET)
debug: $(TARGET).cpp
	g++ $(CXXFLAGS) $(DEBUGFLAGS) $(TARGET).cpp -o $(TARGET)
valgrind: compile
	valgrind ./$(TARGET)
run: compile
	./$(TARGET)
