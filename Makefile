#all:pls
#main: tranpose fileio
#	g++ -lfftw3 -lm -Wall -Wextra -o main src/main.cpp
#	./main
#transpose: fileio
#	g++ -Wall -Wextra -c src/transpose.cpp
#	g++ -lfftw3 -lm -Wall -Wextra -o tr transpose.o fileio.o
#fileio:
#	g++ -Wall -Wextra -c src/fileio.cpp

OUT = tr
CC = g++
FLAGS = -Wall -Wextra -lfftw3 -lm
ODIR = obj
SDIR = src
SOURCES = $(wildcard $(SDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SDIR)/%.cpp=$(ODIR)/%.o)


$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CC) $(FLAGS) -c $< -o $@

$(OUT): $(OBJECTS)
	$(CC) $(FLAGS) -o $(OUT) $(OBJECTS)


.PHONY: clean
clean:
	rm -f $(ODIR)/* $(OUT)
