OUT = re8 
DATA = data
NUVOSOUND = output.wav
GRAPH = graph.png
UST = ust.ust
UTAU = voicelibrary


CC = g++
FLAGS = -Wall -Wextra -pedantic -lfftw3 -lm -std=c++11
ODIR = obj
SDIR = src
SOURCES = $(wildcard $(SDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SDIR)/%.cpp=$(ODIR)/%.o)


.PHONY: all clean pls
all: $(NUVOSOUND) $(DATA)

$(ODIR)/%.o: $(SDIR)/%.cpp
	mkdir -p $(ODIR)
	$(CC) $(FLAGS) -c $< -o $@

$(OUT): $(OBJECTS) 
	$(CC) $(FLAGS) -o $(OUT) $(OBJECTS)

$(DATA) $(NUVOSOUND): $(OUT) $(UST) $(UTAU)
	rm -f $(NUVOSOUND)
	./$(OUT) > $(DATA)

$(GRAPH): $(DATA) $(PLOT)
	gnuplot -e 'dada="'$(DATA)'";graf="'$(GRAPH)'"' $(PLOT)


clean:
	rm -f $(ODIR)/* $(OUT) $(DATA) $(GRAPH) $(NUVOSOUND)

pls: clean  all
