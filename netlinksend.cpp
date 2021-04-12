#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>
#define COMMAND_Start_FileMonitor	   "FileMonitor_Kernel_Start"
#define COMMAND_Stop_FileMonitor	   "FileMonitor_Kernel_Stop"
#define NETLINK_NET			24 // 网络内核netlink号
#define NETLINK_FILE		25 // 文件内核netlink号
//#include <netlink/genl/genl.h>
// new netlink
#// new netlink
#define MSG_LEN         1024
#define MAX_PLOAD       4096

#define MSGLEN      128//单个文件最大完整路径长度

struct file_path_info {
    char cMsgType;
    char cRsv[3];//just covering the position
    int  nPid;
    char cProcessName[MSGLEN];
    char cOldFullPath[MSGLEN];
    char cNewFullPath[MSGLEN];
    int  nMode;          //权限
    int  nUid;           //属主
    int  nGid;           //属组
};

enum {
	MSG_LSM_TYPE_BASE = 0,
	MSG_TYPE_FILE_CREATE,
	MSG_TYPE_FILE_OPEN,
	MSG_TYPE_FILE_READ,
	MSG_TYPE_FILE_WRITE,
	MSG_TYPE_FILE_COPY,
	MSG_TYPE_FILE_MOVE,
	MSG_TYPE_FILE_DELETE,
	MSG_TYPE_FILE_CHMOD,
	MSG_TYPE_DIR_CREATE,
	MSG_TYPE_DIR_DELETE,
	MSG_TYPE_DIR_OPEN
};
typedef struct _user_msg_info
{
    struct nlmsghdr hdr;
    char  msg[MSG_LEN];
} user_msg_info;

void SendFileInfo(struct file_path_info * pathInfo)
{
 // 是删除目录
    if (pathInfo->cMsgType == MSG_TYPE_DIR_DELETE) {
        //判断末尾是否有'/'，没有则添加
        int nLen = strlen(pathInfo->cOldFullPath);
        if(pathInfo->cOldFullPath[nLen - 1] != '/') {
            sprintf(pathInfo->cOldFullPath, "%s%c", pathInfo->cOldFullPath, '/');
        }

        // 测试
        if (strncmp(pathInfo->cOldFullPath, "/root/jason/debug", strlen("/root/jason/debug")) == 0) {
            printf("dsfsdf");
        }
    }

    return;
}
static int openNetlink(int port)
{
    /* 创建NETLINK socket */
    int skfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_FILE);
    if(skfd == -1)
    {
        
        return -1;
    }

    int nOne = 1;
    setsockopt(skfd, SOL_SOCKET, SO_REUSEADDR, &nOne, sizeof(nOne));

    struct sockaddr_nl saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.nl_family = AF_NETLINK; //AF_NETLINK
    saddr.nl_pid = port;  //端口号(port ID) 
    saddr.nl_groups = 0;
    if(bind(skfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
    {
         
        close(skfd);
        return -1;
    }

    return skfd;
}

static int readNetlink(int skfd)
{
    int ret;
    user_msg_info u_info;
    socklen_t len;
    struct nlmsghdr *nlh = NULL;
    struct sockaddr_nl daddr;

    while (true) {
        memset(&u_info, 0, sizeof(u_info));
        len = sizeof(struct sockaddr_nl);
        ret = recvfrom(skfd, &u_info, sizeof(user_msg_info), 0, (struct sockaddr *)&daddr, &len);
        if(!ret)
        {
             
            continue;
        }

        file_path_info *path_info = (file_path_info *) u_info.msg;
        SendFileInfo(path_info);
    }

    return 0;
}

static int sendCmdToKernel(const char *umsg)
{
    int skfd;
    int ret;
    user_msg_info u_info;
    socklen_t len;
    struct nlmsghdr *nlh = NULL;
    struct sockaddr_nl saddr, daddr;

    /* 创建NETLINK socket */
    skfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_FILE);
    if(skfd == -1)
    {
        
        return -1;
    }

  
    memset(&daddr, 0, sizeof(daddr));
    daddr.nl_family = AF_NETLINK;
    daddr.nl_pid = 0; // to kernel 
    daddr.nl_groups = 0;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PLOAD));
    memset(nlh, 0, sizeof(struct nlmsghdr));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PLOAD);
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_type = 0;
    nlh->nlmsg_seq = 0;
    nlh->nlmsg_pid = getpid();//saddr.nl_pid; //self port

    memcpy(NLMSG_DATA(nlh), umsg, strlen(umsg) + 1);
    ret = sendto(skfd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&daddr, sizeof(struct sockaddr_nl));
    if(!ret)
    {
        
        close(skfd);
        return -1;
    }
  

    close(skfd);

    free((void *)nlh);
    return 0;
}
// new netlink
int main()
{
     if (sendCmdToKernel(COMMAND_Start_FileMonitor) != 0)
{
    printf("hello,world");
}
    return 0;
}
// new netlink
