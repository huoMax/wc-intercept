CC = gcc
CFALGS = -shared -fPIC
SRC_PATH = $(shell pwd)/src
SRC_FILE = $(wildcard $(SRC_PATH)/*.c)
ENV = /usr/bin/mylibc

libc_intercept : $(SRC_FILE)
	$(CC) $(CFALGS) $^ -o libc_intercept.so
	$(shell if [! -d $(ENV)]; then mkdir $(ENV); fi)
	mv libc_intercept.so $(ENV)
	cp libc_intercept.sh $(ENV)
	ln -s $(ENV)/libc_intercept.sh $(ENV)/../libc_intercept