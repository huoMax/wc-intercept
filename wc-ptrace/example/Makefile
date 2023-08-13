CC = gcc

# 源文件和目标文件目录
SRCDIR = .
OBJDIR = ./bin

# 要编译的源文件和目标文件
SOURCES = $(wildcard $(SRCDIR)/*.cc)
OBJECTS = $(patsubst $(SRCDIR)/%.cc,$(OBJDIR)/%.so,$(SOURCES))
BINS = $(patsubst $(SRCDIR)/%.cc, $(OBJDIR)/%, $(SOURCES))

all: $(BINS)

%: %.cc
	$(CC) $< -o $@

clean:
	rm $(BINS)

.PHONY: BINS