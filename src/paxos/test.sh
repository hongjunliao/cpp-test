#! /bin/bash
arg_n='';
arg_a='';
args='';
for i in {1..21}; do
#	echo $((i+1));
	arg_n+=",127.0.0.1:"$((i+11110));
done;
	arg_n=${arg_n:1};
#	echo $arg_n;

for i in {1..21}; do
	arg_a="127.0.0.1:"$((i+11110));
	args='paxos1 -i'$((i+11110))' -a'${arg_a}' -n'${arg_n}' -v6 -f /home/jun/sdb1/cpp-test/src/paxos/.echo_file';
#	echo "$((i+11110)) hello" >> .echo_file
#	echo $args;
	cpp-test-main $args > /tmp/paxos_$i&
done;
#	cpp-test-main paxos1 -i$((i+1)) -a 127.0.0.1:11111 -n 127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113 -v6
