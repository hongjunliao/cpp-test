/*!
 * This file is Part of cpp-test
 * @author: hongjun.liao<docici@126.com>
 */
#ifdef WITH_DBUS
#include "cpp_test.h"
#include "hp/hp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>
#include <unistd.h>

/////////////////////////////////////////////////////////////////////////////////////
//https://thebigdoc.readthedocs.io/en/latest/dbus/dbus.html

//4.1.6. Signal的收发的例子
static int send_signal(char *sigvalue)
{
	DBusError err;
	DBusConnection *connection;
	DBusMessage *msg;
	DBusMessageIter arg;
	dbus_uint32_t serial = 0;
	int ret;

	//步骤1:建立与D-Bus后台的连接
	/* initialise the erroes */
	dbus_error_init(&err);
	/* Connect to Bus*/
	connection = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) {
		hp_log(stderr, "Connection Err : %s\n", err.message);
		dbus_error_free(&err);
	}
	if (connection == NULL) {
		return -1;
	}

	//步骤2:给连接名分配一个well-known的名字作为Bus name，这个步骤不是必须的，可以用if 0来注释着一段代码，我们可以用这个名字来检查，是否已经开启了这个应用的另外的进程。
#if 1
	ret = dbus_bus_request_name(connection, "test.singal.source",
			DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
	if (dbus_error_is_set(&err)) {
		hp_log(stderr, "Name Err : %s\n", err.message);
		dbus_error_free(&err);
	}
	if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
		return -1;
#endif

	//步骤3:发送一个信号
	//根据图，我们给出这个信号的路径（即可以指向对象），接口，以及信号名，创建一个Message
	if ((msg = dbus_message_new_signal("/test/signal/Object",
			"test.signal.Type", "Test")) == NULL) {
		hp_log(stderr, "Message NULL\n");
		return -1;
	}
	//给这个信号（messge）具体的内容
	dbus_message_iter_init_append(msg, &arg);
	if (!dbus_message_iter_append_basic(&arg, DBUS_TYPE_STRING, &sigvalue)) {
		hp_log(stderr, "Out Of Memory!\n");
		return -1;
	}

	//步骤4: 将信号从连接中发送
	if (!dbus_connection_send(connection, msg, &serial)) {
		hp_log(stderr, "Out of Memory!\n");
		return -1;
	}
	dbus_connection_flush(connection);
	printf("Signal Send\n");

	//步骤5: 释放相关的分配的内存。
	dbus_message_unref(msg);
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////

//4.1.7. Method的收发小例子
//4.1.7.1. 方法监听的例子
static int listen_signal()
{
	DBusMessage *msg;
	DBusMessageIter arg;
	DBusConnection *connection;
	DBusError err;
	int ret;
	char *sigvalue;

	//步骤1:建立与D-Bus后台的连接
	dbus_error_init(&err);
	connection = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) {
		hp_log(stderr, "Connection Error %s\n", err.message);
		dbus_error_free(&err);
	}
	if (connection == NULL) {
		return;
	}

	//步骤2:给连接名分配一个可记忆名字test.singal.dest作为Bus name，这个步骤不是必须的,但推荐这样处理
	ret = dbus_bus_request_name(connection, "test.singal.dest",
			DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
	if (dbus_error_is_set(&err)) {
		hp_log(stderr, "Name Error %s\n", err.message);
		dbus_error_free(&err);
	}
	if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		return;
	}

	//步骤3:通知D-Bus daemon，希望监听来行接口test.signal.Type的信号
	dbus_bus_add_match(connection, "type='signal',interface='test.signal.Type'",
			&err);
	//实际需要发送东西给daemon来通知希望监听的内容，所以需要flush
	dbus_connection_flush(connection);
	if (dbus_error_is_set(&err)) {
		hp_log(stderr, "Match Error %s\n", err.message);
		dbus_error_free(&err);
	}

	//步骤4:在循环中监听，每隔开1秒，就去试图自己的连接中获取这个信号。这里给出的是从连接中获取任何消息的方式，所以获取后去检查一下这个消息是否我们期望的信号，并获取内容。我们也可以通过这个方式来获取method call消息。
	while (1) {
		dbus_connection_read_write(connection, 0);
		msg = dbus_connection_pop_message(connection);
		if (msg == NULL) {
			sleep(1);
			continue;
		}

		if (dbus_message_is_signal(msg, "test.signal.Type", "Test")) {
			if (!dbus_message_iter_init(msg, &arg)) {
				hp_log(stderr, "Message Has no Param");
			} else if (dbus_message_iter_get_arg_type(&arg)
					!= DBUS_TYPE_STRING) {
				hp_log(stderr, "Param is not string");
			} else {
				dbus_message_iter_get_basic(&arg, &sigvalue);
				printf("Got Singal with value : %s\n", sigvalue);
			}
		}
		dbus_message_unref(msg);
	}	//End of while

	return 0;
}

