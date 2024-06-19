#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define CMD_REC "rec -t raw -b 16 -c 1 -e s -r 44100 -"
#define CMD_PLAY "play -t raw -b 16 -c 1 -e s -r 44100 -"
#define NUM_SOCKETS 2
#define READ_ONCE 1

typedef char sample_t;

void die(const char *s);
void create_connection_host(const int port_num, int *hss, int hcs_arr[]);
void create_connection_client(const char *ip_address, const int port_num, int *css);
void *func_rec(void *sArg);
void *func_play(void *sArg);
void func_rec_play(int *s_rec, int *s_play);

// ================ メイン関数 =================
// コマンドライン引数を解析し、ホストかクライアントかを判定。
// ホストの場合、リッスン用ソケットと受信用・送信用のソケットを作成。
// クライアントの場合、ホストに接続するための受信用・送信用のソケットを作成。
// それぞれのソケットを使用し音声の送受信を行う。
int main(int argc, char **argv)
{
  int isHost; // 1: host, 0: client
  int port_num;
  char *ip_address;

  if (argc == 2)
  {
    isHost = 1;
    port_num = atoi(argv[1]);
  }
  else if (argc == 3)
  {
    isHost = 0;
    ip_address = argv[1];
    port_num = atoi(argv[2]);
  }
  else
  {
    fprintf(stderr, "Usage: %s [<IPv4 Address (if you are a client)>] <(int) Port No.>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // Host
  int hss, hcs_rec, hcs_play;
  // Client
  int css_rec, css_play;

  if (isHost)
  {
    int hcs_arr[NUM_SOCKETS];
    create_connection_host(port_num, &hss, hcs_arr);
    hcs_rec = hcs_arr[0];
    hcs_play = hcs_arr[1];
    func_rec_play(&hcs_rec, &hcs_play);

    close(hcs_rec);
    close(hcs_play);
    close(hss);
  }
  else
  {
    create_connection_client(ip_address, port_num, &css_play);
    create_connection_client(ip_address, port_num, &css_rec);
    func_rec_play(&css_rec, &css_play);

    close(css_rec);
    close(css_play);
  }

  return EXIT_SUCCESS;
}

// エラー処理
void die(const char *s)
{
  perror(s);
  exit(EXIT_FAILURE);
}

// ================ ソケット接続作成関数（ホスト）================
// 指定されたポート番号でソケットを作成し、接続待ち状態にしてクライアントからの接続を受け入れる。
// 送受信のために2つのソケットを作成する。
// HSS: host server socket
// HCS: host client socket
void create_connection_host(const int port_num, int *hss, int hcs_arr[])
{
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;

  *hss = socket(PF_INET, SOCK_STREAM, 0);
  if (*hss < 0)
  {
    die("HSS creation failed");
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port_num);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(*hss, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    die("bind");
  }

  if (listen(*hss, 10) < 0)
  {
    die("listen");
  }

  printf("Waiting for client...\n");

  int i = 0;
  while (i < NUM_SOCKETS)
  {
    socklen_t len = sizeof(struct sockaddr_in);
    hcs_arr[i] = accept(*hss, (struct sockaddr *)&client_addr, &len);
    if (hcs_arr[i] < 0)
    {
      die("accept");
    }
    printf("Accepted connection from %s, port=%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    i++;
  }
}

// ================ ソケット接続作成関数（クライアント）================
// 指定されたIPアドレスとポート番号に対してソケットを作成し、ホストに接続
// CSS: client server socket
void create_connection_client(const char *ip_address, const int port_num, int *css)
{
  *css = socket(PF_INET, SOCK_STREAM, 0);
  if (*css < 0)
  {
    die("CSS creation failed");
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port_num);

  if (inet_aton(ip_address, &server_addr.sin_addr) == 0)
  {
    die("Invalid IP address");
  }

  if (connect(*css, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    die("Connection failed");
  }
}

// ================ 録音処理関数 =================
// 録音した音声データを読み取り、それをソケットを通じて送信
void *func_rec(void *sArg)
{
  int *s = (int *)sArg; // 型キャストと代入
  FILE *fp_rec = popen(CMD_REC, "r");
  if (fp_rec == NULL)
  {
    die("popen");
  }

  int fd_rec = fileno(fp_rec); // ファイルディスクリプタを取得
  int read_byte_rec;
  sample_t data_rec[READ_ONCE];

  while ((read_byte_rec = read(fd_rec, data_rec, sizeof(sample_t) * READ_ONCE)) > 0)
  {
    if (send(*s, data_rec, read_byte_rec, 0) < 0)
    {
      die("send");
    }
  }

  pclose(fp_rec);
  return 0;
}

// ================ 再生処理関数 =================
// ソケットを通じて受信した音声データを再生
void *func_play(void *sArg)
{
  int *s = (int *)sArg;
  FILE *fp_play = popen(CMD_PLAY, "w");
  if (fp_play == NULL)
  {
    die("popen");
  }

  int fd_play = fileno(fp_play);
  int read_byte_play;
  sample_t data_play[READ_ONCE];

  while ((read_byte_play = recv(*s, data_play, sizeof(sample_t) * READ_ONCE, 0)) > 0)
  {
    if (write(fd_play, data_play, read_byte_play) < 0)
    {
      die("write");
    }
  }

  pclose(fp_play);
  return 0;
}

// ================ 音声の送受信処理関数 =================
// 録音と再生をそれぞれ別のスレッドで処理するための関数
void func_rec_play(int *s_rec, int *s_play)
{
  pthread_t pth_rec, pth_play;

  if (pthread_create(&pth_rec, NULL, func_rec, s_rec))
  {
    die("pthread_create func_rec");
  }
  if (pthread_create(&pth_play, NULL, func_play, s_play))
  {
    die("pthread_create func_play");
  }

  if (pthread_join(pth_rec, NULL))
  {
    die("pthread_join func_rec");
  }
  if (pthread_join(pth_play, NULL))
  {
    die("pthread_join func_play");
  }
}
