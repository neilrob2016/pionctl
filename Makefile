CC=cc

# For Helvar 950 Router build
#CC=aarch64-poky-linux-gcc --sysroot=/opt/fsl-imx-wayland/4.19-warrior/sysroots/aarch64-poky-linux

ARGS=-Wall -pedantic -g -c
COMP=$(CC) $(ARGS)
DEPS=globals.h Makefile
OBJS=\
	main.o \
	keyboard.o \
	commands.o \
	network.o \
	print.o \
	buffer.o \
	list.o \
	titles.o \
	menu.o \
	saveart.o \
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

print.o: print.c $(DEPS)
	$(COMP) print.c

buffer.o: buffer.c $(DEPS)
	$(COMP) buffer.c

list.o: list.c $(DEPS)
	$(COMP) list.c

titles.o: titles.c $(DEPS)
	$(COMP) titles.c

menu.o: menu.c $(DEPS)
	$(COMP) menu.c

saveart.o: saveart.c $(DEPS)
	$(COMP) saveart.c

misc.o: misc.c build_date.h $(DEPS)
	$(COMP) misc.c

clean:
	rm -f *.o $(BIN) core *.jpg *.bmp build_date.h 
