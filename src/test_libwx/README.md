简要说明如何在cygwin/mingw平台生成静态链接libwx/wxWidgets的GUI程序,libwx版本 3.0
最后更新: hongjun.liao <docici@126.com> 2017/9/28

本文重点是静态版本libwx,若使用动态方式,使用apt-cyg install mingw64-x86_64-wxWidgets3.0即可
一.安装libwx
(1)$apt-cyg install mingw64-x86_64-wxWidgets3.0
此版本包含静态版本libwx,但测试发现链接时,报"duplicate section wxThreadHelperThread has different size"警告,
且运行时报"segment fault", 目前尚未找到原因
(2)自行编译:
a.可以从cygiwn mirror,如http://mirrors.aliyun.com/cygwin/noarch/release/ \
mingw64-x86_64-wxWidgets3.0/mingw64-x86_64-wxWidgets3.0-3.0.3-1-src.tar.xz下载使用cygport xxx.cygport compile进行编译,
但测试发现创会报(1)中的警告
	
b.从libwx官网下载: https://github.com/wxWidgets/wxWidgets/releases/download/v3.0.3/wxWidgets-3.0.3.zip
配置命令: 解压后, 在根目录
./configure --build=x86_64-w64-mingw32 --enable-static --disable-shared --enable-unicode --enable-utf8 CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ --with-expat --with-png --with-jpeg --enable-monolithic
说明:　--build指定编译的是mingw版本
      CC/CXX指定mingw编译器,而不是默认的gcc/g++
      --enable-monolithic将libwx编译成单个.a库,而不是多个(实际也不是单个,但.a数量明显减少)
      --enable-static --disable-shared生成静态版本 
make && make samples && make install 
编译库及samples/目录,安装; 默认安装在/usr/local/include, 库在/usr/local/lib/ 
二.使用
-D_GNU_SOURCE -D__WXMSW__ -DWX_PRECOMP -DwxDEBUG_LEVEL=0 -D_FILE_OFFSET_BITS=64
-I/usr/local/lib/wx/include/msw-unicode-static-3.0 -I/usr/local/include/wx-3.0/
-L/usr/local/lib/ -mwindows -static  -lwx_mswu-3.0 -lwxscintilla-3.0 -lwxregexu-3.0 -lwx_mswu_gl-3.0 -lpng16 -lz -lrpcrt4 -loleaut32 -lole32 -luuid -lole32 -lwinspool -lwinmm -lshell32 -lcomctl32 -lcomdlg32 -ladvapi32 -lcomdlg32 -lwsock32 -lgdi32

注:
(1)自行下载编译libpng: http://mirrors.aliyun.com/cygwin/noarch/release/mingw64-x86_64-openjpeg/mingw64-x86_64-openjpeg-1.5.2-3-src.tar.xz
