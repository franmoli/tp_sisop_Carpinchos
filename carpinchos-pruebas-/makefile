################################################################################
# Makefile para compilar las pruebas del tp CarpinchOS
################################################################################

# Agrego las libs
LIBS := -lmatelib -lpthread -lcommons

# Agrego los archivos a compilar (todos los archivos *.c en esta carpeta)
CAPY_SRCS := $(wildcard *.c)

# Agrego los nombres de los carpinchos (que van a estar en una carpeta "build")
CARPINCHOS := $(CAPY_SRCS:%.c=build/%)

# Agrego los headers de matelib
HEADERS := -I. -I./lib

# Agrego comandos extra
RM := rm -rf
CC := gcc

# compile --> Crea la carpeta "build" y compila todos los carpinchos
compile: clean build $(CARPINCHOS)

build:
	mkdir $@

build/%: %.c
	$(CC) $(HEADERS) -o "$@" $^ $(LIBS)

# clean --> Limpia todos los carpinchos
clean:
	-$(RM) build

run: rundl

run1: clean build $(CARPINCHOS)
	./build/PruebaBase_Carpincho1 cfg/matelib.cfg

run2: clean build $(CARPINCHOS)
	./build/PruebaBase_Carpincho2 cfg/matelib.cfg

run3: clean build $(CARPINCHOS)
	./build/PruebaAsignacion cfg/matelib.cfg

run4: clean build $(CARPINCHOS)
	./build/PruebaPlanificacion cfg/matelib.cfg

run5: clean build $(CARPINCHOS)
	./build/PruebaSuspension cfg/matelib.cfg

run6: clean build $(CARPINCHOS)
	./build/PruebaDeadlock cfg/matelib.cfg

run7: clean build $(CARPINCHOS)
	./build/prueba_tlb_fifo cfg/matelib.cfg

run8: clean build $(CARPINCHOS)
	./build/prueba_swamp cfg/matelib.cfg

runDL: run6

rundl: runDL

vrun: clean build $(CARPINCHOS)
	valgrind ./build/PruebaBase_Carpincho1 cfg/matelib.cfg

.PHONY:
	compile clean run

	