//4.1.7.2. 方法调用的例子
/*读取消息的参数，并且返回两个参数，一个是bool值stat，一个是整数level*/
static int reply_to_method_call(DBusMessage * msg, DBusConnection * conn)
{
    DBusMessage * reply;
    DBusMessageIter arg;
    char * param = NULL;
    dbus_bool_t stat = TRUE;
    dbus_uint32_t level = 2010;
    dbus_uint32_t serial = 0;

    //从msg中读取参数，这个在上一次学习中学过
    if(!dbus_message_iter_init(msg,&arg)) {
        printf("Message has no args/n");
    } else if(dbus_message_iter_get_arg_type(&arg) != DBUS_TYPE_STRING) {
        printf("Arg is not string!/n");
    } else {
        dbus_message_iter_get_basic(&arg,& param);
    }
    if(param == NULL) return -1;

    hp_log(stdout, "method_call: '%.*s', reply with %d,%d\n", 16, param, level, stat);

    //创建返回消息reply
    reply = dbus_message_new_method_return(msg);
    //在返回消息中填入两个参数，和信号加入参数的方式是一样的。这次我们将加入两个参数。
    dbus_message_iter_init_append(reply,&arg);
    if(!dbus_message_iter_append_basic(&arg,DBUS_TYPE_BOOLEAN,&stat)) {
        printf("Out of Memory!/n");
        exit(1);
    }
    if(!dbus_message_iter_append_basic(&arg,DBUS_TYPE_UINT32,&level)) {
        printf("Out of Memory!/n");
        exit(1);
    }
    //发送返回消息
    if( !dbus_connection_send(conn, reply, &serial)){
        printf("Out of Memory/n");
        exit(1);
    }
    dbus_connection_flush (conn);
    dbus_message_unref (reply);
	return 0;
}

/* 监听D-Bus消息，我们在上次的例子中进行修改 */
static int listen_dbus()
{
    DBusMessage * msg;
    DBusMessageIter arg;
    DBusConnection * connection;
    DBusError err;
    int ret;
    char * sigvalue;

    dbus_error_init(&err);
    //创建于session D-Bus的连接
    connection = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if(dbus_error_is_set(&err)){
        fprintf(stderr,"Connection Error %s/n",err.message);
        dbus_error_free(&err);
    }
    if(connection == NULL) {
        return -1;
    }
    //设置一个BUS name：test.wei.dest
    ret = dbus_bus_request_name(connection,"test.wei.dest",DBUS_NAME_FLAG_REPLACE_EXISTING,&err);
    if(dbus_error_is_set(&err)) {
        fprintf(stderr,"Name Error %s/n",err.message);
        dbus_error_free(&err);
    }
    if(ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        return -1;
    }

    //要求监听某个singal：来自接口test.signal.Type的信号
    dbus_bus_add_match(connection,"type='signal',interface='test.signal.Type'",&err);
    dbus_connection_flush(connection);
    if(dbus_error_is_set(&err)){
        fprintf(stderr,"Match Error %s/n",err.message);
        dbus_error_free(&err);
    }

    while(TRUE){
        dbus_connection_read_write (connection,0);
        msg = dbus_connection_pop_message (connection);

        if(msg == NULL){
            sleep(1);
            continue;
        }

        if(dbus_message_is_signal(msg,"test.signal.Type","Test")){
            if(!dbus_message_iter_init(msg,&arg)) {
                fprintf(stderr,"Message Has no Param");
            } else if(dbus_message_iter_get_arg_type(&arg) != DBUS_TYPE_STRING) {
                printf("Param is not string");
            } else {
                dbus_message_iter_get_basic(&arg,&sigvalue);
                printf("Got Singal with value : %s\n",sigvalue);
            }
        }else if(dbus_message_is_method_call(msg,"test.method.Type","Method")){
            //我们这里面先比较了接口名字和方法名字，实际上应当现比较路径
            if(strcmp(dbus_message_get_path (msg),"/test/method/Object") == 0) {
                reply_to_method_call(msg, connection);
            }
        }
        dbus_message_unref(msg);
    }
	return 0;
}

