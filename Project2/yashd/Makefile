all: yashd yash

yashd: yashd.c
	gcc -o yashd yashd.c yash_commands.c

yash: yash.c
	gcc -o yash yash.c

clean:
	rm yash
	rm yashd