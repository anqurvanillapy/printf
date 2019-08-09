.PHONY: clean

printf.cc.out: printf.cc
	${CXX} -std=c++17 -Wall -Wextra -Wpedantic -Ofast $< -o $@

clean:
	rm -rf *.out
