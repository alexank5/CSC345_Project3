all:
	gcc -o main_pr1 main_pr1.c -lpthread
	gcc -o main_pr2 main_pr2.c -lpthread

clean:
	rm main_pr1
	rm main_pr2
