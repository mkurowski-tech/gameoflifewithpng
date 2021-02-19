CC=/usr/bin/g++
CFLAGS=-std=c++17 -Wall -ggdb

TARGET=target
SRCPATH=src

.PHONY: clean

all:	\
	${TARGET}/gameoflife \
	
${TARGET}/gameoflife: ${SRCPATH}/gameoflife.cpp
	${CC} ${CFLAGS} ${LIBCPPINCLUDE} ${LIBCPPLIB} ${IPOSTGRE} ${SRCPATH}/gameoflife.cpp -o ${TARGET}/gameoflife -lboost_program_options -lstdc++fs -lpng
	
clean:
	rm ${TARGET}/*
	
