CC = gcc
CFLAGS = -shared -fPIC -ldl

# 源文件和目标文件目录
SRCDIR = .
OBJDIR = ./bin

# 要编译的源文件和目标文件
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%,$(SOURCES))

all : test_dynamic test_rw

test_dynamic : test_dynamic.cc
	g++ test_dynamic.cc -o test_dynamic
	mv test_dynamic ./bin

test_rw : test_rw.cc
	g++ test_rw.cc -o test_rw
	mv test_rw ./bin

clean:
	rm $(OBJECTS)