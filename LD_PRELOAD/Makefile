CC = gcc
CFLAGS = -I./include/ -I./backend -I./wc_lib

# 源文件和目标文件目录
SRCDIR = src
OBJDIR = obj
MYLIBSRCDIR = wc_lib
MYLIBOBJDIR = dym_lib
BACKENDDIR = backend
EXAMPLEDIR = example

# 要编译的源文件和目标文件
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

# 要编译的后端处理逻辑代码
BACKEND_SOURCES = $(wildcard $(BACKENDDIR)/*.c)

# 应用示例代码
EXAMPLE_SOURCES = $(wildcard $(EXAMPLEDIR)/*.c)

# 要编译的预加载的动态库代码
LIB_SOURCES = $(wildcard $(MYLIBDIR)/*.c)

tracer : $(OBJECTS) wc_backend.so wc_lib.so
	$(CC)  $^ -o tracer -lelf -L.
	rm wc_backend.o

tracee : $(EXAMPLEDIR)/tracee.c
	$(CC) $^ -o tracee

backend : $(BACKEND_SOURCES)
	$(CC) -c -fPIC $(CFLAGS) $^ -o wc_backend.o
	$(CC) -shared -fPIC wc_backend.o -o wc_backend.so
	rm wc_backend.o

wc_lib.so : $(LIB_SOURCES)
	$(CC) -c -fPIC $(CFLAGS) $^ -o wc_lib.o
	$(CC) -shared -fPIC wc_lib.o -o wc_lib.so

wc_backend.so : $(BACKEND_SOURCES)
	$(CC) -c -fPIC $(CFLAGS) $^ -o wc_backend.o
	$(CC) -shared -fPIC wc_backend.o -o wc_backend.so

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJECTS)

.PHONY: clean tracer tracee backend 
