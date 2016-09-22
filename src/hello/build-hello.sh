#!/bin/sh
g++ -shared -g -Wall hello.cpp -o hello.dll && g++ -static -g -Wall -o "hello.exe" main.cpp -L . -lhello && echo "ldd ./hello.dll" &&  ldd ./hello.dll && echo "ldd ./hello.exe" && ldd ./hello.exe && ./hello.exe
