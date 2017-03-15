#!/bin/bash
# desc: build plcdn_logtrans
# @author: hongjun.liao <docici@126.com>
# @date: 2017/03/15

#build plcdn_logtrans, a faster version
build_plcdn_logtrans2()
{
	[ "$1" = "debug" ] && CPP_FLAGS+=" -g3 -O0" || CPP_FLAGS+=" -O3";
	local bin_name="plcdn_logtrans";
	[ -f "$HOME/ws/debug/linux/$bin_name" ] && mv $HOME/ws/debug/linux/$bin_name /tmp/$bin_name.1;
	local ws_dir="/tmp/$bin_name/";
	local id_str="`date +%Y%m%d%H%M%S`";
	local src_file="${ws_dir}src/${id_str}.cpp";
	local bin_file="${ws_dir}bin/$bin_name_${id_str}";
	printf "build_plcdn_logtrans2: CPP_FLAGS='%s'\n" "${CPP_FLAGS}";
	mkdir -p "${ws_dir}src/" "${ws_dir}bin/";
	cd ${home_dir}ws/cpp-test/src/;
	for file in test_plcdn_log_transform2.cpp \
		string_uitl.cpp chksum.cpp net_util.cpp \
		nginx_log_analysis/nginx_log_analysis.cpp ; do
		printf "//begin file %s__________________________________________________________________________________//\n" ${file} >> "${src_file}";
		cat $file >> "${src_file}";
		printf "//end file %s__________________________________________________________________________________//\n" ${file} >> "${src_file}";
	done;
	echo "int main(int argc, char ** argv) { return test_nginx_transform_log_main_2(argc, argv); }" >> "${src_file}";

	g++ -static -o "${bin_file}" -std=c++1y  ${CPP_FLAGS} -Wall -fmessage-length=0 -I${home_dir}ws/include/ -I${home_dir}ws/cpp-test/inc/ \
"${src_file}" -lcrypto;
	if [ -x "${bin_file}" ]; then
		strip -s "${bin_file}";
		cp -f "${bin_file}" $HOME/ws/debug/linux/$bin_name;
	else 
		echo "build_plcdn_logtrans2: build failed";
		return 1;
	fi;
	return 0;
}
