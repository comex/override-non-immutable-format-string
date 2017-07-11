onifs.dylib: onifs.c Makefile
	clang -dynamiclib -o $@ $<
clean:
	rm -f onifs.dylib
