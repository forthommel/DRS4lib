ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS   = $(shell root-config --libs)
ROOTGLIBS  = $(shell root-config --glibs)
CXXFLAGS  += $(ROOTCFLAGS)
LIBS       = $(ROOTLIBS) 
GLIBS      = $(ROOTGLIBS)
GXX        = g++ -Wall -O3


listGoodEvents: listGoodEvents.cc
	$(GXX) -o listGoodEvents listGoodEvents.cc $(ROOTCFLAGS) $(LIBS) $(ROOTGLIBS)

maketree: maketree.cc
	$(GXX) -o maketree maketree.cc $(ROOTCFLAGS) $(LIBS) $(ROOTGLIBS)

merge: merge.cc
	$(GXX) -o merge merge.cc $(ROOTCFLAGS) $(LIBS) $(ROOTGLIBS)


clean :
	rm -f *.o listGoodEvents maketree merge *~
