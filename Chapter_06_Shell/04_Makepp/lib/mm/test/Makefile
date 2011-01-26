
PLATFORM = i386

INCLUDES := ../../.. ../../../kernel ../../../arch/$(PLATFORM)

CMACROS := PLATFORM="\"$(PLATFORM)\"" DEBUG

CC = gcc

CFLAGS = -O -g -lrt

ff: test.c ../ff_simple.c ../ff_simple.h
	@$(CC) $(CFLAGS) $< -o $@ \
		$(foreach INC,$(INCLUDES),-I$(INC)) \
		$(foreach MACRO,$(CMACROS),-D $(MACRO)) -D FIRST_FIT

gma: test.c ../gma.c ../gma.h
	@$(CC) $(CFLAGS) $< -o $@ \
		$(foreach INC,$(INCLUDES),-I$(INC)) \
		$(foreach MACRO,$(CMACROS),-D $(MACRO)) -D GMA

clean:
	-rm ff gma