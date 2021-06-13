/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2018/8/21
 *
 * from github: https://github.com/SeiyaKobayashi/zlib_sample/blob/master/zlib_sample.c
 * */
/*
 * Author: Seiya Kobayashi
 * Date: Feb.26, 2018 (Updated on Mar.19, 2018)
 * Description: Simple implementation of zlib deflation/inflation in c.
 *              Wrote mainly for solidifying my understanding of zlib,
 *              the most widely-used library for data compression.
 *              This would be an important milestone to be able to write
 *              git implementation, which uses zlib to compress/uncompress
 *              its objects (such as blobs).
 * Reference: https://www.zlib.net/manual.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

// Compress "src" into "dest" ("a" into "b" in this example)
static void _compress(char *src, uInt srcSize, char *dest, uInt destSize)
{
    z_stream zd;
    /* Set zalloc and zfree to Z_NULL, so that deflateInit upadates them
     to use default allocation functions.
     */
    zd.zalloc = Z_NULL;
    zd.zfree = Z_NULL;
    zd.opaque = Z_NULL;

    // Set "src" as an input and "dest" as an output
    zd.next_in = (Bytef *)src;
    zd.avail_in  = srcSize+1;      // String + Terminator
    zd.next_out = (Bytef *)dest;
    zd.avail_out = destSize;      // Size of the output

    if (deflateInit(&zd, Z_BEST_COMPRESSION) != Z_OK)
        perror("deflateInit failed");
    deflate(&zd, Z_FINISH);
    if (deflateEnd(&zd) != Z_OK)
        perror("deflateEnd failed");

    // Prints the compressed string (and its length) stored in "dest"
    printf("Compressed string:\n\t%s\n\twith its length of %lu\n", dest, zd.total_out);
}

// Uncompress "src" to "dest" ("b" into "c" in this example)
static void _uncompress(char *src, uInt srcSize, char *dest, uInt destSize)
{
    z_stream zi;
    /* Set zalloc and zfree to Z_NULL, so that inflateInit upadates them
     to use default allocation functions.
     */
    zi.zalloc = Z_NULL;
    zi.zfree = Z_NULL;
    zi.opaque = Z_NULL;

    // Set "src" as an input and "dest" as an output
    zi.next_in = (Bytef *)src;
    zi.avail_in = (uInt)((char *)zi.next_out - dest);      // Size of the input
    zi.next_out = (Bytef *)dest;
    zi.avail_out = destSize;      // Size of the output

    if (inflateInit(&zi) != Z_OK)
        perror("inflateInit failed");
    inflate(&zi, Z_NO_FLUSH);
    if (inflateEnd(&zi) != Z_OK)
        perror("inflateEnd failed");

    // Prints the uncompressed string (and its length) stored in "dest"
    printf("Uncompressed string:\n\t%s\n\twith its length of %lu\n", dest, strlen(dest));
}

int test_zlib2_main(int argc, char* argv[])
{
    // Initial string length = 34
    char a[] = "Hello, my name is Seiya Kobayashi.aerowqg9ruhfy9ahaiosdfhaosdifhaoisdfhaoisdfha"
    		"asdfasdfasdfffffffffffasdfasdfasdffasdfasdfasdfasdfffffffffffffffffffffffffffffafsfaf"
    		"asfddddddddddddddddddddddddddddddddddddaafsosuidfhaosdfhasoidfhyasorhwaeouirfhweior";
    // Placeholder for the compressed "a"
    char b[1024];
    // Placeholder for the uncompressed "b"
    char c[1024];
    // Prints the initial string (and its length) stored in "a"
    printf("Initial string:\n\t%s\n\twith its length of %lu\n", a, strlen(a));

    // Compression(deflation)
    _compress(a, sizeof(a), b, sizeof(b));
    // Uncompression(inflation)
    _uncompress(b, sizeof(b), c, sizeof(c));

    return 0;
}
