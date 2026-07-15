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

# abaco_test.c é o helper do test-runner (não é uma suíte em si), linkado em cada teste
TEST_HELPER_OBJ = $(BUILDDIR)/abaco_test.o
TEST_SOURCES = $(filter-out $(TESTDIR)/abaco_test.c, $(wildcard $(TESTDIR)/*.c))
TEST_BINS = $(patsubst $(TESTDIR)/%.c, $(BUILDDIR)/%.test, $(TEST_SOURCES))

all: $(LIB) tests

$(LIB): $(OBJECTS) | $(BUILDDIR)
	$(AR) rcs $@ $(OBJECTS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_HELPER_OBJ): $(TESTDIR)/abaco_test.c $(TESTDIR)/abaco_test.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.test: $(TESTDIR)/%.c $(LIB) $(TEST_HELPER_OBJ) | $(BUILDDIR)
	$(CC) $(CFLAGS) $< $(TEST_HELPER_OBJ) $(LIB) -o $@ $(LDFLAGS)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

tests: $(TEST_BINS)

run-tests: tests
	@status=0; \
	for t in $(TEST_BINS); do \
		echo "Executando $$t:"; \
		$$t || status=1; \
	done; \
	exit $$status

clean:
	rm -rf $(BUILDDIR)

.PHONY: all tests run-tests clean
