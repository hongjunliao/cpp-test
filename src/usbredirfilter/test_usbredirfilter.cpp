#ifdef CPP_TEST_WITH_USBREDIRFILTER

/*
 * test_usbredirfilter.cpp
 *
 *  Created on: Jan 21, 2016
 *      Author: liaohj
 */
#ifdef TEST_USB
#include <curl/curl.h>
#include <json/json.h>
#include <usb-device-manager.h>
#include <spice-channel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bd_test.h"

struct curl_write_data_fn_arg{
	SpiceUsbDeviceManager *self;
	SpiceChannel *channel;
};

static curl_write_data_fn_arg g_curl_write_data_fn_arg = {0};
typedef size_t (* curl_write_data_fn)(void *ptr, size_t size, size_t nmemb, void *stream);


static size_t curl_write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
	size_t written = 0;
	printf("%s: ptr = 0x%x, size = %u, nmemb = %u, stream = 0x%x\n", __FUNCTION__, ptr, size, nmemb, stream);
	written = fwrite(ptr, size, nmemb, (FILE *)stream);


	char const * json = "{\"usbredir_filter\": \"0x03,-1,-1,-1,0|-1,0x0781,0x5567,-1,1\"}";
	struct json_object * jsonobj = json_tokener_parse(json);
	struct json_object * jsonfilter = NULL;
	json_bool r = json_object_object_get_ex(jsonobj, "usbredir_filter", &jsonfilter);
	if (!r) {
		printf("%s: filter NOT found!\n", __FUNCTION__);

	}
	char const * afilter = json_object_to_json_string(jsonfilter);

	size_t n = strlen(afilter);
	char * filter = (char *) malloc(n);
	memcpy(filter, afilter + 1, n - 2);

	printf("%s: filter = %s\n", __FUNCTION__, filter);

	json_object_put(jsonobj);

	printf("curl_write_data_fn_arg: self = 0x%x, self = 0x%x\n",
			g_curl_write_data_fn_arg.self, g_curl_write_data_fn_arg.channel);
	return written;
}


/*!get usbredirfilter string from URL @param url, @reutrn 0 on ok*/
int bd_cdn_usbredir_update_filter(char const * szurl, SpiceUsbDeviceManager *self, SpiceChannel *channel)
{
	g_curl_write_data_fn_arg.self = self;
	g_curl_write_data_fn_arg.channel = channel;

	CURL *curl = NULL;
	int ret = 1;
	curl = curl_easy_init();
	if (curl) {
		/* set URL to get */
		curl_easy_setopt(curl, CURLOPT_URL, szurl);

		/* no progress meter please */
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

		/* send all data to this function  */
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data);

		/* get it! */
		curl_easy_perform(curl);

		/* always cleanup */
		curl_easy_cleanup(curl);

		ret = 0;
	}
	curl_global_cleanup();

	return ret;
}

int test_usbredirfilter_main(int argc, char ** argv) {
	char const * szurl = (argc > 1? argv[1] : "http://www.example.com");
	bd_cdn_usbredir_update_filter(szurl, NULL, NULL);

	return 0;
}
#endif /*TEST_USB*/

#endif

