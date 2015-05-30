CXXFLAGS=-std=c++0x

TESTS=Tokenizer_test

GMOCK_DIR=../gmock-1.7.0
TEST_CFLAGS=-I$(GMOCK_DIR)/gtest/include -I$(GMOCK_DIR)/include
TEST_LIBS= \
	$(GMOCK_DIR)/gtest/lib/.libs/libgtest.a \
	$(GMOCK_DIR)/lib/.libs/libgmock.a \
	$(GMOCK_DIR)/lib/.libs/libgmock_main.a

all:

test: $(TESTS)
	for test in $(TESTS); do ./$${test}; done

Tokenizer_test: Tokenizer_test.cc Tokenizer.o
	$(CXX) $(CXXFLAGS) $(TEST_CFLAGS) $^ $(TEST_LIBS) -o $@

clean:
	rm -f *.o $(TESTS)

.PHONY: all clean
