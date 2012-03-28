MSG = "Don't forget to run 'export MXTOOL_XSD=MARC21slim.xsd'!"

all: mxtool mxpylib

mxtool: mxutil.c mxtool.c
	gcc -c mxutil.c mxtool.c -I/usr/include/libxml2/ -fPIC -Wall -g -std=c99
	gcc -lxml2 -o mxtool mxtool.o mxutil.o -Wall -std=c99
	echo $(MSG)


mxpylib: mxpylib.c
	gcc -c mxpylib.c -I/usr/include/libxml2/ -fPIC -I/usr/include/python2.6/ -Wall -g -std=c99
	ld -G -lxml2 mxpylib.o mxutil.o mxtool.o -o Mx.so



diff: mxdiff.c mxutil.c
	gcc -c mxutil.c mxdiff.c -I/usr/include/libxml2/ -Wall -g -std=c99
	gcc -lxml2 -o diff mxutil.o mxdiff.o -Wall -std=c99






vg-review-all: vg-review-in vg-review-out

vg-review-in:
	echo "testing with piped input to -review"
	valgrind --dsymutil=yes --leak-check=full --show-reachable=yes --suppressions=./vg-zlib.supp ./mxtool -review < Sample2.txt > vg-review-output.xml

vg-review-out:
	echo "testing with piped output from -review"
	valgrind --dsymutil=yes --leak-check=full --show-reachable=yes --suppressions=./vg-zlib.supp ./mxtool -review < Sample2.txt

vg-lib:
	valgrind --dsymutil=yes --leak-check=full --show-reachable=yes --suppressions=./vg-zlib.supp ./mxtool -lib < Sample2.txt

vg-bib:
	valgrind --dsymutil=yes --leak-check=full --show-reachable=yes --suppressions=./vg-zlib.supp ./mxtool -bib < Sample2.txt

vg-cat:
	valgrind --dsymutil=yes --leak-check=full --show-reachable=yes --suppressions=./vg-zlib.supp ./mxtool -cat

vg-keep-all: vg-keep-a vg-keep-t vg-keep-p

vg-keep-a:
	valgrind --dsymutil=yes --leak-check=full --show-reachable=yes --suppressions=./vg-zlib.supp ./mxtool -keep a=ZSan < Sample2.txt > keep-a.txt

vg-keep-t:
	valgrind --dsymutil=yes --leak-check=full --show-reachable=yes --suppressions=./vg-zlib.supp ./mxtool -keep t=Arit < Sample2.txt > keep-t.txt

vg-keep-p:
	valgrind --dsymutil=yes --leak-check=full --show-reachable=yes --suppressions=./vg-zlib.supp ./mxtool -keep p=993z < Sample2.txt > keep-p.txt
	
vg-discard:
	valgrind --dsymutil=yes --leak-check=full --show-reachable=yes --suppressions=./vg-zlib.supp ./mxtool -discard

clean:
	rm mxtool mxtool.o mxutil.o mxpylib.o
