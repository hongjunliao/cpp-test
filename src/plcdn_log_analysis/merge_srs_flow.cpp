/*!
 * This file is PART of plcdn_log_analysis
 * parse logs and print results, currently support: nginx, srs logfiles
 * @author hongjun.liao <docici@126.com>
 * @date 2017/1

 * for merge_srs_flow
 */
#include <stdio.h>			/* fgets */
#include <string.h> 		/* strlen */
#include <vector>			/* std::vector */
#include <unordered_map> 	/* std::unordered_map */
#include <tuple>			/* std::tuple */
#include <boost/functional/hash.hpp>	/* boost::hash_combine */

//template<typename T> struct group_field
//{
//	T val;
//	char name[64];
//	bool is_group;
//};
/////////////////////////////////////////////////////////////////////////////////////////////////////
/* @see  fprint_srs_log_stats */
struct srs_flow_table_row
{
	int device_id;
	int site_id;
	int user_id;
	time_t datetime;
	size_t obytes, ibytes;
	double ombps, imbps;
};

/* parse srs_flow_table_row from @param buf, return 0 on success
 * @see fprint_srs_log_stats*/
static int parse_srs_flow_table_row(char const * buf, srs_flow_table_row & row);

typedef std::tuple<time_t, int> srs_flow_key_t; /* std::tuple<datetime, user_id> */
typedef std::tuple<size_t, size_t, double, double> srs_flow_value_t;	/* std::tuple<obytes, ibytes, ombps, imbps> */
/* required by std::unordered_map's key, @see http://en.cppreference.com/w/cpp/utility/hash */
namespace std{
template<> struct hash<srs_flow_key_t>
{
	typedef srs_flow_key_t argument_type;
	typedef std::size_t result_type;
	result_type operator()(argument_type const& s) const;
};

}	//namespace std

std::size_t std::hash<srs_flow_key_t>::operator()(
		srs_flow_key_t const& val) const
{
	size_t ret = 0;
	size_t const h0 ( std::hash<time_t>{}(std::get<0>(val)) );
	size_t const h1 ( std::hash<int>{}(std::get<1>(val)) );
	boost::hash_combine(ret, h0);
	boost::hash_combine(ret, h1);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
int merge_srs_flow(int argc, char ** argv)
{
	std::vector<srs_flow_table_row> flows;
    char buf[512];
    while (fgets(buf, sizeof buf, stdin)){
    	srs_flow_table_row row;
    	if(parse_srs_flow_table_row(buf, row) != 0)
    		continue;
    	flows.push_back(row);
    }

    std::unordered_map<srs_flow_key_t, srs_flow_value_t> flow_map;
    for(auto const & item : flows){
    	auto  k = std::make_tuple(item.datetime, item.user_id);
    	auto & val = flow_map[k];
    	auto & obytes = std::get<0>(val);
    	auto & ibytes = std::get<1>(val);
    	auto & ombps = std::get<2>(val);
    	auto & imbps = std::get<3>(val);

    	obytes += item.obytes;
    	ibytes += item.ibytes;
    	ombps += item.ombps;
    	imbps += item.imbps;
    }
    size_t failed_line = 0;
    for(auto const & item : flow_map){
    	auto t = std::get<0>(item.first);
    	auto user_id = std::get<1>(item.first);

    	auto obytes = std::get<0>(item.second);
    	auto ibytes = std::get<1>(item.second);
    	auto ombps = std::get<2>(item.second);
    	auto imbps = std::get<3>(item.second);

    	char buft[32] = "";
    	strftime(buft, sizeof(buft), "%Y%m%d%H%M", localtime(&t));
		/* format: 'datetime obytes ibytes ombps imbps user_id' @see fprint_srs_log_stats */
		auto sz = fprintf(stdout, "%s %zu %zu %.2f %.2f %d\n", buft,
				obytes, ibytes, ombps, imbps, user_id);
		if(sz <= 0)
			++failed_line;
    }
	return 0;
}

static int parse_srs_flow_table_row(char const * buf, srs_flow_table_row & row)
{
	if(!buf || buf[0] == '\0')
		return -1;

	char datetime[12 + 1]; /* '201611220920' */
	/* @see fprint_srs_log_stats */
	int n = sscanf(buf, "%d%12s%d%zu%zu%lf%lf%d",
	                     &row.site_id, datetime, &row.device_id,
						 &row.obytes, &row.ibytes, &row.ombps, &row.imbps, &row.user_id);
	if(n != 8)
		return -1;

	tm my_tm;
	char const * result = strptime(datetime, "%Y%m%d%H%M" , &my_tm);
	if(!result)
		return -1;
	my_tm.tm_isdst = 0;
	row.datetime = mktime(&my_tm);

	return 0;
}
