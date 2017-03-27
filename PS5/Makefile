LEX=flex
YACC=bison
YFLAGS+=--defines=src/y.tab.h -o y.tab.c
CFLAGS+=-std=c99 -g -Isrc -Iinclude -D_POSIX_C_SOURCE=200809L -DYYSTYPE="node_t *"

src/vslc: src/vslc.c src/parser.o src/scanner.o src/nodetypes.o src/tree.o src/ir.o src/generator.o src/stack.o src/tlhash.o
src/y.tab.h: src/parser.c
src/scanner.c: src/y.tab.h src/scanner.l

clean:
	-rm -f src/parser.c src/scanner.c src/*.tab.* src/*.o vsl_programs/*.s
purge: clean
	-rm -f src/vslc
test: src/vslc
	src/vslc < vsl_programs/test_program.vsl > test_program.s
	gcc test_program.s -o test_program
test_show: src/vslc
	src/vslc < vsl_programs/test_program.vsl
all: src/vslc
	src/vslc < vsl_programs/euclid.vsl > vsl_programs/euclid.s
	src/vslc < vsl_programs/globals_and_args.vsl > vsl_programs/globals_and_args.s
	src/vslc < vsl_programs/nested_scopes.vsl > vsl_programs/nested_scopes.s
	src/vslc < vsl_programs/newton.vsl > vsl_programs/newton.s
	src/vslc < vsl_programs/prec.vsl > vsl_programs/prec.s
	src/vslc < vsl_programs/prime.vsl > vsl_programs/prime.s
	src/vslc < vsl_programs/test_program.vsl > vsl_programs/test_program.s
euclid: src/vslc
	$(info ########### Running euclid.vsl ###########)
	$(shell src/vslc < vsl_programs/euclid.vsl > a.s)
	$(shell gcc a.s -o test)
	./test 102 255
	rm -f a.s test
globals: src/vslc
	$(info ########### Running globals_and_args.vsl ###########)
	$(shell src/vslc < vsl_programs/globals_and_args.vsl > a.s)
	$(shell gcc a.s -o test)
	./test 102 144
	rm -f a.s test
nested: src/vslc
	$(info ########### Running nested_scopes.vsl ###########)
	$(shell src/vslc < vsl_programs/nested_scopes.vsl > a.s)
	$(shell gcc a.s -o test)
	./test 0
	rm -f a.s test
newton: src/vslc
	$(info ########### Running newton.vsl ###########)
	$(shell src/vslc < vsl_programs/newton.vsl > a.s)
	$(shell gcc a.s -o test)
	./test 256
	rm -f a.s test
prime: src/vslc
	$(info ########### Running prime.vsl ###########)
	$(shell src/vslc < vsl_programs/prime.vsl > a.s)
	$(shell gcc a.s -o test)
	./test
	rm -f a.s test
