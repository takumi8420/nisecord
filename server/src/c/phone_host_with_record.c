// host側の実行プログラム
// gcc -o recording_host_kansei recording_host_kansei.c -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define CMD_REC "rec -t raw -b 16 -c 1 -e s -r 44100 -"
#define CMD_PLAY "play -t raw -b 16 -c 1 -e s -r 44100 -"
#define READ_ONCE 1024

typedef char sample_t;

// 関数のプロトタイプ宣言
void die(const char *s);
void *func_rec(void *sArg);
void *func_play(void *sArg);

void die(const char *s)
{
  perror(s);
  exit(EXIT_FAILURE);
}

void *func_rec(void *sArg)
{
  int *s = (int *)sArg;
  FILE *fp_rec = popen(CMD_REC, "r");
  if (fp_rec == NULL)
  {
    die("popen rec");
  }

  int read_rec;
  sample_t buf[READ_ONCE];

  while ((read_rec = read(fileno(fp_rec), buf, sizeof(sample_t) * READ_ONCE)) > 0)
  {
    if (read_rec == 0 || read_rec == -1)
    {
      printf("Connection closed1.\n");
      break;
    }

    if (write(*s, buf, read_rec) < 0)
    {
      printf("2222222.\n");
      printf("Connection closed2.\n");
      printf("read_rec: %d\n", read_rec);
      perror("write to socket");
      break;
    }
    // if (send(*s, buf, read_rec, 0) < 0){
    //     printf("Connection closed.\n");
    //     perror("send to socket");
    //     break;
    // }
  }

  pclose(fp_rec);
  return 0;
}

void *func_play(void *sArg)
{
  int *s = (int *)sArg;
  FILE *fp_play = popen(CMD_PLAY, "w");
  if (fp_play == NULL)
  {
    die("popen play");
  }

  int read_play;
  sample_t buf[READ_ONCE];

  while ((read_play = read(*s, buf, sizeof(sample_t) * READ_ONCE)) > 0)
  {
    if (read_play == 0 || read_play == -1)
    {
      printf("Connection closed3.\n");
      break;
    }

    if (write(fileno(fp_play), buf, read_play) < 0)
    {
      printf("Connection closed4.\n");
      perror("write to pipe");
      break;
    }
  }

  pclose(fp_play);
  return 0;
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    fprintf(stderr, "Usage: %s <Port No.>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  int port = atoi(argv[1]);

  int hss;
  if ((hss = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    die("socket");
  }

  // ここ追記
  int opt = 1;
  if (setsockopt(hss, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
  {
    die("setsockopt");
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(hss, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    close(hss);
    die("bind");
  }

  if (listen(hss, 5) < 0)
  {
    close(hss);
    die("listen");
  }

  printf("Waiting for connection...\n");

  struct sockaddr_in client_addr;
  socklen_t len = sizeof(struct sockaddr_in);
  int hsc_rec, hsc_play;
  if ((hsc_rec = accept(hss, (struct sockaddr *)&client_addr, &len)) < 0)
  {
    close(hss);
    die("accept");
  }
  if ((hsc_play = accept(hss, (struct sockaddr *)&client_addr, &len)) < 0)
  {
    close(hsc_rec);
    close(hss);
    die("accept");
  }

  printf("Connected %s\n", inet_ntoa(client_addr.sin_addr));

  pthread_t rec_thread, play_thread;
  if (pthread_create(&rec_thread, NULL, func_rec, (void *)&hsc_rec) != 0)
  {
    close(hsc_rec);
    close(hsc_play);
    close(hss);
    die("rec_pthread_create");
  }
  if (pthread_create(&play_thread, NULL, func_play, (void *)&hsc_play) != 0)
  {
    pthread_cancel(rec_thread);
    close(hsc_rec);
    close(hsc_play);
    close(hss);
    die("play_pthread_create");
  }

  if (pthread_join(rec_thread, NULL) != 0)
  {
    die("rec_pthread_join");
  }
  if (pthread_join(play_thread, NULL) != 0)
  {
    die("play_pthread_join");
  }

  close(hsc_rec);
  close(hsc_play);
  close(hss);

  return 0;
}
