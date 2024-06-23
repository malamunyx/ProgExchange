CC				=gcc
CFLAGS			=-Wall -Werror -Wvla -O0 -std=c11 -g -fsanitize=address,leak
LDFLAGS			=-lm
BINARIES		=pe_exchange pe_trader
EXCHANGE_FILES	=pe_exchange.c modules/*

TEST_BINARIES=trader-test request-test rbuf-test product-test orderbook-test test_autotrader

all: $(BINARIES)

# $^ means right of colon, $@ means left of colon
pe_exchange: $(EXCHANGE_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -f $(BINARIES) $(TEST_BINARIES) *.o 

.PHONY: tests
tests:
# Unittests
	$(CC) modules/trader.c tests/trader-test.c -o trader-test -L"./tests/cmocka/" -lcmocka-static
	$(CC) modules/request.c tests/request-test.c -o request-test -L"./tests/cmocka/" -lcmocka-static
	$(CC) modules/rbuf.c tests/rbuf-test.c -o rbuf-test -L"./tests/cmocka/" -lcmocka-static
	$(CC) modules/product.c tests/product-test.c -o product-test -L"./tests/cmocka/" -lcmocka-static
	$(CC) modules/* tests/orderbook-test.c -o orderbook-test -L"./tests/cmocka/" -lcmocka-static
# E2E
	$(CC) $(CFLAGS) $(LDFLAGS) tests/traders/test_autotrader.c -o test_autotrader

run_tests:
	./trader-test
	./request-test
	./rbuf-test
	./product-test
	./orderbook-test
	./pe_exchange products.txt test_autotrader pe_trader | diff - tests/out/autotrader_test.out
