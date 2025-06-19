USE_DEBUG = NO
USE_64BIT = NO

ifeq ($(USE_64BIT),YES)
TOOLS=c:\tdm64\bin
else
TOOLS=d:\tdm32\bin
endif

ifeq ($(USE_DEBUG),YES)
CFLAGS=-Wall -O -g
else
CFLAGS=-Wall -O3 -s
endif
CFLAGS += -Weffc++ 

all: swdf.exe

clean:
	rm -f *.exe

lint:
	cmd /C "c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) -ic:\lint9 mingw.lnt -os(_lint.tmp) swdf.cpp"

swdf.exe: swdf.cpp
	$(TOOLS)\g++ $(CFLAGS) $< -o $@

