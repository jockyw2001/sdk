arm-linux-gnueabihf-gcc  -g -lpthread SimMiTest.c     -static -o SimMiTest.o      -c
arm-linux-gnueabihf-gcc  -g -lpthread SimMiTestDisp.c -static -o SimMiTestDisp.o  -c
arm-linux-gnueabihf-gcc  -lpthread -o DispTest SimMiTest.o SimMiTestDisp.o
