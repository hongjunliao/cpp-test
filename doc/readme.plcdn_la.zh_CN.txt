!!!IMPORTANT!!!
本说明文档可能未及时更新, 若与plcdn_la --help输出描述有出入, 请以plcdn_la --help为准!

plcdn_la日志分析程序中文帮助文档 
编写: hongjun.liao <docici@126.com> 
更新日期: 2017/05/20

##概述
nginx, srs日志转储分析程序

##简介
plcdn_la能分析特定格式nginx和srs日志文件, 输出相应统计结果表,并能合并srs流量到nginx流量表, 特性:
1.支持同时分析nginx和srs日志文件
2.支持输出文件名格式自定义
3.支持按指定格式拆分输入的日志文件
4.支持合并其它流量(目前是srs)到nginx流量
5.支持按用户合并srs流量
6.静态链接,程序无依赖,能直接复制到各种linux发行版本运行
7.支持日志转储及分析

##术语说明
1.转储: 移走nginx,srs服务进程的日志文件, 向nginx,srs服务进程发送"reopen"或类似信号,使nginx或srs重新打开日志文件; 
        通过这种方式, 避免了nginx,srs服务进程的日志文件增长过大

##plcdn_la参数说明
  -l, --nginx-log-file=STRING                  指定nginx日志文件
      --nginx-rotate-dir=STRING                指定nginx转储目录
      --nginx-rotate-time=INT                  指定nginx转储时间, 超过转储时间的日志文件将被删除
      --begin-time=STRING                      
      --end-time=STRING                        
											   要分析的起止时间, 可以用此限定只分析指定时间范围内的日志;可指定为0,或1，详见NOTES
      --no-merge-datetime                      不要合并时间相同的记录;如果输出文件存在且本次统计结果与文件中有主键相同的字段，
                                               那么默认会进行合并，打开此选项禁止合并
  -d, --device-list-file=STRING                devicelist_file, 详见NOTES
  -s, --siteuid-list-file=STRING               siteuidlist_file， 详见NOTES
  -m, --ipmap-file=STRING                      ipmap_file，IP数据库文件，详见NOTES
      --parse-url-mode=INT                     解析nginx '$request_uri' 中url的模式, 0|1|2, 当前使用模式2
  -i, --interval=INT                           统计分组的时间间隔, 如300,即每5min统计一次
  -n, --srs-log-file=STRING                    指定srs日志文件
      --srs-calc-flow-mode=INT                 srs流量计算方式, 仅针对自定义格式: 通过直接累计流量字段还是通过累计时间×带宽得出的流量值，0|1
  -k, --output-srs-sid=STRING                  目录用于存储srs sid日志, sid日志包含连接信息
  -b, --output-srs-flow=STRING                 目录用于存储srs_flow_table,如果NULL则禁用
  -B, --format-srs-flow=STRING                 srs_flow_table文件名格式, 默认 'srscountfile.${day}.${site_id}.${device_id}'
      --output-split-srs-log-by-sid=STRING     目录用于存储按sid拆分的日志, 仅调试使用
  -o, --output-nginx-flow=STRING               目录用于存储nginx_flow_table,如果NULL则禁用
  -O, --format-nginx-flow=STRING               nginx_flow_table文件名格式, 默认 'countfile.${interval}.${site_id}.${device_id}'
  -u, --output-file-url-popular=STRING         目录用于存储url_popular_table, 如果NULL则禁用
  -U, --format-url-popular=STRING              filename format for url_popular table, default
                                               'urlstat.${interval}.${site_id}.${device_id}'
  -p, --output-file-ip-popular=STRING          目录用于存储ip_popular_table, 如果NULL则禁用
  -P, --format-ip-popular=STRING               filename format for ip_popular table, default 'UANStats.${datetime}.${device_id}'
      --min-ip-popular=INT                     min_access_count, filter for ip_popular table, default 10(>=), disabled if negative
      --max-ip-popular=INT                     max_access_count, filter for ip_popular table, default unlimited(<=), disabled if
                                               negative
  -t, --output-file-http-stats=STRING          目录用于存储http_stats_table, 如果NULL则禁用
  -T, --format-http-stats=STRING               filename format for http_stats table, default 'DASStats.${datetime}.${device_id}'
  -w, --output-file-ip-slowfast=STRING         目录用于存储ip_slowfast_table, 如果NULL则禁用
  -W, --format-ip-slowfast=STRING              filename format for ip_slowfast table, default
                                               'UASStats.${interval}.${site_id}.${device_id}'
  -f, --output-file-cutip-slowfast=STRING      目录用于存储cutip_slowfast_table, 如果NULL则禁用
  -F, --format-cutip-slowfast=STRING           filename format for cutip_slowfast table, default 'ASStats.${datetime}.${device_id}'
  -r, --output-file-ip-source=STRING           目录用于存储ip_source_table, 如果NULL则禁用
  -R, --format-ip-source=STRING                filename format for ip_source table, default 'IPSource.${interval}.${site_id}.${device_id}'
  -g, --output-split-nginx-log=STRING          目录用于存储拆分的nginx日志, 如果NULL则禁用
  -G, --format-split-nginx-log=STRING          filename format for split_nginx_log, default '${site_id}/${day}'
  -j, --output-split-srs-log=STRING            目录用于存储拆分的srs日志, 如果NULL则禁用
  -J, --format-split-srs-log=STRING            拆分的srs日志文件名格式, default '${site_id}/${day}'
      --device-id=INT                          指定device_id
  -c, --print-divice-id                        输出 device_id并退出
      --enable-multi-thread                    enable_multi_thread, ONLY for nginx yet(除非你明白整个流程，否则不要打开)
      --merge-srs-flow                         设置工作模式为merge_srs_flow
      --append-flow-nginx                      如果设置了,合并其它的flow(现在是srs)到nginx flow
      --log-file                               输出错误日志到文件，而不是stderr
      --config-file                            从配置文件中载入配置，详见doc/.plcdn_la; 注意：命令行参数的值将会覆盖其应对的值
  -h, --help                                   输出本帮助
      --version                                输出版本信息并退出程序
  -v, --verbose=INT                            输出工作时的详细信息, 值越大越详细,0以禁止输出

