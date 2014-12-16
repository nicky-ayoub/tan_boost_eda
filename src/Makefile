#
#
# http://www.drdobbs.com/exploring-eda-algorithms-with-the-boost/184401676
#
#
#

OBJECTS=CmdLine.o  Debug.o  DLogic.o atpg.o
SOURCE=CmdLine.cpp  Debug.cpp  DLogic.cpp atpg.cpp
EXEC=atpg
CXXFLAGS=-Wall -std=c++11

#------------------------------------------------------------------------------

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CXX)  -o$(EXEC) $(OBJECTS)

clean:

	rm -f $(EXEC)  $(OBJECTS)
