/*!
 * This file is PART of plcdn_log_analysis
 * @author hongjun.liao <docici@126.com>
 * @date 2017/03/30
 *
 * ini config file parse, syntax sample:
 *
 * #this is a commnet
 * [section1]
 * key=value
 * [section2]
 * key2 = value2
 *
 * @date 2017/04/20, use boost.property_tree instead
 */
#include <stdio.h>
#include <ctype.h>	         /* isblank */
#include <string.h>	         /* strrchr */
#include "plcdn_la_conf.h"   /* plcdn_la_conf */

struct strl_t{
	char const * data;
	size_t len;
};

/* FIXME: this function NOT complete yet!!! currently for test only!!!
 * */
static int parse_ini(char const * buf, char const * end, strl_t * comment, strl_t * section, strl_t * key, strl_t * value)
{
	auto s = buf;
	for(auto p = buf; p != end; ++p){
		if(isblank(*p))
			continue;

		s = p;	/* first non-blank */

		if(*p == '#'){	/* maybe comments */
			comment->data = p + 1;
			comment->len = end - comment->data;
			return 2; /* begin with '#', commnets */
		}

		if(*p == '['){ /* begin with '[', section */
			auto c = strrchr(p + 1, ']');
			if(c){
				for(auto q = p + 1; q != c; ++q){
					if(*q == '[' || *q == ']')
						return -1;	/* syntax error: '[' or  ']' between [ and ] not allowed */
				}

				for(auto q = c + 1; q != end; ++q){
					if(!isblank(*q))
						return -1; /* syntax error: characters after ] not allowed */
				}

				section->data = p + 1;
				section->len = c - section->data;
				return 3;	/* [x], section */
			}
		}

		if(*p == '='){	/* maybe key-value */
			/* blanks in key not allowed */

			key->data = s;
			key->len = p - key->data;

			value->data = p + 1;
			value->len = end - key->data;
			return 4;
		}
	}
	return -1;
}

/* needed for ignore exceptions throwed from boost::property_tree::read_ini
 * gcc define: -fno-exceptions */
//namespace boost {
//    void throw_exception(std::exception const& e) {
//        std::cerr << "Fake exception: " << e.what() << "\n";
////        std::exit(255);
//    }
//}

int plcdn_la_parse_config_file(char const *f, plcdn_la_conf_t & conf)
{
	if(access(f, F_OK) != 0)
		return -1;
//	fprintf(stdout, "%s: __________'%s'________\n", __FUNCTION__, f);
	namespace pt = boost::property_tree;
	try{
		pt::read_ini(f, conf.impl);
	}
	catch(...){}
	return 0;
}

std::string plcdn_la_config_get_string(plcdn_la_conf_t const& conf, char const * key)
{
	try{
		return conf.impl.get<std::string>(key);
	}
	catch(...){
		return "";
	}
}
