#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* シリアルインターフェースに対応するデバイスファイル */
#define SERIAL_PORT "/dev/ttyUSB0"

int main()
{

  /* シリアルインターフェースのオープン */
  /* O_RDWR: 入出力用にオープン */
  /* O_NOCTTY: ノイズ等による不意のctrl-cを防ぐため，tty制御なし */
  /* シリアルインターフェースをint型変数"fd"の名前で扱えるように */
  int fd;
  fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY);
  if (fd < 0)
  {
    printf("%s doesn't open it\n", SERIAL_PORT);
    return -1;
  }

  /* シリアルポートの設定を行う変数を宣言 */
  struct termios oldtio, newtio;
  /* 現在の設定を oldtio に保存 */
  tcgetattr(fd, &oldtio);
  /* 今回使用する設定 newtio に現在の設定 oldtio をコピー */
  newtio = oldtio;
  cfsetspeed(&newtio, B9600); /* 入出力スピード設定 */

  /* 27行目-42行目までの設定を有効にする */
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &newtio); /* 設定を有効に */

  /* ここからがユーザ固有の処理:例として文字列 hello を送信(write関数)．受信はread関*/
  unsigned char buf[11];
  while (1)
  {
    // strcpy(buf, "hello");
    // write(fd, buf, sizeof(buf)); /* write関数:送信処理 */
    read(fd, buf, sizeof(buf));
    if (buf[0] == 0x55)
    {
      if (buf[1] == 0x53)
      {
        printf("|%x|%x|%x|%x|%x|%x|%x|%x|%x|%x|%x|\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10]);
      }
    }
  }
  /* デバイスの設定を戻す */
  tcsetattr(fd, TCSANOW, &oldtio);
  close(fd);
  return 0;
}
