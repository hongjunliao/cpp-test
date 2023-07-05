/*!
 * a test for LLVM, clang
 * doc:
 * http://geek.csdn.net/news/detail/37785
 * http://www.open-open.com/lib/view/open1353806821527.html
 *
 * install LLVM:
 * apt install llvm clang
 *
 * compile by clang:
 * clang hello_llvm.c -o hello_llvm-clang -v
 *
 * run by LLVM:
 * lli hello_llvm.bc
 *
 * to human-readable
 * llvm-dis hello_llvm.bc
 *
 * to assemble
 * llc hello_llvm.bc -o hello_llvm.S
 *
 * assemble to binary/native
 * gcc hello_llvm.S -o hello_llvm.native
 * */
#include <stdio.h>
int test_llvm_main(int argc, char ** argv)
{
	printf("%s: hello, llvm!\n", __FUNCTION__);
	return 0;
}
