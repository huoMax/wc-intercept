CC = g++

# 源文件和目标文件目录
SRCDIR = ./
OBJDIR = obj

# 要编译的源文件和目标文件
SOURCES = $(wildcard $(SRCDIR)/*.cc)
OBJECTS = $(patsubst $(SRCDIR)/%.cc,$(OBJDIR)/%.o,$(SOURCES))

tracer : $(OBJECTS)
	$(CC) $^ -o tracer -lelf
	mv tracer ./obj/
	rm $(OBJDIR)

$(OBJDIR)/%.o : $(SRCDIR)/%.cc
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJECTS)

.PHONY: clean tracer tracee backend