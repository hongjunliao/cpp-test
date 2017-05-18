#include "string_util.h"
#include <stdlib.h> /* atoi */

int test_string_util_main(int argc, char ** argv)
{
	fprintf(stdout, "%s: using myatoi:\n'1'=%d, '1d'=%d, 'd1'=%d\n"
			"'22'=%d, '22d'=%d, 'd22'=%d\n"
			"'123'=%d, '123d'=%d, 'd123'=%d\n"
			"'1d23'=%d\n"
			"'-1'=%d, '-1d'=%d, '-22'=%d, '-22d'=%d\n"
			"'+1'=%d, '+1d'=%d, '+22'=%d, '+22d'=%d\n"
			,
			__FUNCTION__
			, myatoi("1", 1), myatoi("1d", 1), myatoi("d1", 1)
			, myatoi("22", 2), myatoi("22d", 3), myatoi("d22", 3)
			, myatoi("123", 3), myatoi("123d", 4), myatoi("d123", 4)
			, myatoi("1d23", 4)
			, myatoi("-1", 2), myatoi("-1d", 3), myatoi("-22", 3), myatoi("-22d", 4)
			, myatoi("+1", 2), myatoi("+1d", 3), myatoi("+22", 3), myatoi("+22d", 4)
			);

	fprintf(stdout, "%s: using atoi:\n'1'=%d, '1d'=%d, 'd1'=%d\n"
			"'22'=%d, '22d'=%d, 'd22'=%d\n"
			"'123'=%d, '123d'=%d, 'd123'=%d\n"
			"'1d23'=%d\n"
			"'-1'=%d, '-1d'=%d, '-22'=%d, '-22d'=%d\n"
			"'+1'=%d, '+1d'=%d, '+22'=%d, '+22d'=%d\n",
			__FUNCTION__
			, atoi("1"), atoi("1d"), atoi("d1")
			, atoi("22"), atoi("22d"), atoi("d22")
			, atoi("123"), atoi("123d"), atoi("d123")
			, atoi("1d23")
			, atoi("-1"), atoi("-1d"), atoi("-22"), atoi("-22d")
			, atoi("+1"), atoi("+1d"), atoi("+22"), atoi("+22d")
			);

	return 0;
}
