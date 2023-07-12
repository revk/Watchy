#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := Watchy
SUFFIX := $(shell components/ESP32-RevK/buildsuffix)

all:
	@echo Make: build/$(PROJECT_NAME)$(SUFFIX).bin
	@idf.py build
	@cp build/$(PROJECT_NAME).bin $(PROJECT_NAME)$(SUFFIX).bin
	@echo Done: build/$(PROJECT_NAME)$(SUFFIX).bin

issue:
	-git pull
	-git submodule update --recursive
	-git commit -a -m checkpoint
	@make set
	cp $(PROJECT_NAME)*.bin release
	git commit -a -m release
	git push

main/icons.h: $(patsubst %.svg,%.h,$(wildcard icons/*.svg))
	grep -h const icons/*.h | sed 's/const unsigned char icon_\([A-Za-z0-9]*\).*/extern const unsigned char icon_\1[];extern const unsigned char icon_\1_size;/' > main/icons.h

main/icons.c: $(patsubst %.svg,%.h,$(wildcard icons/*.svg))
	cat icons/*.h > main/icons.c

icons/%.png:    icons/%.svg
	inkscape --export-background=WHITE --export-type=png --export-filename=$@ $<
	file $@

icons/%.mono:   icons/%.png
	convert $< -monochrome $@

icons/%.h:      icons/%.mono
	echo "const unsigned char icon_$(patsubst icons/%.h,%,$@)[]={" > $@
	od -Anone -tx1 -v -w64 $< | sed 's/ \(..\)/0x\1,/g' >> $@
	echo "};" >> $@
	echo "const unsigned char icon_$(patsubst icons/%.h,%,$@)_size=sizeof(icon_$(patsubst icons/%.h,%,$@));" >> $@

set:	watchy

watchy: main/icons.h main/icons.c
	components/ESP32-RevK/setbuildsuffix -S1-V0-SSD1681-D4
	@make

flash:
	idf.py flash

monitor:
	idf.py monitor

clean:
	idf.py clean

menuconfig:
	idf.py menuconfig

pull:
	git pull
	git submodule update --recursive

update:
	git submodule update --init --recursive --remote
	git commit -a -m "Library update"

