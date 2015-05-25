CXXFLAGS=-std=c++0x

TESTS=Tokenizer_test

GTEST_DIR=../gtest-1.7.0
TEST_CFLAGS=-I$(GTEST_DIR)/include
TEST_LIBS=$(GTEST_DIR)/lib/.libs/libgtest.a $(GTEST_DIR)/lib/.libs/libgtest_main.a

all:

test: $(TESTS)
	for test in $(TESTS); do ./$${test}; done

Tokenizer_test: Tokenizer_test.cc Tokenizer.o
	$(CXX) $(CXXFLAGS) $(TEST_CFLAGS) $^ $(TEST_LIBS) -o $@

clean:
	rm -f *.o $(TESTS)

.PHONY: all clean