##备注
  1.work_mode, 工作模式
    analysis, 分析模式: 分析日志文件并输出结果表
    rotate, 转储模式: 类似分析模式, 指定转储目录, plcdn_la将会把日志文件的每一行追加到转储目录下相应日期的文件中, 再重新分析转储目录
    merge_srs_flow, 合并srs流量: 按用户合并srs_flow_table(use --merge-srs-flow). 输出格式: '${datetime} ${obytes} ${ibytes} ${obps} ${ibps} ${user_id}'
  2.about 'filename format'(option --format-*, e.g. --format-ip-source):
    ${datetime}   current date time, format YYYYmmDDHHMM
    ${interval}   according to option --interval, in minute, format YYYYmmDDHHMM
    ${day}        $time_local in log, in day, format YYYYmmDD
    ${device_id}  device_id
    ${site_id}    site_id/domain_id
    ${user_id}    user_id
    ${domain}     domain
  3.时间范围格式 'YYYY-mm-dd'(sample '2017-02-14'), range in [begin_time,end_time) (左闭右开区间)
    default disabled, applied for both nginx and srs log if enabled
  4.使用ulimit(or other command)命令调整每个进程可打开的文件数量
  5.srs是什么: https://github.com/ossrs/srs/wiki/v2_CN_Home
  6.支持的nginx日志格式: $host $remote_addr $request_time_msec $cache_status [$time_local] "$request_method $request_uri $server_protocol" $status $bytes_sent "$http_referer$remote_user$http_cookie$http_user_agent" $scheme $request_length $upstream_response_time
  7.DO NOT mix up option '--output-srs-sid' with '--output-split-srs-log' when split srs log!
  8.输出表格式
    for nginx:
    (1)nginx_flow_table:     '${site_id} ${datetime} ${device_id} ${num_total} ${bytes_total} ${user_id} ${pvs_m} ${px_m} ${tx_rtmp_in} ${tx_rtmp_out}'
    (2)url_popular_table:    '${datetime} ${url_key} ${num_total} ${num_200} ${size_200} ${num_206} ${size_206} ${num_301302}
                                ${num_304} ${num_403} ${num_404} ${num_416} ${num_499} ${num_500} ${num_502} ${num_other}'
    (3)ip_popular_table:     '${site_id} ${device_id} ${ip} ${datetime} ${num}'
    (4)http_stats_table:     '${site_id} ${device_id} ${httpstatus} ${datetime} ${num} ${num_m}'
    (5)ip_slowfast_table:    '${device_id} ${ip} ${datetime} ${speed} ${type}'
    (6)cutip_slowfast_table: '${device_id} ${datetime} ${ip} ${speed}'
    (7)ip_source_table:      '${datetime} ${local_id} ${isp_id} ${pvs} ${tx} ${pvs_m} ${tx_m} ${device_id}'
    for srs:
    (1)srs_flow_table:       '${site_id} ${datetime} ${device_id} ${obytes} ${ibytes} ${obps} ${ibps} ${user_id}'
  9.srs日志中增加了定义格式, for srs log, currently supports 2 formats of trans_log, 0: official , 1: custom
    >>sample official_format: '[2016-12-07 14:49:54.057][trace][20308][105] <- CPB time=1980013, \
      okbps=0,0,0, ikbps=477,428,472, mr=0/350, p1stpt=20000, pnt=20000'
    >>sample custom_format: '[2017-02-07 15:03:31.138][trace][6946][107] time=3460008, type=CPB, ip=127.0.0.1, \
      tcUrl=rtmp://localhost/live, vhost=__defaultVhost__, obytes=4187, ibytes=206957159, okbps=0,0,0, ikbps=475,580,471'
  10.about option --srs-calc-flow-mode, 0: use obytes/ibytes, 1: use okbps/ikbps
  11.file '--device-list-file' format: '${device_id} ${device_ip}'
  12.file '--siteuid-list-file' format: '${site_id} ${user_id} ${domain}'

