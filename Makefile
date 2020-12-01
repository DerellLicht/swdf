USE_DEBUG = NO
USE_64BIT = NO

ifeq ($(USE_DEBUG),YES)
CFLAGS=-Wall -O -g
else
CFLAGS=-Wall -O3 -s
endif

ifeq ($(USE_64BIT),YES)
TOOLS=c:\tdm64\bin
else
TOOLS=c:\mingw\bin
endif

all: swdf.exe

clean:
	rm -f *.exe

lint:
	\lint9\lint-nt +v -width(160,4) -i\lint9 mingw.lnt swdf.cpp

swdf.exe: swdf.cpp
	$(TOOLS)\g++ $(CFLAGS) -Weffc++ $< -o $@

