CC=cc

# ARM Linux
#CC=aarch64-poky-linux-gcc --sysroot=/opt/fsl-imx-wayland/4.19-warrior/sysroots/aarch64-poky-linux

ARGS=-Wall -pedantic -g -c -O
COMP=$(CC) $(ARGS)
DEPS=globals.h Makefile
OBJS=\
	main.o \
	keyboard.o \
	commands.o \
	network.o \
	printrx.o \
	printf.o \
	prompt.o \
	buffer.o \
	rxlist.o \
	titles.o \
	menu.o \
	saveart.o \
	macros.o \
	strings.o \
	time.o \
	misc.o
BIN=pionctl

$(BIN): build_date.h $(OBJS) Makefile
	$(CC) $(OBJS) -o $(BIN)

build_date.h:
	echo "#define BUILD_DATE \"`date -u +'%F %T %Z'`\"" > build_date.h

main.o: main.c globals.h Makefile
	$(COMP) main.c

keyboard.o: keyboard.c $(DEPS)
	$(COMP) keyboard.c

commands.o: commands.c $(DEPS)
	$(COMP) commands.c

network.o: network.c $(DEPS)
	$(COMP) network.c

printrx.o: printrx.c $(DEPS)
	$(COMP) printrx.c

printf.o: printf.c $(DEPS)
	$(COMP) printf.c

prompt.o: prompt.c $(DEPS)
	$(COMP) prompt.c

buffer.o: buffer.c $(DEPS)
	$(COMP) buffer.c

rxlist.o: rxlist.c $(DEPS)
	$(COMP) rxlist.c

titles.o: titles.c $(DEPS)
	$(COMP) titles.c

menu.o: menu.c $(DEPS)
	$(COMP) menu.c

saveart.o: saveart.c $(DEPS)
	$(COMP) saveart.c

macros.o: macros.c build_date.h $(DEPS)
	$(COMP) macros.c

strings.o: strings.c $(DEPS)
	$(COMP) strings.c

time.o: time.c $(DEPS)
	$(COMP) time.c

misc.o: misc.c build_date.h $(DEPS)
	$(COMP) misc.c

clean:
	rm -f *.o $(BIN) core *.jpg *.bmp build_date.h 
