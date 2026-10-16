# Harvest Moon: Back to Nature (US, SLUS_011.15) -- matching decompilation.
#
# Domain modules are compiled through ten unity units to retain the proven
# GCC 2.95 translation-unit boundaries.
#
# Run inside the btnx-decomp:latest image (see docker/cmd.ps1 or docker/cmd.sh),
# from /work. First time: put your baserom in place, then `make setup`.

NAME     := slus_011.15
BASEROM  := baserom/SLUS_011.15

BUILD    := build
ASM      := asm
INCLUDE  := include

LD_SCRIPT := $(BUILD)/$(NAME).ld
SOURCE_INPUTS := $(shell find src include -type f \( -name '*.c' -o -name '*.h' \))
RETAINED_ASM_INPUTS := $(shell find asm/nonmatchings -type f -name '*.s' 2>/dev/null)
ELF       := $(BUILD)/$(NAME).elf
TARGET    := $(BUILD)/SLUS_011.15
MAP       := $(BUILD)/$(NAME).map

CROSS   := mipsel-linux-gnu-
AS      := $(CROSS)as
LD      := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy

# -no-pad-sections keeps gas from rounding section sizes up, which would shift
# everything downstream of .text and break the byte match.
ASFLAGS := -EL -march=r3000 -mabi=32 -I$(INCLUDE) -no-pad-sections

# Discovered rather than listed: .text is carved into one file per PSY-Q
# object, so the set changes whenever the config's subsegments change. The
# linker script names each one.
#
# asm/matchings/ holds splat's per-function reference disassembly for the `c`
# subsegments. It exists to be compared against, never assembled: the C unit
# supplies those functions, and assembling them too would define every one of
# them twice.
#
# One discovered file is assembled but deliberately never linked:
# asm/data/text.rodata.s, the reference disassembly of jtbl_80010144. That
# table is emitted by src/text.c, so the linker script places
# build/src/text.o(.rodata) at 0x80010144 and never mentions the reference
# object. If the C ever stopped emitting the table, the address run would shift
# and `make verify` would fail rather than silently pick the stale copy.
S_FILES := $(shell find $(ASM) \( -path '$(ASM)/overlays' -o -path '$(ASM)/matchings' -o -path '$(ASM)/nonmatchings' \) -prune -o -name '*.s' -print 2>/dev/null | sort)
O_FILES := $(addprefix $(BUILD)/,$(S_FILES:.s=.o))

# C translation units declared as `c` subsegments in $(CONFIG) and therefore
# placed by the linker script. Listed explicitly: only these files own
# linker-script slots.
LINKED_C   := src/crt0.c src/boot.c src/display.c src/system.c src/archive.c src/text.c \
              src/flags.c src/state_core.c src/gameplay.c src/ui.c
LINKED_C_O := $(addprefix $(BUILD)/,$(LINKED_C:.c=.o))

CONFIG := config/$(NAME).yaml
TEXT_BANK := src/text/text_bank.json

.PHONY: all setup split verify match test check forward-check header-check diff fndiff clean distclean
all: verify

# ---------------------------------------------------------------------------
# Setup. Everything derived from the game -- the disassembly of code and data
# that is not C yet, and the readable text bank -- is generated from YOUR
# baserom here and never committed.
# ---------------------------------------------------------------------------
$(BASEROM):
	@echo "missing $(BASEROM): run 'python3 tools/extract_baserom.py <your disc .bin>' first" >&2
	@exit 1

setup: $(BASEROM)
	$(MAKE) --no-print-directory split
	$(MAKE) --no-print-directory $(TEXT_BANK)

# The linker script and the undefined-symbol lists come from splat. Re-run this
# after changing config/ or symbols/.
split: $(BASEROM)
	splat split $(CONFIG) --disassemble-all --make-full-disasm-for-code
	@$(MAKE) --no-print-directory restore-include-asm

# `splat split` overwrites include/include_asm.h with splat's stock template,
# which does not match the self-contained block tools/asm_order.py relies on.
# Restoring it is part of splitting: the stock macro produces a different
# rodata layout.
.PHONY: restore-include-asm
restore-include-asm:
	@cp config/include/include_asm.h include/include_asm.h

# Fires only when the bank is absent, so it never overwrites your edits.
$(TEXT_BANK): | $(BASEROM)
	python3 tools/text_bank.py extract --baserom $(BASEROM) --output $@

.PHONY: require-split
require-split:
	@test -f $(ASM)/data/39314.data.s || { echo "no disassembly yet: run 'make setup'" >&2; exit 1; }

$(BUILD)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -o $@ $<

# Human-readable text is the build input; the split data supplies labels and
# all unrelated data only. Generated assembly stays under build/.
$(BUILD)/generated/text_bank.s: $(TEXT_BANK) tools/text_bank.py tools/text_codec.py asm/data/39314.data.s
	python3 tools/text_bank.py build --output $@

$(BUILD)/asm/data/39314.data.o: $(BUILD)/generated/text_bank.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -o $@ $<

