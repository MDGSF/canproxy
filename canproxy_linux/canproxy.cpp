#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <syslog.h>
#include <termios.h>
#include <unistd.h>

///////////////////////////////////////////////////////

bool bIsBigEndian() {
  union endian_tester {
    unsigned int m_int;
    unsigned char m_byte[4];
  };

  endian_tester test;
  test.m_int = 0x0a0b0c0d;

  if (test.m_byte[0] == 0x0a) {
    return true;
  }
  return false;
}

int32_t iEncode64(uint64_t ullValue, char** ppcBuf, uint16_t* pusEncodeLen) {
  if (NULL == ppcBuf || NULL == *ppcBuf || NULL == pusEncodeLen) {
    return EXIT_FAILURE;
  }

  char* pcValue = (char*)&ullValue;
  if (bIsBigEndian()) {
    memcpy(*ppcBuf, pcValue, sizeof(uint64_t));
  } else {
    (*ppcBuf)[7] = pcValue[0];
    (*ppcBuf)[6] = pcValue[1];
    (*ppcBuf)[5] = pcValue[2];
    (*ppcBuf)[4] = pcValue[3];
    (*ppcBuf)[3] = pcValue[4];
    (*ppcBuf)[2] = pcValue[5];
    (*ppcBuf)[1] = pcValue[6];
    (*ppcBuf)[0] = pcValue[7];
  }

  *ppcBuf += sizeof(uint64_t);
  *pusEncodeLen += sizeof(uint64_t);
  return EXIT_SUCCESS;
}

int32_t iDecode64(uint64_t* pullValue, char** ppcMsg, uint16_t* pusMsgLen) {
  if (NULL == pullValue || NULL == ppcMsg || NULL == *ppcMsg ||
      NULL == pusMsgLen || *pusMsgLen < sizeof(uint64_t)) {
    return EXIT_FAILURE;
  }

  char* pcValue = (char*)pullValue;
  if (bIsBigEndian()) {
    memcpy(pcValue, *ppcMsg, sizeof(uint64_t));
  } else {
    pcValue[7] = (*ppcMsg)[0];
    pcValue[6] = (*ppcMsg)[1];
    pcValue[5] = (*ppcMsg)[2];
    pcValue[4] = (*ppcMsg)[3];
    pcValue[3] = (*ppcMsg)[4];
    pcValue[2] = (*ppcMsg)[5];
    pcValue[1] = (*ppcMsg)[6];
    pcValue[0] = (*ppcMsg)[7];
  }

  *ppcMsg += sizeof(uint64_t);
  *pusMsgLen -= sizeof(uint64_t);
  return EXIT_SUCCESS;
}

///////////////////////////////////////////////////////

#define DEFAULT_CAN_NAME "can1"
#define DEFAULT_NAME "ctun1"
#define BUF_LEN 4096
#define DEFAULT_CAN_ID 0x208
#define DEFAULT_CAN_DATA_SIZE 8

static volatile int running = 1;

void sigterm(int signo) { running = 0; }

int createCanFd() {
  int ret = 0;

  int fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (fd < 0) {
    printf("create can socket fd failed, err = %s\n", strerror(errno));
    return -1;
  }

  struct ifreq ifr;
  strcpy(ifr.ifr_name, DEFAULT_CAN_NAME);
  ret = ioctl(fd, SIOCGIFINDEX, &ifr);
  if (ret < 0) {
    printf("ioctl SIOCGIFINDEX failed, err = %s\n", strerror(errno));
    close(fd);
    return -1;
  }

  // bind fd to socket can
  struct sockaddr_can addr;
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;
  ret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
  if (ret < 0) {
    printf("bind fd to can failed, err = %s\n", strerror(errno));
    close(fd);
    return -1;
  }

  //设置规则，只接受 can_id = 0x208 的数据包
  struct can_filter rfilter[1];
  rfilter[0].can_id = DEFAULT_CAN_ID;
  rfilter[0].can_mask = CAN_SFF_MASK;
  ret = setsockopt(fd, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
  if (ret < 0) {
    printf("setsockopt CAN_RAW_FILTER failed, err = %s\n", strerror(errno));
    close(fd);
    return -1;
  }

  // 回环功能，0 表示关闭, 1 表示开启( 默认)
  int loopback = 0;
  setsockopt(fd, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback));

  // 开启之后，可以收到由自己这个 fd 发送出去的消息。
  // int recv_own_msgs = 1;
  // setsockopt(fd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &recv_own_msgs,
  //            sizeof(recv_own_msgs));
  // if (ret < 0) {
  //   printf("setsockopt CAN_RAW_RECV_OWN_MSGS failed, err = %s\n",
  //   strerror(errno)); return -1;
  // }

  return fd;
}

int iCanRead(int s, char* pcBuf, int iLen) {
  struct can_frame stCanHeader;
  int ret = read(s, &stCanHeader, sizeof(stCanHeader));
  if (ret < 0) {
    printf("read can header failed, err = %s\n", strerror(errno));
    return -1;
  }
  if (stCanHeader.can_dlc != 8) {
    return 0;
  }

  uint64_t u64PayloadCanFrameNum = 0;
  char* pcData = (char*)stCanHeader.data;
  uint16_t usDataLen = stCanHeader.can_dlc;
  if (EXIT_SUCCESS != iDecode64(&u64PayloadCanFrameNum, &pcData, &usDataLen)) {
    printf("decode header from can data failed\n");
    return -1;
  }

  char* pcBufOffset = pcBuf;
  for (int i = 0; i < u64PayloadCanFrameNum; i++) {
    struct can_frame frame;
    ret = read(s, &frame, sizeof(frame));
    if (ret < 0) {
      printf("read can pay load failed, err = %s\n", strerror(errno));
      return -1;
    }

    memcpy(pcBufOffset, frame.data, frame.can_dlc);
    pcBufOffset += frame.can_dlc;
  }
  return pcBufOffset - pcBuf;
}

