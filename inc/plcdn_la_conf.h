/*!
 * This file is PART of nginx_log_analysis
 * data structs
 */
#ifndef _PLCDN_LA_CONF_H_
#define _PLCDN_LA_CONF_H_

/* use boost.property_tree to parse ini */
#define PLCDN_LA_USE_BOOST_INI

#ifdef PLCDN_LA_USE_BOOST_INI
//#define BOOST_NO_EXCEPTIONS
#include <boost/property_tree/ini_parser.hpp> /* boost::property_tree::ptree */
#endif /* PLCDN_LA_USE_BOOST_INI */


struct plcdn_la_conf {

#ifdef PLCDN_LA_USE_BOOST_INI
	boost::property_tree::ptree impl;
#endif /* PLCDN_LA_USE_BOOST_INI */
};

typedef plcdn_la_conf plcdn_la_conf_t;

int plcdn_la_parse_config_file(char const *f, plcdn_la_conf_t & conf);

std::string plcdn_la_config_get_string(plcdn_la_conf_t const& conf, char const * key);

#endif /*_PLCDN_LA_CONF_H_*/

