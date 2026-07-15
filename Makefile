CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I./include
LDFLAGS = -lm
AR = ar

SRCDIR = src
TESTDIR = tests
BUILDDIR = build

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SOURCES))

LIB = $(BUILDDIR)/libabaco.a

TEST_SOURCES = $(wildcard $(TESTDIR)/*.c)
TEST_BINS = $(patsubst $(TESTDIR)/%.c, $(BUILDDIR)/%.test, $(TEST_SOURCES))

all: $(LIB) tests

$(LIB): $(OBJECTS) | $(BUILDDIR)
	$(AR) rcs $@ $(OBJECTS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.test: $(TESTDIR)/%.c $(LIB) | $(BUILDDIR)
	$(CC) $(CFLAGS) $< $(LIB) -o $@ $(LDFLAGS)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

tests: $(TEST_BINS)

run-tests: tests
	@for t in $(TEST_BINS); do echo "Executando $$t:"; $$t; done

clean:
	rm -rf $(BUILDDIR)

.PHONY: all tests run-tests clean
