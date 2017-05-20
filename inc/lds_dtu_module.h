/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun.liao <docici@126.com>, @date 2017/05/20
 * dtu module
 */
#ifndef LDS_DTU_MODULE_H_
#define LDS_DTU_MODULE_H_

#if !(defined __GCC__ || defined __CYGWIN__)
#include <windows.h>
//#include "D:/libs/landmark/inc/gprsdll.h"
//数据结构：
//1．用以区分标识各台Modem的数据结构：
typedef UINT u32t;
typedef UCHAR u8t;
typedef USHORT u16t;
typedef ULONG	u64t;
typedef struct _modem_info_t_
{
	u32t	m_modemId;   	  //Modem模块的ID号
	u8t		m_phoneno[12];	  //Modem的11位电话号码，必须以'\0'字符结尾
	u8t		m_dynip[4];		  //Modem的4位动态ip地址
	u64t	m_conn_time; 	  //Modem模块最后一次建立TCP连接的时间
	u64t	m_refresh_time;    //Modem模块最后一次收发数据的时间
} ModemInfoStruct;

#define MAX_RECEIVE_BUF 1450
typedef struct _modem_data_t {
	   u32t       m_modemId;		     	// Modem模块的ID号
	   u64t       m_recv_time;				//接收到数据包的时间
	   u8t        m_data_buf[MAX_RECEIVE_BUF+1];//存储接收到的数据
	   u16t     m_data_len;				//接收到的数据包长度
	   u8t      m_data_type;	          	//接收到的数据包类型,
											//	0x01：用户数据包
											//	0x02：对控制命令帧的回应
}ModemDataStruct;

int lds_dtu_module_load(char const * s);
/* call lds_dtu_module_load before call the flowing functions */
int lds_dtu_start_service(u16t port);
int lds_dtu_stop_service();
int lds_dtu_get_next_data(ModemDataStruct & d, u16t wait);

#endif /* !(defined __GCC__ || defined __CYGWIN__) */

#endif /* LDS_DTU_MODULE_H_ */
