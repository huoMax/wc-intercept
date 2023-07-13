CC = gcc
CFLAGS = -I./include/ -I./backend

INCLUDES = -I./include/
BACKEND_INCLUDES =  -I./backend

# 源文件和目标文件目录
SRCDIR = src
OBJDIR = obj
BACKENDDIR = backend
EXAMPLEDIR = example
LIBOBJDIR = dym_lib

# 要编译的源文件和目标文件
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

# 要编译的后端处理逻辑代码
BACKEND_SOURCES = $(wildcard $(BACKENDDIR)/*.c)

# 应用示例代码
EXAMPLE_SOURCES = $(wildcard $(EXAMPLEDIR)/*.c)

tracer : $(OBJECTS) wc_backend.so
	$(CC)  $^ -o tracer -lelf -L.

backend : $(BACKEND_SOURCES)
	$(CC) -c -fPIC $(CFLAGS) $^ -o wc_backend.o
	$(CC) -shared -fPIC wc_backend.o -o wc_backend.so
	rm wc_backend.o

wc_backend.so : $(BACKEND_SOURCES)
	$(CC) -c -fPIC $(CFLAGS) $^ -o wc_backend.o
	$(CC) -shared -fPIC wc_backend.o -o wc_backend.so

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

tracee : $(EXAMPLEDIR)/tracee.c
	$(CC) $^ -o tracee

clean:
	rm $(OBJECTS)

.PHONY: clean tracer tracee backend
