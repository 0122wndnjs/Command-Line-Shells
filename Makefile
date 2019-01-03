CFLAGS = -Wall -g
CC     = gcc $(CFLAGS)

commando : commando.o cmd.o cmdctl.o util.o commando.h
	$(CC) -o $@ $^

binary_tests : binary_tests.o cmd.o cmdctl.o test_utils.o
	$(CC) -o $@ $^

test-binary : binary_tests
	valgrind ./binary_tests

test-shell : commando
	./shell_tests.sh

sleep_print : sleep_print.c
	$(CC) -o $@ $<

binary_tests.o : binary_tests.c commando.h tests.h
	$(CC) -c $<

test_utils.o : test_utils.c tests.h
	$(CC) -c $<

cmd.o : cmd.c commando.h
	$(CC) -c $<

cmdctl.o : cmdctl.c commando.h
	$(CC) -c $<

util.o : util.c commando.h
	$(CC) -c $<

clean:
	rm -f *.o actual.txt expect.txt valgrind.txt

cleanall:
	rm -f *.o actual.txt expect.txt valgrind.txt binary_tests commando sleep_print
