#! /bin/bash
# This file is PART of cpp-test project
# @author hongjun.liao <docici@126.com>, @date 2018/7/11
# 
# test paxos protocol
# https://github.com/Tencent/phxpaxos

arg_n='';
arg_a='';
args='';
for i in {1..99}; do
#	echo $((i+1));
	arg_n+=",127.0.0.1:"$((i+11100));
done;
	arg_n=${arg_n:1};
#	echo $arg_n;

for i in {1..99}; do
	arg_a="127.0.0.1:"$((i+11100));
	args='paxos1 -i'$((i+11100))' -a'${arg_a}' -n'${arg_n}' -v6 -f /home/jun/sdb1/cpp-test/src/paxos/.echo_file';
	echo "$((i+11100)) $((i+11100))" >> .echo_file
#	echo $args;
	cpp-test-main $args > /tmp/paxos_$i&
done;
#	cpp-test-main paxos1 -i$((i+1)) -a 127.0.0.1:11111 -n 127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113 -v6
