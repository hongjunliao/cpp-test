/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2015, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
/* <DESC>
 * HTTP PUT with easy interface and read callback
 * </DESC>
 */
#ifdef __CYGWIN_GCC__
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include "bd_test.h"

/*
 * This example shows a HTTP PUT operation. PUTs a file given as a command
 * line argument to the URL also given on the command line.
 *
 * This example also uses its own read callback.
 *
 * Here's an article on how to setup a PUT handler for Apache:
 * http://www.apacheweek.com/features/put
 */

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t retcode;
  curl_off_t nread;

  /* in real-world cases, this would probably get this data differently
     as this fread() stuff is exactly what the library already would do
     by default internally */
  retcode = fread(ptr, size, nmemb, stream);

  nread = (curl_off_t)retcode;

  fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
          " bytes from file\n", nread);

  return retcode;
}


int test_curl_http_put_main(int argc, char **argv)
{
  CURL *curl;
  CURLcode res;
  FILE * hd_src ;
  struct stat file_info;

  char *file;
  char *url;

  if(argc < 3){
	  printf("%s %s %s\n", argv[0], "file", "url");
	  return 1;
  }

  file= argv[1];
  url = argv[2];

  /* get the file size of the local file */
  stat(file, &file_info);

  /* get a FILE * of the same file, could also be made with
     fdopen() from the previous descriptor, but hey this is just
     an example! */
  hd_src = fopen(file, "rb");

  /* In windows, this will init the winsock stuff */
  curl_global_init(CURL_GLOBAL_ALL);

  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {
    /* we want to use our own read function */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

    /* enable uploading */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    /* HTTP PUT please */
    curl_easy_setopt(curl, CURLOPT_PUT, 1L);

    /* specify target URL, and note that this URL should include a file
       name, not only a directory */
    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* now specify which file to upload */
    curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);

    /* provide the size of the upload, we specicially typecast the value
       to curl_off_t since we must be sure to use the correct data size */
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)file_info.st_size);

    /* Now run off and do what you've been told! */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  fclose(hd_src); /* close the local file */

  curl_global_cleanup();
  return 0;
}

int test_http_post_main(void)
{
  CURL *curl;
  CURLcode res;

  /* In windows, this will init the winsock stuff */
  curl_global_init(CURL_GLOBAL_ALL);

  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {
    /* First set the URL that is about to receive our POST. This URL can
       just as well be a https:// URL if that is what should receive the
       data. */
    curl_easy_setopt(curl, CURLOPT_URL, "http://postit.example.com/moo.cgi");
    /* Now specify the POST data */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=daniel&project=curl");

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return 0;
}
//int curl_post_res(const char*postdata,const char *url,cb_func func,void *data)
//{
//}
int test_http_post_json_main(){
//	{
//	    CURLcode res = 0;
//	    char tmp[32]={0};
//	    CURL *curl=NULL;
//	    struct curl_slist *headers = NULL;
//	    if(!data)
//	        return -1 ;
//	    if( !url)
//	        return -1;
//
//	    snprintf(tmp,sizeof(tmp),"Content-Length: %d",strlen(postdata));
//	    curl_slist_append(headers, "Accept: application/json");
//	    curl_slist_append(headers, "Content-Type: application/json");
//	    curl_slist_append(headers, "charset: utf-8");
//	    curl_slist_append(headers, tmp);
//	    curl = curl_easy_init();//瀵筩url杩涜鍒濆鍖�
//	    if(curl){
//	        curl_easy_setopt(curl, CURLOPT_URL, url); //璁剧疆涓嬭浇鍦板潃
//	        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//	        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 8);//璁剧疆瓒呮椂鏃堕棿
//	        curl_easy_setopt(curl, CURLOPT_POST, 1L);//璁剧疆瓒呮椂鏃堕棿
//	        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);//璁剧疆瓒呮椂鏃堕棿
////	        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, func);//璁剧疆鍐欐暟鎹殑鍑芥暟
//	        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);//璁剧疆鍐欐暟鎹殑鍙橀噺
//	        res = curl_easy_perform(curl);//鎵ц涓嬭浇
//	        curl_easy_cleanup(curl);
//	        curl_slist_free_all(headers);
//	        return res;
//	    }else{
//	        curl_slist_free_all(headers);
//	        return -2;
//	    }
//	}
//	}
}
int test_curl_main(int argc, char * argv[])
{
	test_http_post_main();
	test_curl_http_put_main(argc, argv);
}
#endif /*__CYGWIN_GCC__*/
