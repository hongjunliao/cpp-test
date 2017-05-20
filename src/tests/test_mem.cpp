#if (defined __GCC__ || defined  __CYGWIN_GCC__)
#include "bd_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h> /*mmap*/
#include <map>
#ifdef _WIN32
#include <memoryapi.h>	/*VirtualAlloc*/
#endif /*_WIN32*/
#include <unistd.h> /*sleep*/

int test_alloc_mmap_main(int argc, char * argv[])
{
//	for(int i = 0; i < 12; ++i){
//		long sz = 1024 * 1024 * 500;
//		char * buff = (char *)malloc(sz);
//		fprintf(stdout, "malloc done, addr=%p, size=500M\n", buff);
//		memset(buff, 'a', sz);
//		sleep(1);
//	}
//	return 0;
//	long sz = 1024 * 1024 * 500;
//	for(int i = 0; i < 5; ++i){
//		char filename[512];
//		sprintf(filename, "500M_0%d", i);
//		FILE * test_file = fopen(filename, "r");
//		if(!test_file){
//			fprintf(stderr, "fopen file %s failed\n", filename);
//			return 1;
//		}
//		char const * ptr = (char const *)mmap(NULL, sz, PROT_READ, MAP_PRIVATE, fileno(test_file), 0);
//		if(ptr == MAP_FAILED){
//			fprintf(stderr, "mmap file %s failed\n", filename);
//			return 1;
//		}
//		char buff[64];
//		memcpy(buff, ptr + sz - 65, 64);
//		hexdump(buff, 64);
//		sleep(1);
//	}

//	srand(time(0)); //use current time as seed for random generator
//	std::map<std::string, char[1024]> mapdata;
//	for(long i = 0; i < 10000 * 4000; ++i){
//		time_t t = time(NULL);
//		char key[512];
//		sprintf(key, "%s_%06d", ctime(&t), rand());
//		memcpy(mapdata[key], key, 1024);
//		if(i % 1000000 == 0){
//			fprintf(stdout, "current=%ld, count=%ld, [%s-%s]\n",
//					i, mapdata.size(), key, mapdata.at(key));
//		}
//	}

//	for(int i = 0; i < 12; ++i){
//		size_t sz = 256 * 1024 * 1024;
//		char * buff = (char *)VirtualAlloc(NULL, sz, MEM_COMMIT, PAGE_READWRITE);
//		memset(buff, 'a', sz);
//		fprintf(stdout, "VirtualAlloc done, addr=%p, size=256M\n", buff);
//		sleep(1);
//	}

	return 0;
}
#endif /* (defined __GCC__ || defined  __CYGWIN_GCC__) */
