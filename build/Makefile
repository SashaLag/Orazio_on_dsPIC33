.phony:	clean all

all:
	make -C host
	make -C atmega2560
	#make -C atmega328
	#make -C dspic

clean:
	rm -rf `find . -iname *~`
	make -C host clean
	make -C atmega2560 clean
	#make -C atmega328 clean
	#make -C dspic clean
