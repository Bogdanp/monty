CFLAGS := -Iinclude

BUILDDIR = build
SOURCEDIR = src
SOURCES = $(wildcard $(SOURCEDIR)/*.c)
OBJECTS = $(patsubst $(SOURCEDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))

.PHONY: all
all: build monty $(OBJECTS)

.PHONY: clean
clean:
	rm -rf build monty tests/build

monty: $(OBJECTS) monty.c
	$(CC) $(CFLAGS) $^ -o $@

build:
	mkdir -p build

$(OBJECTS): $(BUILDDIR)/%.o: $(SOURCEDIR)/%.c
	$(CC) -c $(CFLAGS) $^ -o $@

TESTBUILDDIR = tests/build
TESTSOURCEDIR = tests
TESTSOURCES = $(wildcard $(TESTSOURCEDIR)/*.c)
TESTOBJECTS = $(patsubst $(TESTSOURCEDIR)/%.c,$(TESTBUILDDIR)/%,$(TESTSOURCES))

.PHONY: tests
tests: build tests/build $(OBJECTS) $(TESTOBJECTS)
	./tests/build/test_scanner

tests/build:
	mkdir -p tests/build

$(TESTOBJECTS): $(TESTBUILDDIR)/%: $(OBJECTS) $(TESTSOURCEDIR)/%.c
	$(CC) $(CFLAGS) $^ -o $@
