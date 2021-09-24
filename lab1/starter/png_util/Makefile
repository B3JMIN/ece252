# Makefile, ECE252  
# Yiqing Huang

CC = gcc       # compiler
CFLAGS = -Wall -g -std=c99 # compilation flags
LD = gcc       # linker
LDFLAGS = -g   # debugging symbols in build
LDLIBS = -lz   # link with libz

# For students
LIB_UTIL = zutil.o crc.o
SRCS   = pnginfo.c findpng.c crc.c zutil.c
OBJS = catpng.o $(LIB_UTIL)

TARGETS= pnginfo findpng catpng

all: pnginfo.out  findpng.out catpng.out findpng.out

pnginfo: pnginfo.o 
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS)

findpng: findpng.c
	$(CC) -g -o $@ $^

%.o: %.c 
	$(CC) $(CFLAGS) -c $< 

.c.o:
	$(CC) $(CFLAGS) -c $<

%.d: %.c
	gcc -MM -MF $@ $<

-include $(SRCS:.c=.d)

.PHONY: clean
clean:
	rm -f *.d *.o $(TARGETS) 
