#
#
# http://www.drdobbs.com/exploring-eda-algorithms-with-the-boost/184401676
#
#
#

SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
EXEC=atpg
CXXFLAGS=-Wall -std=c++11
CXXFLAGS=-Wall
LIBS= -lboost_graph -lboost_program_options -lboost_regex

#------------------------------------------------------------------------------
%.o : %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c $^

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:

	rm -f $(EXEC) $(OBJECTS) *~
