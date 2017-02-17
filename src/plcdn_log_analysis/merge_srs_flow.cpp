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
#include "test_options.h"				/*plcdn_la_options*/
/*plcdn_log_analysis/option.cpp*/
extern struct plcdn_la_options plcdn_la_opt;

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
	double obps, ibps;
};

struct nginx_flow_table_row
{
	 int site_id;
	 time_t datetime;
	 int device_id;
	 size_t num_total;
	 size_t bytes_total;
	 int user_id;
	 size_t pvs_m;
	 size_t px_m;

	 size_t srs_in, srs_out;
};

struct nginx_url_popular_table_row
{
	time_t datetime;
	std::string url_key;
	size_t num_total, num_200, size_200, num_206, size_206, num_301302, num_304,
			 num_403, num_404, num_416, num_499, num_500, num_502, num_other;
};
/* parse srs_flow_table_row from @param buf, return 0 on success
 * @see fprint_srs_log_stats*/
static int parse_table_row(char const * buf, srs_flow_table_row & row);
static int parse_table_row(char const * buf, nginx_flow_table_row & row);
static int parse_table_row(char const * buf, nginx_url_popular_table_row & row);

/////////////////////////////////////////////////////////////////////////////////////////////////////
typedef std::tuple<time_t, int> srs_user_flow_key_t; /* std::tuple<datetime, user_id> */
typedef std::tuple<size_t, size_t, double, double> srs_user_flow_value_t;	/* std::tuple<obytes, ibytes, obps, ibps> */
typedef std::tuple<time_t, int, int, int> merge_key_t; 	/* std::tuple<datetime, site_id, device_id, user_id> */
typedef std::tuple<time_t, std::string> merge_nginx_url_popular_key_t; 	/* std::tuple<datetime, url_key> */

/* std::tuple<num_total, bytes_total, pvs_m, px_m, srs_in, srs_out> */
typedef std::tuple<size_t, size_t, size_t, size_t, size_t, size_t> nginx_flow_value_t;
/* std::tuple<num_total, num_200, size_200, num_206, size_206, num_301302, num_304,
			 num_403, num_404, num_416, num_499, num_500, num_502, num_other> */
typedef std::tuple<size_t, size_t, size_t, size_t, size_t, size_t, size_t,
				size_t, size_t, size_t, size_t, size_t, size_t, size_t> nginx_url_popular_value_t;
/* std::tuple<num_total, bytes_total, pvs_m, px_m, srs_in, srs_out> */
typedef srs_user_flow_value_t srs_flow_value_t;

/* required by std::unordered_map's key, @see http://en.cppreference.com/w/cpp/utility/hash */
namespace std{
template<> struct hash<srs_user_flow_key_t>
{
	typedef srs_user_flow_key_t argument_type;
	typedef std::size_t result_type;
	result_type operator()(argument_type const& s) const;
};

}	//namespace std

