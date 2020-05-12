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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <syslog.h>
#include <termios.h>
#include <unistd.h>

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

int iCanWrite(int s, const char* pcBuf, int iLen) {
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
  unsigned char buffer[BUF_LEN];
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
      struct can_frame frame;
      nbytes = read(s, &frame, sizeof(frame));
      if (nbytes < 0) {
        printf("read raw can socket failed, err = %s\n", strerror(errno));
        return -1;
      }

      ret = write(t, frame.data, frame.can_dlc);
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
      nbytes = read(t, buffer, BUF_LEN);
      if (nbytes < 0) {
        printf("read tunfd");
        return -1;
      }
      if (nbytes > BUF_LEN) {
        return -1;
      }

      ret = iCanWrite(s, (const char*)buffer, nbytes);
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
