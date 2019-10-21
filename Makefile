GRAPHBLAS=deps/GraphBLAS/build/libgraphblas.a

$(GRAPHBLAS):
ifeq (,$(wildcard $(GRAPHBLAS)))
	@$(MAKE) -C deps/GraphBLAS CMAKE_OPTIONS="-DCMAKE_C_COMPILER='$(CC)' -DCMAKE_CXX_COMPILER='$(CXX)'" static_only
endif
.PHONY: $(GRAPHBLAS)


SOURCEDIR=$(shell pwd -P)
CC_SOURCES = $(wildcard $(SOURCEDIR)/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/cfpq_algorithms/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/grammar/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/graph/*.c)

run_all: all
	./main

all: main.c $(GRAPHBLAS)
	gcc -o main ${CC_SOURCES} -fopenmp $(GRAPHBLAS) -lm

clang: $(GRAPHBLAS)
	clang -o main ${CC_SOURCES} -fopenmp=libiomp5 $(GRAPHBLAS) -lm