# splat emits these as `name = 0xaddr;` lines, which ld accepts verbatim.
UNDEF_LD := $(BUILD)/undefined_syms.ld
$(UNDEF_LD): undefined_syms_auto.txt undefined_funcs_auto.txt
	@mkdir -p $(BUILD)
	@cat $^ > $@

# Compiled by the game's own PSY-Q 4.6 cc1 under wibo, through the same code
# path as `make match`, so a function cannot pass the gate and then be built
# differently into the executable. Objects also depend on the tools that
# produce them, so an edit to the compile path rebuilds everything.
CC_TOOLS := tools/psyq_cc.py tools/match_check.py tools/asm_order.py

$(BUILD)/src/%.o: src/%.c $(CC_TOOLS) $(SOURCE_INPUTS) tools/source_layout.py docs/retail-symbols.json $(RETAINED_ASM_INPUTS)
	@mkdir -p $(dir $@)
	python3 tools/psyq_cc.py $< --output $@ --verbose

$(ELF): $(O_FILES) $(LINKED_C_O) $(LD_SCRIPT) $(UNDEF_LD)
	$(LD) -T $(UNDEF_LD) -T $(LD_SCRIPT) -Map $(MAP) --no-check-sections -o $@

$(LD_SCRIPT): config/linker/$(NAME).ld
	@mkdir -p $(BUILD)
	cp $< $@

$(TARGET): $(ELF)
	$(OBJCOPY) -O binary $< $@

verify: require-split $(TARGET)
	@echo "expected: $$(sha1sum $(BASEROM) | cut -d' ' -f1)  (baserom)"
	@echo "built   : $$(sha1sum $(TARGET)  | cut -d' ' -f1)  (build)"
	@if cmp -s $(BASEROM) $(TARGET); then \
		echo "OK: byte-identical"; \
	else \
		echo "MISMATCH:"; \
		cmp -l $(BASEROM) $(TARGET) | wc -l | xargs echo "  differing bytes:"; \
		cmp $(BASEROM) $(TARGET) | head -1 | sed 's/^/  /'; \
		ls -l $(BASEROM) $(TARGET) | sed 's/^/  /'; \
		exit 1; \
	fi

diff: $(TARGET)
	@cmp -l $(BASEROM) $(TARGET) | head -40

# ---------------------------------------------------------------------------
# C is compiled by the game's own compiler -- CC1PSX.EXE from PSY-Q 4.6, run
# under wibo -- so the container needs your SDK mounted at /psyq.
#
#   GNU C version 2.95.2 19991024 BUILD 4.0.0030 (PSX)
#
# `make match` compiles every translation unit and checks each function against
# the bytes splat pulled out of the retail executable. It is a gate, not a
# report: a single differing instruction fails the build.
# ---------------------------------------------------------------------------
C_FILES := $(wildcard src/*.c)

test:
	python3 -m unittest discover -s tests

forward-check:
	python3 tools/forward_calls.py

header-check:
	python3 tools/check_headers.py

check: verify match test forward-check header-check

match: require-split
	@test -x /usr/local/bin/wibo || { echo "wibo missing; rebuild the image"; exit 1; }
	@test -f /psyq/BIN/CC1PSX.EXE || \
	  { echo "PSY-Q SDK not mounted at /psyq -- set PSYQ_DIR, see docker/cmd.ps1"; exit 1; }
	@rc=0; for f in $(C_FILES); do \
	  python3 tools/match_check.py $$f || rc=1; \
	done; exit $$rc

# ---------------------------------------------------------------------------
# Per-function assembly diff, via asm-differ and diff_settings.py.
#
#     make fndiff FUNC=func_80016054
#
# Diffs the LINKED images, not objects, so what it shows is the comparison
# `verify` makes. -s stops at the first `jr $ra`, which bounds the dump to the
# one function.
# ---------------------------------------------------------------------------
DIFF_PY := /opt/decomp-python/bin/python
fndiff: $(TARGET)
	@test -n "$(strip $(FUNC))" || { echo "usage: make fndiff FUNC=<name>"; exit 2; }
	@test -x $(DIFF_PY) || { echo "asm-differ missing; rebuild the image"; exit 1; }
	@$(DIFF_PY) -m diff --no-pager --format=$(if $(strip $(FMT)),$(FMT),color) \
	  -s $(DIFFFLAGS) $(FUNC)

clean:
	rm -rf $(BUILD)

# Also drops splat's generated disassembly; `make split` regenerates it. The two
# hand-kept CRT0 fragments under asm/nonmatchings/crt0 are left in place.
distclean: clean
	find $(ASM) -mindepth 1 -maxdepth 1 ! -name nonmatchings -exec rm -rf {} +
	find $(ASM)/nonmatchings -mindepth 1 -maxdepth 1 ! -name crt0 -exec rm -rf {} +
	rm -f undefined_funcs_auto.txt undefined_syms_auto.txt
