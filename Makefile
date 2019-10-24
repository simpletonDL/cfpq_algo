GRAPHBLAS=deps/GraphBLAS/build/libgraphblas.a

SOURCEDIR=$(shell pwd -P)
CC_SOURCES = $(wildcard $(SOURCEDIR)/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/cfpq_algorithms/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/grammar/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/graph/*.c)
CC_SOURCES += $(wildcard $(SOURCEDIR)/utils/*.c)

run: all
	./main

all: $(GRAPHBLAS) $(CC_SOURCES)
	clang -o main ${CC_SOURCES} -fopenmp $(GRAPHBLAS) -lm

$(GRAPHBLAS):
ifeq (,$(wildcard $(GRAPHBLAS)))
	@$(MAKE) -C deps/GraphBLAS CMAKE_OPTIONS="-DCMAKE_C_COMPILER='clang' -DCMAKE_CXX_COMPILER='clang++'" static_only
endif
.PHONY: $(GRAPHBLAS)