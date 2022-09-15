CCFLAGS :=

# os detection:
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CCFLAGS += -I /usr/include/ -l m
else ifeq ($(UNAME_S),Darwin)
	CCFLAGS += -I /usr/local/Cellar/libebur128/1.2.6/include/
else
$(error Unknown or unsupported platform)
endif

all: ebumcli
ebumcli:
	gcc $(CCFLAGS) -l ebur128 -l sndfile ebumcli.c -o ebumcli
clean:
	$(RM) ebumcli
