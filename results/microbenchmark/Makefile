CC=gcc
QUARKLIBIO_SRC_DIR = $(abspath ../../quarkstore/quarklibio)
QUARKLIBIO_DIR = $(QUARKLIBIO_SRC_DIR)/build
LIBS=-lpthread -lrt -lquarklibio

all: microbenchmark concatenate

microbenchmark:
	$(CC) microbenchmark.c -o microbenchmark $(LIBS) -L$(QUARKLIBIO_DIR) -I$(QUARKLIBIO_SRC_DIR) -g

concatenate:
	$(CC) concatenate.c -o concatenate $(LIBS) -L$(QUARKLIBIO_DIR) -I$(QUARKLIBIO_SRC_DIR) -g

install:
	bash ../../experiments/tests/quarkcontroller_uninstall.sh
	clear
	bash ../../experiments/tests/quarkcontroller_install.sh 0

run: install
	LD_LIBRARY_PATH=../../quarkstore/quarklibio/build ./concatenate

test: install
	# make -C ../../quarkstore/quarkcontroller clean
	# make -C ../../quarkstore/quarkcontroller
	# make -C ../../quarkstore/quarklibio
	LD_LIBRARY_PATH=../../quarkstore/quarklibio/build ./concatenate

clean:
	rm -rf *.o
	rm -rf concatenate microbenchmark 