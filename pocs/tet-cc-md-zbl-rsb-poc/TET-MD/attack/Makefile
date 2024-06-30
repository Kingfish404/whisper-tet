CC :=gcc
CPP :=g++
LIB_PATH :=.
CFLAGS :=-Wall -O0 -I$(LIB_PATH) -pthread
CPPFLAGS :=-Wall -O0 -I$(LIB_PATH) -pthread
LIBC = $(wildcard $(LIB_PATH)/*/*.c)
C_SOURCES :=$(wildcard *.c)
C_EXECUTABLE :=$(C_SOURCES:.c=.c.out)
CPP_SOURCES :=$(wildcard *.cpp)
CPP_EXECUTABLE :=$(CPP_SOURCES:.cpp=.cpp.out)
 
all:$(C_EXECUTABLE) $(CPP_EXECUTABLE)

build:
ifeq ($(S),)
	@echo "Usage: make build S=source_file"
	@echo "Specify a source file to build"
	@echo "    Example: make build S=main.c"
	@echo "    Example: make build S=main.cpp"
else
ifeq ($(shell echo $(S) | grep -c '\.cpp'), 1)
	rm -rf $(S:.cpp=.cpp.out)
	$(CPP) $(CPPFLAGS) $(LIBC) $(S) -o $(S:.cpp=.cpp.out)
else
	rm -rf $(S:.c=.c.out)
	$(CC) $(CFLAGS) $(LIBC) $(S) -o $(S:.c=.c.out)
endif
endif

%.c.out: %.c
	$(CC) $(CFLAGS) $(LIBC) $< -o $@

%.cpp.out: %.cpp
	$(CPP) $(CPPFLAGS) $(LIBC) $< -o $@

clean:
	rm -rf $(C_EXECUTABLE)
	rm -rf $(CPP_EXECUTABLE)

secret:
	sudo taskset -c 1 ./secret.c.out

attack:
	taskset -c 5 ./tet_time_demo.c.out [PHYSICAL_ADDRESS_OF_SECRET]
