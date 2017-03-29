OUT = re8 
DATA = data
SOUND = output.wav
GRAPH = graph.png
UST = ust.ust
UTAU = voicelibrary


CC = g++
FLAGS = -Wall -Wextra -pedantic -std=c++17 -Wno-sign-compare -lfftw3 -lm -lstdc++fs
DEBUG_FLAGS = -D_GLIBCXX_DEBUG
ODIR = obj
SDIR = src
SOURCES = $(wildcard $(SDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SDIR)/%.cpp=$(ODIR)/%.o)


.PHONY: all clean pls release
all: debug

release: $(OUT)

debug: FLAGS += $(DEBUG_FLAGS)
debug: $(SOUND) $(DATA)

$(ODIR)/%.o: $(SDIR)/%.cpp
	mkdir -p $(ODIR)
	$(CC) -c $< -o $@ $(FLAGS) 

$(OUT): $(OBJECTS) 
	$(CC) -o $(OUT) $(OBJECTS) $(FLAGS) 

$(DATA) $(SOUND): $(OUT) $(UST) $(UTAU)
	rm -f $(SOUND)
	./$(OUT) $(UST) $(UTAU) $(SOUND) > $(DATA)

$(GRAPH): $(DATA) $(PLOT)
	gnuplot -e 'dada="'$(DATA)'";graf="'$(GRAPH)'"' $(PLOT)


clean:
	rm -f $(ODIR)/* $(OUT) $(DATA) $(GRAPH) $(SOUND)

pls: clean  all
