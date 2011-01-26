# Building system script (for 'make')

#------------------------------------------------------------------------------
# Common configuration that affects both kernel and user programs
#------------------------------------------------------------------------------

OS_NAME = "ByTheBookOS"
NAME_MAJOR := $(shell basename `cd ..; pwd -P`)
NAME_MINOR := $(shell basename `pwd -P`)
PROJECT := $(NAME_MINOR)

PLATFORM = i386
VERSION = 1.0
AUTHOR = leonardo@zemris.fer.hr

# Intermediate and output files are placed into BUILDDIR
BUILDDIR = build

CDIMAGE = $(BUILDDIR)/$(PROJECT).iso

CMACROS = OS_NAME="\"$(OS_NAME)\"" PROJECT="\"$(PROJECT)\"" \
	  NAME_MAJOR="\"$(NAME_MAJOR)\"" NAME_MINOR="\"$(NAME_MINOR)\"" \
	  PLATFORM="\"$(PLATFORM)\"" AUTHOR="\"$(AUTHOR)\"" \
	  VERSION="\"$(VERSION)\""

LIBS = lib lib/mm

#------------------------------------------------------------------------------
# Devices

DEVICES = VGA_TEXT I8042 I8259 I8253 UART

DEVICES_DEV = vga_text_dev uart_com1 i8042_dev

comma := ,
empty :=
space := $(empty) $(empty)
DEV_VARS := $(subst $(space),$(comma),$(DEVICES_DEV))
DEV_PTRS := $(addprefix \&,$(DEVICES_DEV))
DEV_PTRS := $(subst $(space),$(comma),$(DEV_PTRS))

CMACROS += $(DEVICES) DEVICES_DEV=$(DEV_VARS) DEVICES_DEV_PTRS=$(DEV_PTRS) \
	IC_DEV=i8259 TIMER=i8253 K_INITIAL_STDOUT=vga_text_dev		   \
	K_STDOUT="\"COM1\"" U_STDOUT="\"VGA_TXT\"" U_STDIN="\"i8042\""
#	K_STDOUT="\"VGA_TXT\"" U_STDOUT="\"COM1\"" U_STDIN="\"COM1\""

#------------------------------------------------------------------------------
# Memory
# Memory allocators: 'gma' and/or 'first_fit'

FIRST_FIT = 1
GMA = 2

CMACROS += FIRST_FIT=$(FIRST_FIT) GMA=$(GMA)

# Maximum system resources
MAX_RESOURCES = 1000
CMACROS += MAX_RESOURCES=$(MAX_RESOURCES)

#------------------------------------------------------------------------------
# Threads

CMACROS += MAX_THREADS=256 PRIO_LEVELS=64 THR_DEFAULT_PRIO=20
CMACROS += KERNEL_STACK_SIZE=0x1000 DEFAULT_THREAD_STACK_SIZE=0x1000 \
	PROG_HEAP_SIZE=0x10000

OPTIONALS := MESSAGES

CMACROS += $(OPTIONALS)
#------------------------------------------------------------------------------
all: $(CDIMAGE)

include Makefile.kernel
# Variables defined in *.kernel: KERNEL_IMG KERNEL_FILE_NAME OBJS_K DEPS_K

include Makefile.progs
# Variables defined in *.progs: PROGRAMS PROGRAMS_BIN BUILD_U OBJS_U DEPS_U

BOOTCD := $(BUILDDIR)/cd
GRUBMENU := $(BOOTCD)/boot/grub/menu.lst
GRUBFILE := $(BOOTCD)/boot/grub/stage2_eltorito
GRUBFILE_ORIG := arch/$(PLATFORM)/grub_file

$(GRUBFILE):
	@-if [ ! -e $(BOOTCD) ]; then mkdir -p $(BOOTCD)/boot/grub ; fi;
	@cp -a $(GRUBFILE_ORIG) $(GRUBFILE)

$(GRUBMENU): $(KERNEL_IMG) $(PROGRAMS_BIN)
	@-if [ ! -e $(BOOTCD) ]; then mkdir -p $(BOOTCD)/boot/grub ; fi;
	@echo "default 0" > $(GRUBMENU)
	@echo "timeout=0" >> $(GRUBMENU)
	@echo "title $(PROJECT)" >> $(GRUBMENU)
	@echo "root (cd)" >> $(GRUBMENU)
	@echo "kernel /boot/$(KERNEL_FILE_NAME)" >> $(GRUBMENU)
	@echo "module /boot/$(PROG).bin.gz prog_name=$(PROG)" >> $(GRUBMENU); )
	@echo "boot" >> $(GRUBMENU)

# ISO CD image for booting (with grub as boot loader and $(KERNEL) as OS image)
$(CDIMAGE): $(KERNEL_IMG) $(PROG_BIN) $(GRUBFILE) $(GRUBMENU)
	@cp $(KERNEL_IMG) $(BOOTCD)/boot/$(KERNEL_FILE_NAME)
	@gzip -c $(PROG_BIN) > $(BOOTCD)/boot/$(PROG).bin.gz
	@mkisofs -m '.svn' -J -R -b boot/grub/stage2_eltorito		\
	-no-emul-boot -boot-load-size 4 -boot-info-table -V $(PROJECT)	\
	-A $(PROJECT) -o $(CDIMAGE) $(BOOTCD) 2> /dev/null
	@echo
	@echo ISO CD image: $(CDIMAGE)
	@echo


OBJECTS = $(OBJS_K) $(OBJS_U)
DEPS = $(DEPS_K) $(DEPS_U)

clean:
	@echo Cleaning.
	@-rm -f $(OBJECTS) $(DEPS) $(CDIMAGE) $(PROGRAMS_BIN) $(KERNEL_IMG)

clean_all cleanall:
	@echo Removing build directory!
	@-rm -rf $(BUILDDIR)

# starting compiled system in 'qemu' emulator
qemu: $(CDIMAGE)
	@echo Starting...
	@-qemu -no-kvm -cdrom $(CDIMAGE) -serial stdio

# Debugging - just example! run it manually in different consoles
# for debugging to work: include '-g' in CFLAGS and omit -s and -S from LDFLAGS
qemu_debug: $(CDIMAGE)
	@echo Starting...
	@-qemu -s -S -no-kvm -cdrom $(CDIMAGE) &
	@-gdb -s $(KERNEL_IMG) -ex 'target remote localhost:1234'

-include $(DEPS)
