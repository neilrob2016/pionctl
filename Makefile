CC=cc

# -fsanitize is for runtime OOB memory checking. Debug use only as it has a
# severe speed penalty and doesn't work with -O. Linker needs it too
#SANI=-fsanitize=address

# ARM Linux
#CC=aarch64-poky-linux-gcc --sysroot=/opt/fsl-imx-wayland/4.19-warrior/sysroots/aarch64-poky-linux

ARGS=-Wall -Wextra -pedantic -g -c $(SANI)
#ARGS=-Wall -pedantic -g -c $(SANI)
#ARGS=-Wall -pedantic -g -c -O
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
	reverse.o \
	cmdfile.o \
	misc.o
BIN=pionctl

$(BIN): $(OBJS) Makefile
	$(CC) $(OBJS) $(SANI) -o $(BIN)

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

macros.o: macros.c $(DEPS)
	$(COMP) macros.c

strings.o: strings.c $(DEPS)
	$(COMP) strings.c

time.o: time.c $(DEPS)
	$(COMP) time.c

reverse.o: reverse.c $(DEPS)
	$(COMP) reverse.c

cmdfile.o: cmdfile.c $(DEPS)
	$(COMP) cmdfile.c

# Always rebuild so get correct build date
.PHONY: misc.o
misc.o: 
	$(COMP) misc.c

clean:
	rm -f *.o $(BIN) core *.jpg *.bmp
