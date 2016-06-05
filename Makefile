#all:pls
#main: tranpose fileio
#	g++ -lfftw3 -lm -Wall -Wextra -o main src/main.cpp
#	./main
#transpose: fileio
#	g++ -Wall -Wextra -c src/transpose.cpp
#	g++ -lfftw3 -lm -Wall -Wextra -o tr transpose.o fileio.o
#fileio:
#	g++ -Wall -Wextra -c src/fileio.cpp

OUT = test
DATA = data
SOUND = voicelibrary/_にゃ.wav
NUVOSOUND = output.wav
GRAPH = graph.png
PLOT = plot.gpi

CC = g++
FLAGS = -Wall -Wextra -lfftw3 -lm
ODIR = obj
SDIR = src
SOURCES = $(wildcard $(SDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SDIR)/%.cpp=$(ODIR)/%.o)

.PHONY: all
all: clean $(GRAPH) 

$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CC) $(FLAGS) -c $< -o $@

$(OUT): $(OBJECTS) 
	$(CC) $(FLAGS) -o $(OUT) $(OBJECTS)

$(DATA) $(NUVOSOUND): $(OUT) $(SOUND)
	./$(OUT) $(SOUND) $(NUVOSOUND) > $(DATA)

$(GRAPH): $(DATA) $(PLOT)
	gnuplot -e 'dada="'$(DATA)'";graf="'$(GRAPH)'"' $(PLOT)

.PHONY: clean
clean:
	rm -f $(ODIR)/* $(OUT) $(DATA) $(GRAPH) $(NUVOSOUND)
