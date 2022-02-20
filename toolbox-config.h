#ifndef __C_TOOLBOX_CONFIGURATIONS_H__
#define __C_TOOLBOX_CONFIGURATIONS_H__

/////////////// TOOLBOX_Toolbox Configurations ///////////////

#define C_TOOLBOX_CHARARRAY 0
#define C_TOOLBOX_NETWORK 0
#define C_TOOLBOX_HTTP_CLIENT 0
#define C_TOOLBOX_SOCKET_SERVER 0
#define C_TOOLBOX_SOCKET_CLIENT 0

#define C_TOOLBOX_LOGIN 0
#define C_TOOLBOX_TIMING 1
#define C_TOOLBOX_LOG_COMMAND 0

#define C_TOOLBOX_THREAD_AND_MUTEX 0

#define C_TOOLBOX_LINKED_LIST 0
#define C_TOOLBOX_CONFIG_MD5 0
#define C_TOOLBOX_CONFIG_INI_MANAGER 0
#define C_TOOLBOX_CGI 0
#define C_TOOLBOX_HTTP_SERVER 0
#define C_TOOLBOX_XML 0
#define C_TOOLBOX_GENERIC_TREE 1
#define C_TOOLBOX_BINARY_TREE 0
#define C_TOOLBOX_CRYPT 0

#if ( defined(_MSC_VER) )
#	define tmpPath "c:/temp/"
#	define C_TOOLBOX_LOG_FILENAME tmpPath "c_toolbox.log"
#else
#	define tmpPath "/tmp"
#	define C_TOOLBOX_LOG_FILENAME tmpPath "/web.log"
#endif


#endif // __C_TOOLBOX_CONFIGURATIONS_H__
