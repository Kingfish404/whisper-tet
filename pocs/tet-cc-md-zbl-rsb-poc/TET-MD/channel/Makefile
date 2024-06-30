CC :=gcc
CPP :=g++
CFLAGS :=
CPPFLAGS :=
C_SOURCES :=$(wildcard *.c)
C_EXECUTABLE :=$(C_SOURCES:.c=.c.out)
CPP_SOURCES :=$(wildcard *.cpp)
CPP_EXECUTABLE :=$(CPP_SOURCES:.cpp=.cpp.out)
 
all:$(C_EXECUTABLE) $(CPP_EXECUTABLE)

%.c.out: %.c
	$(CC) $< -o $@ $(CFLAGS) 

%.cpp.out: %.cpp
	$(CPP) $< -o $@ $(CPPFLAGS)

asm_c:
	$(CC) -S $(CFLAGS) $(C_SOURCES)

asm_cpp:
	$(CPP) -S $(CPPFLAGS) $(CPP_SOURCES)

clean:
	rm -rf $(C_EXECUTABLE)
	rm -rf $(CPP_EXECUTABLE)
	rm -rf *.s