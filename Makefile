obj-m += multiplier.o

all:
	make -C /home/ugrads/m/michaelmengistu/ecen449/linux-3.14 M=$(PWD) modules

clean:
	make -C /home/ugrads/m/michaelmengistu/ecen449/linux-3.14 M=$(PWD) clean
