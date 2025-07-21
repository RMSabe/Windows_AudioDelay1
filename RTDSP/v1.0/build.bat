"C:\MinGW64\bin\g++.exe" globldef.c -c -std=c++11 -o globldef.o
"C:\MinGW64\bin\g++.exe" cstrdef.c -c -std=c++11 -o cstrdef.o
"C:\MinGW64\bin\g++.exe" thread.c -c -std=c++11 -o thread.o
"C:\MinGW64\bin\g++.exe" strdef.cpp -c -std=c++11 -o strdef.o
"C:\MinGW64\bin\g++.exe" main.cpp -c -std=c++11 -o main.o
"C:\MinGW64\bin\g++.exe" AudioRTDSP.cpp -c -std=c++11 -o AudioRTDSP.o
"C:\MinGW64\bin\g++.exe" AudioRTDSP_i16_1ch.cpp -c -std=c++11 -o AudioRTDSP_i16_1ch.o
"C:\MinGW64\bin\g++.exe" AudioRTDSP_i16_2ch.cpp -c -std=c++11 -o AudioRTDSP_i16_2ch.o
"C:\MinGW64\bin\g++.exe" AudioRTDSP_i24_1ch.cpp -c -std=c++11 -o AudioRTDSP_i24_1ch.o
"C:\MinGW64\bin\g++.exe" AudioRTDSP_i24_2ch.cpp -c -std=c++11 -o AudioRTDSP_i24_2ch.o

"C:\MinGW64\bin\g++.exe" main.o globldef.o cstrdef.o thread.o strdef.o AudioRTDSP.o AudioRTDSP_i16_1ch.o AudioRTDSP_i16_2ch.o AudioRTDSP_i24_1ch.o AudioRTDSP_i24_2ch.o -lole32 -mwindows -o rtdsp.exe

del globldef.o
del cstrdef.o
del thread.o
del strdef.o
del main.o
del AudioRTDSP.o
del AudioRTDSP_i16_1ch.o
del AudioRTDSP_i16_2ch.o
del AudioRTDSP_i24_1ch.o
del AudioRTDSP_i24_2ch.o