//建立与session D-Bus daemo的连接，并设定连接的名字，相关的代码已经多次使用过了
DBusConnection *  connect_dbus()
{
    DBusError err;
    DBusConnection * connection;
    int ret;

    //Step 1: connecting session bus
    /* initialise the erroes */
    dbus_error_init(&err);
    /* Connect to Bus*/
    connection = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if(dbus_error_is_set(&err)){
        fprintf(stderr,"Connection Err : %s\n",err.message);
        dbus_error_free(&err);
    }
    if(connection == NULL) {
        return NULL;
    }

    //step 2: 设置BUS name，也即连接的名字。
    ret = dbus_bus_request_name(connection,"test.wei.source",DBUS_NAME_FLAG_REPLACE_EXISTING,&err);
    if(dbus_error_is_set(&err)) {
        fprintf(stderr,"Name Err : %s\n",err.message);
        dbus_error_free(&err);
    }

    if(ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        return NULL;
    }

    return connection;
}

void method_call(DBusConnection * connection,char * param)
{
    connection = connect_dbus();
    if(connection == NULL) {
        return -1;
    }

    DBusError err;
    DBusMessage * msg;
    DBusMessageIter    arg;
    DBusPendingCall * pending;
    dbus_bool_t * stat;
    dbus_uint32_t * level;

    dbus_error_init(&err);

    //针对目的地地址，请参考图，创建一个method call消息。 Constructs a new message to invoke a method on a remote object.
    msg = dbus_message_new_method_call ("test.wei.dest","/test/method/Object","test.method.Type","Method");
    if(msg == NULL) {
        hp_log(stderr, "Message NULL");
        return;
    }

    //为消息添加参数。Append arguments
    dbus_message_iter_init_append(msg, &arg);
    if(!dbus_message_iter_append_basic (&arg, DBUS_TYPE_STRING,&param)) {
        hp_log(stderr, "Out of Memory!");
        exit(1);
    }

    //发送消息并获得reply的handle 。Queues a message to send, as with dbus_connection_send() , but also returns a DBusPendingCall used to receive a reply to the message.
    if(!dbus_connection_send_with_reply(connection, msg,&pending, -1)){
        hp_log(stderr, "Out of Memory!");
        exit(1);
    }

    if(pending == NULL) {
        hp_log(stderr, "Pending Call NULL: connection is disconnected ");
        dbus_message_unref(msg);
        return;
    }

    dbus_connection_flush(connection);
    dbus_message_unref(msg);

    //waiting a reply，在发送的时候，已经获取了method reply的handle，类型为DBusPendingCall。
    // block until we recieve a reply， Block until the pending call is completed.
    dbus_pending_call_block (pending);
    // get the reply message，Gets the reply, or returns NULL if none has been received yet.
    msg = dbus_pending_call_steal_reply (pending);
    if (msg == NULL) {
        fprintf(stderr, "Reply Null\n");
         exit(1);
    }
     // free the pending message handle
     dbus_pending_call_unref(pending);
    // read the parameters
    if (!dbus_message_iter_init(msg, &arg)) {
        fprintf(stderr, "Message has no arguments!\n");
    } else if ( dbus_message_iter_get_arg_type (&arg) != DBUS_TYPE_BOOLEAN) {
        fprintf(stderr, "Argument is not boolean!\n");
    } else {
        dbus_message_iter_get_basic (&arg, &stat);
    }

    if (!dbus_message_iter_next(&arg)) {
        fprintf(stderr, "Message has too few arguments!\n");
    } else if ( dbus_message_iter_get_arg_type (&arg) != DBUS_TYPE_UINT32 ) {
        fprintf(stderr, "Argument is not int!\n");
    } else {
        dbus_message_iter_get_basic (&arg, &level);
    }

    printf("Got Reply: %d, %d\n", (int)stat, (int)level);
    dbus_message_unref(msg);
}

int test_dbus_main(int argc, char **argv) {
	hp_log(stdout, "%s send_signal | listen_signal, listen_dbus | method_call\n", argv[0]);
	if( argc < 2) argv[1] = "send_signal";

	if (strcmp("send_signal", argv[1]) == 0) send_signal("Hello,world!");
	else if (strcmp("listen_signal", argv[1]) == 0) listen_signal();
	else if (strcmp("listen_dbus", argv[1]) == 0) listen_dbus();
	else if (strcmp("method_call", argv[1]) == 0) method_call(0, "Hello, D-Bus");

	return 0;
}

#endif //#ifdef WITH_DBUS
