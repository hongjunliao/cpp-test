/*
 * test_json.cpp
 *
 *  Created on: Jan 18, 2016
 *      Author: liaohj
 */
#include "bd_test.h"
#include <json/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_jsonc_usbredirfilter()
{
    char const * json = "{\"usbredir_filter\": \"0x03,-1,-1,-1,0|-1,0x0781,0x5567,-1,1\"}";
    struct json_object * jsonobj = json_tokener_parse(json);
    struct json_object * jsonfilter = NULL;
    json_bool r = json_object_object_get_ex(jsonobj, "usbredir_filter", &jsonfilter);
    if(!r){
        printf("%s: filter NOT found!\n", __FUNCTION__);

    }
    char const * afilter = json_object_to_json_string(jsonfilter);

	size_t n = strlen(afilter);
	char  * filter = (char  * )malloc(n);
	memcpy(filter, afilter + 1, n - 2);

	printf("%s: filter = %s\n", __FUNCTION__, filter);


    json_object_put(jsonobj);
}
int test_jsonc_main(int, char **)
{
	test_jsonc_usbredirfilter();

    struct json_object *infor_object = NULL;
    infor_object = json_object_new_object();
    if (NULL == infor_object)
    {
        printf("new json object failed.\n");
        return 1;
    }

    struct json_object *para_object = NULL;
    para_object = json_object_new_object();
    if (NULL == para_object)
    {
        json_object_put(infor_object);//free
        printf("new json object failed.\n");
        return 1;
    }

    struct json_object *array_object = NULL;
    array_object = json_object_new_array();
    if (NULL == array_object)
    {
        json_object_put(infor_object);//free
        json_object_put(para_object);//free
        printf("new json object failed.\n");
        return 1;
    }

    /*添加json值类型到数组中*/
    json_object_array_add(array_object, json_object_new_int(256));
    json_object_array_add(array_object, json_object_new_int(257));
    json_object_array_add(array_object, json_object_new_int(258));

    json_object_object_add(para_object, "DeviceId", json_object_new_string("sn_iso_9000"));
    json_object_object_add(para_object, "MacAddr", json_object_new_string("AA:BB:CC:DD:EE:FF"));
    json_object_object_add(para_object, "Visible", json_object_new_int(1));

    /*添加json名称和值到json对象集合中*/
    json_object_object_add(infor_object, "method", json_object_new_string("GetSystemInfo"));
    json_object_object_add(infor_object, "param", para_object);
    json_object_object_add(infor_object, "id", json_object_new_string("101"));

    /*添加数组集合到json对象中*/
    json_object_object_add(infor_object, "array", array_object);

    printf("-----------json infor ---------------------------\n");
    printf("%s\n", json_object_to_json_string(infor_object));
    printf("-----------json infor ---------------------------\n");

    struct json_object *result_object = NULL;


    result_object =  json_object_object_get(infor_object, "method");
    printf("-----------result_object method ---------------------------\n");
    printf("%s\n", json_object_to_json_string(result_object));
    printf("-----------result_object method---------------------------\n");

    result_object =  json_object_object_get(infor_object, "param");
    printf("-----------result_object param ---------------------------\n");
    printf("%s\n", json_object_to_json_string(result_object));
    printf("-----------result_object param---------------------------\n");

    result_object =  json_object_object_get(infor_object, "array");
    printf("-----------result_object  array---------------------------\n");
    printf("%s\n", json_object_to_json_string(result_object));
    printf("-----------result_object array---------------------------\n");

    int i;
    for(i = 0; i < json_object_array_length(result_object); i++) {
      struct json_object *obj = json_object_array_get_idx(result_object, i);
      printf("\t[%d]=%s\n", i, json_object_to_json_string(obj));
    }

    json_object_put(infor_object);//free


    return 0;
}