##使用示例
1.分析nginx日志文件access.log, 并输出所有流量到/tmp/nginx_flow.txt
./plcdn_la -l  nginx.log -o /tmp/ -O nginx_flow.txt

2.分析srs日志文件srs.log, 并输出所有流量到/tmp/srs_flow.txt, 输出详细信息,级别为2
./plcdn_la -n srs.log  -v2 -k /tmp/ -b /tmp/ -B srs_flow.txt

3.分析nginx,srs日志文件, 输出相应流量表, 追加srs流量到nginx流量
./plcdn_la -l nginx.log -o /tmp/ -O nginx_flow.txt -v5 -n srs.log -b /tmp/ -B srs_flow.txt --append-flow-nginx

4.从配置文件'.plcdn_la'中载入相关参数，而不是从命令行指定,但覆盖-l参数
./plcdn_la　--config-file .plcdn_la -l nginx.log

5.使用配置文件转储及分析
./nginx_log_analysis nginx_log_analysis_conf

6.日志分析：一个复杂示例
./plcdn_la -i 300 -d /opt/fenxi/devicelist.txt -s /opt/fenxi/siteuidlist.txt -m /opt/fenxi3/src/iplocaltion.bin \
-l /var/log/nginx/tmp_acslogs/bdrz.log -o /data/fenxi_file/ -u /data/urlstat/ -w /data/UseAccessSpeedStats/ \
-p /data/UseAccessNumStats/ -t /data/DomainAccessStatusStats/ -f /data/asstats/ -r /data/IPSource/ -y /data/UrlKey/ \
--local-url-key /data/localeUrlKey/ -g /data/fenxi_log/ -G '${site_id}/${day}/${interval}' \
--nginx-hit 'STALE|UPDATING|REVALIDATED|HIT' --nginx-rotate-dir /opt/fenxi//rotate/ --end-time 1

7.按用户ID合并srs流量表１，2到stdout
cat 1 2 | plcdn_la --merge-srs-flow

8.转换nginx日志格式到格式２(用户＇云端＇的日志格式)
cat nginx.log | plcdn_la --nginx-transform-log 2

##其它
1.云端日志格式说明
(1)有两个版本: 1.10.3, 1.12.0, 字段说明见yunduan_log_format.txt
(2)版本1.10.3的样本日志见yunduan_1_10_3.log
(3)版本1.12.0的样本日志见yunduan_1_12_0.log

2.旧版本(yong.lu版)的nginx日志分析程序参见：/home/jun/plcdn/plcdn-lame-importer-bin.git

##参见
(1)plcdn_la_dev_doc.txt
(2)<卢永交接文档>
