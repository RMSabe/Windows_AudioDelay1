"C:\MinGW64\bin\g++.exe" globldef.c -c -std=c++11 -m32 -o globldef_32.o
"C:\MinGW64\bin\g++.exe" cstrdef.c -c -std=c++11 -m32 -o cstrdef_32.o
"C:\MinGW64\bin\g++.exe" thread.c -c -std=c++11 -m32 -o thread_32.o
"C:\MinGW64\bin\g++.exe" strdef.cpp -c -std=c++11 -m32 -o strdef_32.o
"C:\MinGW64\bin\g++.exe" main.cpp -c -std=c++11 -m32 -o main_32.o
"C:\MinGW64\bin\g++.exe" AudioRTDSP.cpp -c -std=c++11 -m32 -o AudioRTDSP_32.o
"C:\MinGW64\bin\g++.exe" AudioRTDSP_i16_1ch.cpp -c -std=c++11 -m32 -o AudioRTDSP_i16_1ch_32.o
"C:\MinGW64\bin\g++.exe" AudioRTDSP_i16_2ch.cpp -c -std=c++11 -m32 -o AudioRTDSP_i16_2ch_32.o
"C:\MinGW64\bin\g++.exe" AudioRTDSP_i24_1ch.cpp -c -std=c++11 -m32 -o AudioRTDSP_i24_1ch_32.o
"C:\MinGW64\bin\g++.exe" AudioRTDSP_i24_2ch.cpp -c -std=c++11 -m32 -o AudioRTDSP_i24_2ch_32.o

"C:\MinGW64\bin\g++.exe" main_32.o globldef_32.o cstrdef_32.o thread_32.o strdef_32.o AudioRTDSP_32.o AudioRTDSP_i16_1ch_32.o AudioRTDSP_i16_2ch_32.o AudioRTDSP_i24_1ch_32.o AudioRTDSP_i24_2ch_32.o -lole32 -lksuser -mwindows -m32 -o rtdsp32.exe

del globldef_32.o
del cstrdef_32.o
del thread_32.o
del strdef_32.o
del main_32.o
del AudioRTDSP_32.o
del AudioRTDSP_i16_1ch_32.o
del AudioRTDSP_i16_2ch_32.o
del AudioRTDSP_i24_1ch_32.o
del AudioRTDSP_i24_2ch_32.o

