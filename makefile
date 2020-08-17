CC := arm-linux-gnueabihf-gcc

usb_hidtest := usb_hidtest
hiddevtest  := hiddevtest

hiddevtestSource += hiddevtest.o
usb_hidtestSource += usb_hidraw.o \
									usb_hidtest.o 
CFLAGS += -I./include 

all: $(usb_hidtest) $(hiddevtest)

$(usb_hidtest): $(usb_hidtestSource)
	$(CC) -o $(usb_hidtest) $^

$(hiddevtest): $(hiddevtestSource)
	$(CC) -o $(hiddevtest) $^

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm *.o