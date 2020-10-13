Owlisp: Owlisp.cpp
	clang++ -std=c++20 Owlisp.cpp -o Owlisp
run-interp: Owlisp
	./Owlisp -i
clean:
	rm Owlisp
