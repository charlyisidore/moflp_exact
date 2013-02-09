CC = g++
CFLAGS = -O2 -Iinclude -ansi -Wall -pedantic
LDFLAGS = -O2 -lscip -lobjscip -lzimpl -lnlpi.cppad -llpispx -lsoplex -lz -lgmp -lreadline -lncurses

EXEC = moflp_exact

SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)

all : $(EXEC)

$(EXEC) : $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

src/%.o : src/%.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

clean :
	rm -f $(EXEC) $(OBJ)

mrproper:
	find . -name '*~' -print0 | xargs -0 -r rm

archive :
	tar -czf $(EXEC)_`date --rfc-3339='date'`.tar.gz --ignore-failed-read makefile README.md COPYING include/*.hpp src/*.cpp

