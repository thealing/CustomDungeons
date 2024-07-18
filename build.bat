@ windres -i game.rc --input-format=rc -o game.res -O coff

@ g++ -c -O2 -s game.cpp -o game.o 

@ g++ -mwindows game.res game.o -o game.exe -static-libgcc -l gdi32 -l comdlg32 -l winmm

@ del *.o *.res
