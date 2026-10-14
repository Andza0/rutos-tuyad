BIN:=tuyad
SRCS := $(wildcard *.c)	
OBJECTS = $(SRCS:.c=.o)

CFLAGS := -Wall -pedantic -g 
LDFLAGS := -lcjson -llink_core -lmiddleware_implementation -lplatform_port -lutils_modules -largp -lblobmsg_json -lubox -lubus

all: clean $(BIN)
.PHONY: all

$(BIN): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(BIN) $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(BIN)
.PHONY: clean
