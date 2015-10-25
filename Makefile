#
# Makefile
# CS51 Final Project
# 
# Jason Stein and Mason Hale
#


# Rule to make executables
all: trie radix

trie: trie.o common.o
	gcc -o trie trie.o common.o
	
radix: radix.o common.o
	gcc -o radix radix.o common.o

# Rule to clean all files created by compiler
clean:
	rm -f trie radix core *.o
