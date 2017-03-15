#!/bin/bash
# desc: build plcdn_la
# @author: hongjun.liao <docici@126.com>
# @date: 2017/03/15
build_plcdn_la()
{
	[ "$1" = "debug" ] && CPP_FLAGS+=" -g3 -O0" || CPP_FLAGS+=" -O3";

	[ -f "$HOME/ws/debug/linux/plcdn_la" ] && mv $HOME/ws/debug/linux/plcdn_la /tmp/plcdn_la.1;
	local ws_dir="/tmp/plcdn_la/";
	local id_str="`date +%Y%m%d%H%M%S`";
	local src_file="${ws_dir}src/${id_str}.cpp";
	local bin_file="${ws_dir}bin/plcdn_la_${id_str}";
	printf "build_plcdn_la: CPP_FLAGS='%s'\n" "${CPP_FLAGS}";
	mkdir -p "${ws_dir}src/" "${ws_dir}bin/";
	cd ${HOME}/ws/cpp-test/src/;
	for file in plcdn_log_analysis/*.cpp \
		nginx_log_analysis/nginx_log_analysis.cpp \
		nginx_log_analysis/parse_fmt.cpp \
		nginx_log_analysis/nginx_rotate.cpp \
		srs_log_analysis/srs_log_analysis.cpp \
		srs_log_analysis/split_log.cpp \
		srs_log_analysis/print_table.cpp \
		test_plcdn_log_transform2.cpp \
		string_uitl.cpp chksum.cpp net_util.cpp termio_util.cpp; do
		printf "//begin file %s__________________________________________________________________________________//\n" ${file} >> "${src_file}";
		cat $file >> "${src_file}";
		printf "//end file %s__________________________________________________________________________________//\n" ${file} >> "${src_file}";
	done;
	echo "int main(int argc, char ** argv) { return test_plcdn_log_analysis_main(argc, argv); }" >> "${src_file}";
	g++ -static -o "${bin_file}" \
-DENABLE_IPMAP \
-std=c++0x ${CPP_FLAGS} -Wall -fmessage-length=0 \
-I${HOME}/ws/include/ \
-I${HOME}/ws/cpp-test/inc/ \
"${src_file}" \
-pthread -lboost_filesystem -lboost_system -lboost_regex -lpthread -lcrypto  \
-L"${HOME}/ws/lib/" -lipmap -lpopt \
1>/dev/null;
	if [ -x "${bin_file}" ]; then
		[ "$1" = "debug" ] || strip -s "${bin_file}";
		cp -f "${bin_file}" $HOME/ws/debug/linux/plcdn_la;
	else 
		echo "build_plcdn_la: build failed";
		return 1;
	fi;
	return 0;
}

