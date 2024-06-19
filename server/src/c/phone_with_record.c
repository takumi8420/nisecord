#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

// play -t raw -b 16 -c 1 -e s -r 44100 recorded_audio_client_20240618161242.raw

#define CMD_REC "rec -t raw -b 16 -c 1 -e s -r 44100 -"
#define CMD_PLAY "play -t raw -b 16 -c 1 -e s -r 44100 -"
#define READ_ONCE 1024

typedef char sample_t;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void die(const char *s)
{
  perror(s);
  exit(EXIT_FAILURE);
}

void get_file_name(char *filename, size_t size, char option)
{
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  if (tm == NULL)
  {
    die("localtime");
  }

  // Ensure the directory exists
  if (mkdir("./recordings", 0777) && errno != EEXIST)
  {
    die("mkdir");
  }

  if (option == 'c')
  {
    int ret = snprintf(filename, size,
                       "./recordings/audio_%d%02d%02d%02d%02d%02d_client.raw",
                       tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                       tm->tm_hour, tm->tm_min, tm->tm_sec);
    if (ret < 0 || ret >= size)
    {
      die("snprintf");
    }
  }
  else if (option == 's')
  {
    int ret = snprintf(filename, size,
                       "./recordings/audio_%d%02d%02d%02d%02d%02d_server.raw",
                       tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                       tm->tm_hour, tm->tm_min, tm->tm_sec);
    if (ret < 0 || ret >= size)
    {
      die("snprintf");
    }
  }
  else
  {
    die("get_file_name");
  }
}

void *func_rec_client(void *sArg)
{
  int *s = (int *)sArg;

  char filename[128];
  get_file_name(filename, sizeof(filename), 'c');

  FILE *fp_out = fopen(filename, "wb");
  if (fp_out == NULL)
  {
    die("fopen");
  }

  FILE *fp_rec = popen(CMD_REC, "r");
  if (fp_rec == NULL)
  {
    fclose(fp_out);
    die("popen rec");
  }

  int read_rec;
  sample_t buf[READ_ONCE];

  while ((read_rec = fread(buf, sizeof(sample_t), READ_ONCE, fp_rec)) > 0)
  {
    // ローカルファイルに書き込む
    if (fwrite(buf, sizeof(sample_t), read_rec, fp_out) < read_rec)
    {
      perror("write to file");
      break;
    }

    pthread_mutex_lock(&mutex); // ロック開始
    // サーバーに送信する
    if (write(*s, buf, read_rec * sizeof(sample_t)) < 0)
    {
      printf("Connection closed.\n");
      perror("write to socket");
      pthread_mutex_unlock(&mutex); // ロック解除
      break;
    }
    pthread_mutex_unlock(&mutex); // ロック解除
  }

  pclose(fp_rec);
  fclose(fp_out);
  return 0;
}

void *func_play_client(void *sArg)
{
  int *s = (int *)sArg;

  char filename[128];
  get_file_name(filename, sizeof(filename), 's');

  FILE *fp_out = fopen(filename, "wb");
  if (fp_out == NULL)
  {
    die("fopen");
  }

  FILE *fp_play = popen(CMD_PLAY, "w");
  if (fp_play == NULL)
  {
    fclose(fp_out);
    die("popen play");
  }

  int read_play;
  sample_t buf[READ_ONCE];

  while ((read_play = read(*s, buf, sizeof(sample_t) * READ_ONCE)) > 0)
  {
    // ローカルファイルに書き込む
    if (fwrite(buf, sizeof(sample_t), read_play, fp_out) < read_play)
    {
      perror("write to file");
      break;
    }

    pthread_mutex_lock(&mutex); // ロック開始
    if (write(fileno(fp_play), buf, read_play * sizeof(sample_t)) < 0)
    {
      printf("Connection closed.\n");
      perror("write to pipe");
      pthread_mutex_unlock(&mutex); // ロック解除
      break;
    }
    pthread_mutex_unlock(&mutex); // ロック解除
  }

  pclose(fp_play);
  fclose(fp_out);
  return 0;
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s <IP Address> <Port No.>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char *ip_address = argv[1];
  int port = atoi(argv[2]);

  int css_rec, css_play;
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip_address);

  if ((css_rec = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    die("socket");
  }
  if ((css_play = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    close(css_rec);
    die("socket");
  }
  // play と rec の順番を、サーバーと逆にしないといけない
  if (connect(css_play, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    close(css_rec);
    close(css_play);
    die("connect");
  }

  if (connect(css_rec, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    close(css_rec);
    close(css_play);
    die("connect");
  }

  // play と rec は同時に実行する
  pthread_t rec_thread, play_thread;
  if (pthread_create(&rec_thread, NULL, func_rec_client, (void *)&css_rec) != 0)
  {
    close(css_rec);
    close(css_play);
    die("rec_pthread_create");
  }
  if (pthread_create(&play_thread, NULL, func_play_client, (void *)&css_play) != 0)
  {
    pthread_cancel(rec_thread);
    close(css_rec);
    close(css_play);
    die("play_pthread_create");
  }

  // スレッドの終了を待つ
  if (pthread_join(rec_thread, NULL) != 0)
  {
    die("rec_pthread_join");
  }
  if (pthread_join(play_thread, NULL) != 0)
  {
    die("play_pthread_join");
  }

  close(css_rec);
  close(css_play);

  pthread_mutex_destroy(&mutex); // ミューテックスの破棄

  return 0;
}
