/*!
 * This file is PART of plcdn_log_analysis
 * @author: hongjun.liao<docici@126.com>
 * @desc: error code for plcdn_la
 */
#ifndef PLCDN_LA_ERRNO_H_
#define PLCDN_LA_ERRNO_H_
#ifdef __cplusplus
extern "C"{
#endif

/* error code for plcdn_la,
 * @NOTE: these error codes only used in test_plcdn_log_analysis_main */
#define PLA_ERR_OK                 0   /* no error */
#define PLA_ERR_DEVICLIST          101 /* load devicelist failed */
#define PLA_ERR_SITEUIDLIST        102 /* load siteuidlist failed */
#define PLA_ERR_IPMAP              103 /* load ipmap failed */
#define PLA_ERR_NGX_FILE           104 /* cannot load nginx log */
#define PLA_ERR_NGX_ANALYSIS       105 /* analysis nginx log failed */
#define PLA_ERR_SRS_FILE           106 /* cannot load srs log */
#define PLA_ERR_SRS_ANALYSIS       107 /* analysis srs log failed */
#define PLA_ERR_LOG_FILE           108 /* no log file can be read */
#define PLA_ERR_LOG_TRANS          109 /* transform log format failed */
#define PLA_ERR_SRS_MERGE_USER     110 /* merge srs flow for user failed */

#ifdef __cplusplus
}
#endif	//__cplusplus

#endif /* PLCDN_LA_ERRNO_H_ */
