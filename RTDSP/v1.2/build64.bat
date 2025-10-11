"C:\MinGW64\bin\g++.exe" globldef.c -c -std=c++11 -m64 -o globldef_64.o
"C:\MinGW64\bin\g++.exe" cstrdef.c -c -std=c++11 -m64 -o cstrdef_64.o
"C:\MinGW64\bin\g++.exe" thread.c -c -std=c++11 -m64 -o thread_64.o
"C:\MinGW64\bin\g++.exe" strdef.cpp -c -std=c++11 -m64 -o strdef_64.o
"C:\MinGW64\bin\g++.exe" main.cpp -c -std=c++11 -m64 -o main_64.o
"C:\MinGW64\bin\g++.exe" AudioRTDSP.cpp -c -std=c++11 -m64 -o AudioRTDSP_64.o
"C:\MinGW64\bin\g++.exe" AudioRTDSP_i16.cpp -c -std=c++11 -m64 -o AudioRTDSP_i16_64.o
"C:\MinGW64\bin\g++.exe" AudioRTDSP_i24.cpp -c -std=c++11 -m64 -o AudioRTDSP_i24_64.o

"C:\MinGW64\bin\g++.exe" main_64.o globldef_64.o cstrdef_64.o thread_64.o strdef_64.o AudioRTDSP_64.o AudioRTDSP_i16_64.o AudioRTDSP_i24_64.o -lole32 -lksuser -mwindows -m64 -o rtdsp64.exe

del globldef_64.o
del cstrdef_64.o
del thread_64.o
del strdef_64.o
del main_64.o
del AudioRTDSP_64.o
del AudioRTDSP_i16_64.o
del AudioRTDSP_i24_64.o

