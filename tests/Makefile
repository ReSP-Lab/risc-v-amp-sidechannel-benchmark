TESTFLAGS=-fprofile-arcs -ftest-coverage


all: test-c test-py

test-c:
	gcc $(TESTFLAGS) -I../include/ ../src/stats.c test-stats.c -o test -lm
	./test
	gcov -c -p test-stats

test-py:
	python -m unittest test-stats.py

clean:
	rm -f test *.gcov *.gcda *.gcno
