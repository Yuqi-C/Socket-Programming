all: clientA.c clientB.c central.c serverT.c serverS.c serverP.c
	gcc -o clientA clientA.c
	gcc -o clientB clientB.c
	gcc -o serverC central.c
	gcc -o serverT serverT.c
	gcc -o serverS serverS.c
	gcc -o serverP serverP.c -lm #=lm is to compile math.h used for abs

serverC:
	./serverC

serverT:
	./serverT

serverS:
	./serverS

serverP:
	./serverP

.PHONY: serverC serverT serverS serverP
