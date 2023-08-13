CC = gcc
CFLAGS = -shared -fPIC -ldl

# 源文件和目标文件目录
SRCDIR = ./src
OBJDIR = ./shared

# 要编译的源文件和目标文件
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.so,$(SOURCES))

all : $(OBJECTS)

$(OBJDIR)/%.so : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS)