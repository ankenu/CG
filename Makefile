# CXX=gcc
# CXXFLAGS=
# CPPFLAGS=-DDEBUG -Isrc -Isrc/vendor
# LDFLAGS=
# LDLIBS=-lglfw -lGL

# SRCS=
# OBJS=$(SRCS:.cpp=.o)
# EXECUTABLE=a.out

# all: $(SRCS) $(EXECUTABLE)

# $(EXECUTABLE): $(OBJS)
#   $(CXX) $(LDFLAGS) $(OBJS) -o $@ $^

# %.o: %

build:
	g++ main.cpp vendor/Glad/src/glad.c vendor/stb_image/stb_image.cpp -lglfw -lGL -ldl -DGLFW_INCLUDE_NONE -Ivendor/Glad/include -o cg  && ./cg