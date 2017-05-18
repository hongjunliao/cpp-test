#include "string_util.h"
#include <boost/regex.hpp> 		/*  boost::regex_search */

int parse_domain_from_url(const char* url, char* domain)
{
	/*sample: 'rtmp://127.0.0.1:1359/'*/
	boost::cmatch cm;
	auto f = boost::regex_search(url, cm, boost::regex("://([^/:]+)(?::[0-9]+)?/"));
	if(!f) return -1;

	auto length = cm.length(1);
	strncpy(domain, cm[1].first, length);
	domain[length] = '\0';
	return 0;
}

int parse_domain_from_url(char const * url, str_t * domain)
{
//	fprintf(stderr, "%s: url='%s'\n", __FUNCTION__, url);
	auto c = strchr(url, ':');
	if(c && *(c + 1) == '/' && *(c + 2) == '/'){
		domain->beg = const_cast<char *>(c + 3);
		domain->end = strchr(domain->beg, '/');

//		str_t_fprint(domain, stdout);
		return 0;
	}
	return -1;
}

int str_t_fprint(str_t const * s, FILE * f)
{
	int r = 0;
	for(auto c = s->beg; c != s->end; ++c){
		if(*c == '\0')
			r = fprintf(f, "\\0");
		else if(*c == '\n')
			r = fprintf(f, "\\n");
		else
			r = fprintf(f, "%c", *c);
	}
	if(s->beg)
		fprintf(f, "\n");
	return r;
}

#if (defined __GNUC__) && !(defined __CYGWIN__)

char *strlwr(char *s)
{
	for (char *str = s; *str; ++str) {
		if (*str >= 'A' && *str <= 'Z') {
			*str += ('a' - 'A');
		}
	}
	return s;
}

char *strupr(char *s)
{
	for (char *str = s; *str; ++str) {
		if (*str >= 'a' && *str <= 'z') {
			*str -= ('a' - 'A');
		}
	}
	return s;
}

#endif	/*(defined __GNUC__) && !(defined __CYGWIN__)*/

/*@param unit 'K': KB, 'M':MB, 'G':GB, ' ':B*/
static double byte_to_mb_kb(size_t bytes, char & unit)
{
	if(bytes >= 1024 * 1024 && bytes < 1024 * 1024 * 1024){
		unit = 'M';
		return bytes / (1024.0 * 1024);
	}
	else if(bytes >= 1024 * 1024 * 1024){
		unit = 'G';
		return bytes / (1024.0 * 1024 * 1024);
	}
	else if(bytes >= 1024){
		unit = 'K';
		return bytes / 1024.0;
	}
	else{
		unit = ' ';
		return bytes / 1.0;
	}
}

char * byte_to_mb_kb_str_r(size_t bytes, char const * fmt, char * buff)
{
	if(!fmt) return buff;
	char c;
	double b = byte_to_mb_kb(bytes, c);
	snprintf(buff, 64, fmt, b, c);
	return buff;
}
/*@param fmt: "%-.2f %cB*/
char const * byte_to_mb_kb_str(size_t bytes, char const * fmt)
{
	static char buff[64] = "";
	return byte_to_mb_kb_str_r(bytes, fmt, buff);
}


char const * strnrchr(char const * buf, int sz, char ch)
{
	for(auto const * p = buf + sz - 1; p != buf - 1; --p) {
		if(*p == ch)
			return p;
	}
	return 0;
}

void str_dump(FILE * f, char const * buf, size_t len)
{
	for(size_t i = 0; i < len; ++i){
		if(buf[i] == '\r')
			fprintf(f, "\\r");
		if(buf[i] == '\n')
			fprintf(f, "\\n");
		else if(buf[i] == '\0')
			fprintf(f, "\\0");
		else
			fprintf(f, "%c", buf[i]);
	}
}

/* FIXME: overflow */
int myatoi(char const * str, size_t len)
{
	if(!(str && str[0] != '\0' && len > 0))
		return 0;

	bool f = false;
	if(str[0] == '+' || str[0] == '-'){
		if(str[0] == '-')
			f = true;
		++str;
		--len;
	}

	int ints[64];
	int i = 0;
	for(auto p = str; p != str + len; ++p){
		if(!(*p >= '0' && *p <= '9'))
			break;
		ints[i++] = *p - '0';
	}

	int r = 0;
	for(int j = 0; j < i; ++j){
		r += ints[j] * pow(10, i - j - 1);
	}
	return f? -r : r;
}
