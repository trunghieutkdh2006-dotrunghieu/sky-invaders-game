CXX     = g++
CXXFLAGS = -std=c++17 -Wall $(shell sdl2-config --cflags)
LIBS     = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lSDL2_mixer

SRCS = main.cpp render.cpp enemy.cpp bullet.cpp powerup.cpp audio.cpp gamestate.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = invaders

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
