
all:
	BEAROS=$(BEAROS) make -C bearos_bc
	BEAROS=$(BEAROS) make -C bearos_blink
	BEAROS=$(BEAROS) make -C bearos_bute
	BEAROS=$(BEAROS) make -C bearos_cal
	BEAROS=$(BEAROS) make -C bearos_calc
	BEAROS=$(BEAROS) make -C bearos_cpm
	BEAROS=$(BEAROS) make -C bearos_frotz
	BEAROS=$(BEAROS) make -C bearos_hexdump
	BEAROS=$(BEAROS) make -C bearos_more
	BEAROS=$(BEAROS) make -C bearos_show_jpeg

clean:
	rm -rf rootfs
	make -C bearos_bc clean
	make -C bearos_blink clean
	make -C bearos_bute clean
	make -C bearos_cal clean
	make -C bearos_calc clean
	make -C bearos_cpm clean
	make -C bearos_frotz clean
	make -C bearos_hexdump clean
	make -C bearos_more clean
	make -C bearos_show_jpeg clean

stage:
	make -C bearos_bc stage
	make -C bearos_blink stage
	make -C bearos_bute stage
	make -C bearos_cal stage
	make -C bearos_calc stage
	make -C bearos_cpm stage
	make -C bearos_frotz stage
	make -C bearos_hexdump stage
	make -C bearos_more stage
	make -C bearos_show_jpeg stage

