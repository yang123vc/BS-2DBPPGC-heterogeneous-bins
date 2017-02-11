CXXFLAGS =	-O2 -Wall -fmessage-length=0
#CXXFLAGS = -g -DDEBUG -Wall -fmessage-length=0

OBJS =	Angles.o area.o Basic_Funcs.o Candidates.o Children_Func.o \
		ConstructGlobalSolution.o ConstructSolution.o ConvexHull.o \
		CreateTree.o Dibujar.o GuillotineCuts.o HorizPlacement.o IrrBin.o \
		LastBin.o MatchPieces.o MoveInSection.o Node.o OpenBin.o piece.o \
		Print.o SectionsInNodes.o VerticalPlacement.o main.o

LIBS = 

TARGET = BPGC_heter_bins

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)