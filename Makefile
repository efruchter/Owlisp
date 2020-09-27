Owlisp: Owlisp.cpp
	clang++ Owlisp.cpp -o Owlisp
run-interp: Owlisp
	./Owlisp -i
clean:
	rm Owlisp