std::size_t std::hash<srs_user_flow_key_t>::operator()(
		srs_user_flow_key_t const& val) const
{
	size_t ret = 0;
	size_t const h0 ( std::hash<time_t>{}(std::get<0>(val)) );
	size_t const h1 ( std::hash<int>{}(std::get<1>(val)) );
	boost::hash_combine(ret, h0);
	boost::hash_combine(ret, h1);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
namespace std{
template<> struct hash<merge_key_t>
{
	typedef merge_key_t argument_type;
	typedef std::size_t result_type;
	result_type operator()(argument_type const& s) const;
};

}	//namespace std

std::size_t std::hash<merge_key_t>::operator()(
		merge_key_t const& val) const
{
	size_t ret = 0;
	size_t const h0 ( std::hash<time_t>{}(std::get<0>(val)) );
	size_t const h1 ( std::hash<int>{}(std::get<1>(val)) );
	size_t const h2 ( std::hash<int>{}(std::get<2>(val)) );
	size_t const h3 ( std::hash<int>{}(std::get<3>(val)) );

	boost::hash_combine(ret, h0);
	boost::hash_combine(ret, h1);
	boost::hash_combine(ret, h2);
	boost::hash_combine(ret, h3);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
namespace std{
template<> struct hash<merge_nginx_url_popular_key_t>
{
	typedef merge_nginx_url_popular_key_t argument_type;
	typedef std::size_t result_type;
	result_type operator()(argument_type const& s) const;
};

}	//namespace std

std::size_t std::hash<merge_nginx_url_popular_key_t>::operator()(
		merge_nginx_url_popular_key_t const& val) const
{
	size_t ret = 0;
	size_t const h0 ( std::hash<time_t>{}(std::get<0>(val)) );
	size_t const h1 ( std::hash<string>{}(std::get<1>(val)) );

	boost::hash_combine(ret, h0);
	boost::hash_combine(ret, h1);
	return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
int merge_srs_flow_user(int argc, char ** argv)
{
	std::vector<srs_flow_table_row> flows;
    char buf[512];
    while (fgets(buf, sizeof buf, stdin)){
    	srs_flow_table_row row;
    	if(parse_table_row(buf, row) != 0)
    		continue;
    	flows.push_back(row);
    }

    std::unordered_map<srs_user_flow_key_t, srs_user_flow_value_t> flow_map;
    for(auto const & item : flows){
    	auto  k = std::make_tuple(item.datetime, item.user_id);
    	auto & val = flow_map[k];
    	auto & obytes = std::get<0>(val);
    	auto & ibytes = std::get<1>(val);
    	auto & obps = std::get<2>(val);
    	auto & ibps = std::get<3>(val);

    	obytes += item.obytes;
    	ibytes += item.ibytes;
    	obps += item.obps;
    	ibps += item.ibps;
    }
    size_t failed_line = 0;
    for(auto const & item : flow_map){
    	auto t = std::get<0>(item.first);
    	auto user_id = std::get<1>(item.first);
    	auto obytes = std::get<0>(item.second);
    	auto ibytes = std::get<1>(item.second);
    	auto obps = std::get<2>(item.second);
    	auto ibps = std::get<3>(item.second);

		/* format: 'datetime obytes ibytes obps ibps user_id' @see fprint_srs_log_stats */
		auto sz = fprintf(stdout, "%ld %zu %zu %.0f %.0f %d\n", t,
				obytes, ibytes, obps, ibps, user_id);
		if(sz <= 0)
			++failed_line;
    }
	return 0;
}

static int parse_table_row(char const * buf, srs_flow_table_row & row)
{
	if(!buf || buf[0] == '\0')
		return -1;
	/* @see fprint_srs_log_stats */
	int n = sscanf(buf, "%d%ld%d%zu%zu%lf%lf%d",
	                     &row.site_id, &row.datetime, &row.device_id,
						 &row.obytes, &row.ibytes, &row.obps, &row.ibps, &row.user_id);
	if(n != 8)
		return -1;
	return 0;
}

static int parse_table_row(char const * buf, nginx_flow_table_row & row)
{
	if(!buf || buf[0] == '\0')
		return -1;
	if(!plcdn_la_opt.append_flow_nginx){
		int n = sscanf(buf, "%d%ld%d%zu%zu%d%zu%zu",
							 &row.site_id, &row.datetime, &row.device_id,
							 &row.num_total, &row.bytes_total, &row.user_id, &row.pvs_m, &row.px_m);
		if(n != 8)
			return -1;
	}
	else{
		int n = sscanf(buf, "%d%ld%d%zu%zu%d%zu%zu%zu%zu",
							 &row.site_id, &row.datetime, &row.device_id,
							 &row.num_total, &row.bytes_total, &row.user_id, &row.pvs_m, &row.px_m,
							 &row.srs_in, &row.srs_out);
		if(n != 10)
			return -1;
	}
	return 0;
}

static int parse_table_row(char const * buf, nginx_url_popular_table_row & row)
{
	if(!buf || buf[0] == '\0')
		return -1;
	/* @see nginx/print_url_popular_table */
	char url[1500];
	url[0] = '\0';
	int n = sscanf(buf, "%ld%s%zu%zu%zu%zu%zu%zu%zu%zu%zu%zu%zu%zu%zu%zu",
				&row.datetime, url,
				&row.num_total, &row.num_200, &row.size_200, &row.num_206, &row.size_206, &row.num_301302, &row.num_304,
				&row.num_403, &row.num_404, &row.num_416, &row.num_499, &row.num_500, &row.num_502, &row.num_other);
	if(n != 16)
		return -1;
	row.url_key = url;
	return 0;
}

/* merge rows where ${datetime}  same, @see plcdn_la_options.srs_flow_merge_same_datetime */
int merge_srs_flow_datetime(FILE *& f)
{
	std::vector<srs_flow_table_row> flows;
    char buf[512];
    while (fgets(buf, sizeof buf, f)){
    	srs_flow_table_row row;
    	if(parse_table_row(buf, row) != 0)
    		continue;
    	flows.push_back(row);
    }

    f = std::freopen(NULL, "w", f);
    if(!f) return -1;
    /* std::tuple<site_id, device_id, obytes, ibytes, obps, ibps, user_id> */
    std::unordered_map<merge_key_t, srs_flow_value_t> flow_map;
    for(auto const & item : flows){
    	auto  k = std::make_tuple(item.datetime, item.site_id, item.device_id, item.user_id);
    	auto & val = flow_map[k];
    	std::get<0>(val) += item.obytes;
    	std::get<1>(val) += item.ibytes;
    	/* FIXME: use obytes/interval? */
    	std::get<2>(val) += item.obps;
    	std::get<3>(val) += item.ibps;
    }
    size_t failed_line = 0;
    for(auto const & item : flow_map){
		/* format: 'site_id datetime device_id obytes ibytes obps ibps user_id' @see fprint_srs_log_stats */
		auto sz = fprintf(f, "%d %ld %d %zu %zu %.0f %.0f %d\n", std::get<1>(item.first), std::get<0>(item.first),
				std::get<2>(item.first),
				std::get<0>(item.second), std::get<1>(item.second),
				std::get<2>(item.second), std::get<3>(item.second),
				std::get<3>(item.first));
		if(sz <= 0)
			++failed_line;
    }
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
int merge_nginx_flow_datetime(FILE *& f)
{
	std::vector<nginx_flow_table_row> rows;
    char buf[512];
    while (fgets(buf, sizeof buf, f)){
    	nginx_flow_table_row row;
    	if(parse_table_row(buf, row) != 0)
    		continue;
    	rows.push_back(row);
    }

    f = std::freopen(NULL, "w", f);
    if(!f) return -1;
    /* '${site_id} ${device_id} ${num_total} ${bytes_total} ${user_id} ${pvs_m} ${px_m} (${tx_rtmp_in} ${tx_rtmp_out})' */
    std::unordered_map<merge_key_t, nginx_flow_value_t> flow_map;
    for(auto const & item : rows){
    	auto  k = std::make_tuple(item.datetime, item.site_id, item.device_id, item.user_id);
    	auto & val = flow_map[k];
    	std::get<0>(val) += item.num_total;
    	std::get<1>(val) += item.bytes_total;
    	std::get<2>(val) += item.pvs_m;
    	std::get<3>(val) += item.px_m;
    	std::get<4>(val) += item.srs_in;
    	std::get<5>(val) += item.srs_out;
    }
    size_t failed_line = 0;
    for(auto const & item : flow_map){
		int sz = 0;
    	if(!plcdn_la_opt.append_flow_nginx){
    		/* @see nginx/print_flow_table */
			sz = fprintf(f, "%d %ld %d %zu %zu %d %zu %zu\n", std::get<1>(item.first), std::get<0>(item.first),
					std::get<2>(item.first),
					std::get<0>(item.second), std::get<1>(item.second),
					std::get<3>(item.first), std::get<2>(item.second),
					std::get<3>(item.second));
    	}
    	else{
			sz = fprintf(f, "%d %ld %d %zu %zu %d %zu %zu %zu %zu\n", std::get<1>(item.first), std::get<0>(item.first),
					std::get<2>(item.first),
					std::get<0>(item.second), std::get<1>(item.second),
					std::get<3>(item.first), std::get<2>(item.second),
					std::get<3>(item.second),
					std::get<4>(item.second), std::get<5>(item.second));
    	}
		if(sz <= 0)
			++failed_line;
    }
	return 0;
}

int merge_nginx_url_popular_datetime(FILE *& f)
{
	std::vector<nginx_url_popular_table_row> rows;
    char buf[512];
    while (fgets(buf, sizeof buf, f)){
    	nginx_url_popular_table_row row;
    	if(parse_table_row(buf, row) != 0)
    		continue;
    	rows.push_back(row);
    }

    f = std::freopen(NULL, "w", f);
    if(!f) return -1;
    std::unordered_map<merge_nginx_url_popular_key_t, nginx_url_popular_value_t> flow_map;
    for(auto const & item : rows){
    	auto  k = std::make_tuple(item.datetime, item.url_key);
    	auto & val = flow_map[k];
    	std::get<0>(val) += item.num_total;
    	std::get<1>(val) += item.num_200; std::get<2>(val) += item.size_200;
    	std::get<3>(val) += item.num_206; std::get<4>(val) += item.size_206;
    	std::get<5>(val) += item.num_301302; std::get<6>(val) += item.num_304;
    	std::get<7>(val) += item.num_403; std::get<8>(val) += item.num_404;
    	std::get<9>(val) += item.num_416; std::get<10>(val) += item.num_499;
    	std::get<11>(val) += item.num_500; std::get<12>(val) += item.num_502;
    	std::get<13>(val) += item.num_other;
    }
    size_t failed_line = 0;
    for(auto const & item : flow_map){
    	auto & k = item.first;
    	auto & v = item.second;
    	/* @see nginx/print_url_popular_table */
		auto sz = fprintf(f, "%ld %s %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu\n",
				std::get<0>(k), std::get<1>(k).c_str()
					, std::get<0>(v), std::get<1>(v), std::get<2>(v), std::get<3>(v), std::get<4>(v), std::get<5>(v), std::get<6>(v)
					, std::get<7>(v), std::get<8>(v), std::get<9>(v), std::get<10>(v), std::get<11>(v), std::get<12>(v)
					, std::get<13>(v)
					);
		if(sz <= 0)
			++failed_line;
    }
	return 0;
}