int iCanWrite(int s, const char* pcBuf, int iLen) {
  // send header
  uint64_t u64PayloadCanFrameNum = iLen / 8;
  if (iLen % 8 != 0) {
    u64PayloadCanFrameNum++;
  }

  struct can_frame stCanHeader;
  stCanHeader.can_id = DEFAULT_CAN_ID;
  stCanHeader.can_dlc = DEFAULT_CAN_DATA_SIZE;

  char* pcData = (char*)stCanHeader.data;
  uint16_t usDataLen = DEFAULT_CAN_DATA_SIZE;
  iEncode64(u64PayloadCanFrameNum, &pcData, &usDataLen);

  int ret = write(s, &stCanHeader, sizeof(stCanHeader));
  if (ret == -1) {
    printf("write header to can failed, err = %s\n", strerror(errno));
    return -1;
  }
  printf("write header to can success, u64PayloadCanFrameNum = %lld\n",
         u64PayloadCanFrameNum);

  // send pay load
  const char* pcBufOffset = pcBuf;
  int iLeftLen = iLen;
  struct can_frame stCanSendFrame;
  stCanSendFrame.can_id = DEFAULT_CAN_ID;

  while (iLeftLen > 0) {
    if (iLeftLen >= DEFAULT_CAN_DATA_SIZE) {
      stCanSendFrame.can_dlc = DEFAULT_CAN_DATA_SIZE;
      memcpy(stCanSendFrame.data, pcBufOffset, DEFAULT_CAN_DATA_SIZE);
      pcBufOffset += DEFAULT_CAN_DATA_SIZE;
      iLeftLen -= DEFAULT_CAN_DATA_SIZE;
    } else {
      stCanSendFrame.can_dlc = iLeftLen;
      memcpy(stCanSendFrame.data, pcBufOffset, iLeftLen);
      pcBufOffset += iLeftLen;
      iLeftLen = 0;
    }
    int ret = write(s, &stCanSendFrame, sizeof(stCanSendFrame));
    if (ret == -1) {
      printf("write to can failed, err = %s\n", strerror(errno));
      return -1;
    }
    printf("write to can success\n");
  }
  return iLen - iLeftLen;
}

int main() {
  printf("Hello World\n");

  int run_as_daemon = 0;
  int ret = 0;
  static int verbose;

  int s = createCanFd();
  if (s < 0) {
    printf("create socket can failed");
    exit(EXIT_FAILURE);
  }

  // tun/tap
  int t;
  if ((t = open("/dev/net/tun", O_RDWR)) < 0) {
    printf("open tunfd failed, errno = %d\n", errno);
    close(s);
    close(t);
    exit(EXIT_FAILURE);
  }

  struct ifreq ifr;
  static char name[sizeof(ifr.ifr_name)] = DEFAULT_NAME;
  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
  /* string termination is ensured at commandline option handling */
  strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

  if (ioctl(t, TUNSETIFF, (void*)&ifr) < 0) {
    printf("ioctl tunfd");
    close(s);
    close(t);
    exit(EXIT_FAILURE);
  }

  /* Now the tun device exists. We can daemonize to let the
   * parent continue and use the network interface. */
  if (run_as_daemon) {
    if (daemon(0, 0)) {
      printf("failed to daemonize");
      exit(EXIT_FAILURE);
    }
  }

  signal(SIGTERM, sigterm);
  signal(SIGHUP, sigterm);
  signal(SIGINT, sigterm);

  fd_set rdfs;
  int nbytes;
  char ucCanBuffer[BUF_LEN];
  char ucTunBuffer[BUF_LEN];
  while (running) {
    FD_ZERO(&rdfs);
    FD_SET(s, &rdfs);
    FD_SET(t, &rdfs);

    if ((ret = select(t + 1, &rdfs, NULL, NULL, NULL)) < 0) {
      printf("select");
      continue;
    }

    // socket can --> tun/tap
    if (FD_ISSET(s, &rdfs)) {
      nbytes = iCanRead(s, ucCanBuffer, sizeof(ucCanBuffer));
      if (nbytes < 0) {
        printf("read raw can socket failed, err = %s\n", strerror(errno));
        return -1;
      }

      ret = write(t, ucCanBuffer, nbytes);
      if (verbose) {
        if (ret < 0 && errno == EAGAIN)
          printf(";");
        else
          printf(",");
        fflush(stdout);
      }
    }

    // tun/tap --> socket can
    if (FD_ISSET(t, &rdfs)) {
      nbytes = read(t, ucTunBuffer, BUF_LEN);
      if (nbytes < 0) {
        printf("read tunfd");
        return -1;
      }
      if (nbytes > BUF_LEN) {
        return -1;
      }

      ret = iCanWrite(s, (const char*)ucTunBuffer, nbytes);
      // ret = write(s, buffer, nbytes);
      if (verbose) {
        if (ret < 0 && errno == EAGAIN)
          printf(":");
        else
          printf(".");
        fflush(stdout);
      }
    }
  }

  close(s);
  close(t);
  return EXIT_SUCCESS;
}
