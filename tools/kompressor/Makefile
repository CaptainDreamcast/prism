# TODO: refactor common functionality as lib

SOURCE_FILES= main.c ../../quicklz.c ../common/argumentparser.c ../common/scriptreader.c

all:
	gcc -I ../../include -Wall $(SOURCE_FILES) -o kompressor
	mkdir -p ../bin
	mv kompressor ../bin/kompressor
	
clean:
	rm *.o
	rm ../bin/kompressor
