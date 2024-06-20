// // #include <stdio.h>
// // #include <stdlib.h>
// // #include <unistd.h>
// // #include <arpa/inet.h>
// // #include <pthread.h>
// // #include <signal.h>

// // #define NUM_SOCKETS 2
// // #define READ_ONCE 1

// // typedef char sample_t;

// // volatile int mute = 0;  // ミュートフラグ

// // #define CMD_REC "rec -t raw -b 16 -c 1 -e s -r 44100 -"
// // #define CMD_PLAY "play -t raw -b 16 -c 1 -e s -r 44100 -"

// // void die(const char *s);
// // void create_connection_host(const int port_num, int *hss, int hcs_arr[]);
// // void create_connection_client(const char *ip_address, const int port_num, int *css);
// // void *func_rec(void *sArg);
// // void *func_play(void *sArg);
// // void func_rec_play(int *s_rec, int *s_play);
// // void *monitor_input(void *arg);  // ユーザー入力を監視する関数

// // // ================ メイン関数 =================
// // int main(int argc, char **argv) {
// //     int isHost;  // 1: host, 0: client
// //     int port_num;
// //     char *ip_address;

// //     if (argc == 2) {
// //         isHost = 1;
// //         port_num = atoi(argv[1]);
// //     } else if (argc == 3) {
// //         isHost = 0;
// //         ip_address = argv[1];
// //         port_num = atoi(argv[2]);
// //     } else {
// //         fprintf(stderr, "Usage: %s [<IPv4 Address (if you are a client)>] <(int) Port No.>\n", argv[0]);
// //         exit(EXIT_FAILURE);
// //     }

// //     // Host
// //     int hss, hcs_rec, hcs_play;
// //     // Client
// //     int css_rec, css_play;

// //     if (isHost) {
// //         int hcs_arr[NUM_SOCKETS];
// //         create_connection_host(port_num, &hss, hcs_arr);
// //         hcs_rec = hcs_arr[0];
// //         hcs_play = hcs_arr[1];
// //         func_rec_play(&hcs_rec, &hcs_play);

// //         close(hcs_rec);
// //         close(hcs_play);
// //         close(hss);
// //     } else {
// //         create_connection_client(ip_address, port_num, &css_play);
// //         create_connection_client(ip_address, port_num, &css_rec);
// //         func_rec_play(&css_rec, &css_play);

// //         close(css_rec);
// //         close(css_play);
// //     }

// //     return EXIT_SUCCESS;
// // }

// // // エラー処理
// // void die(const char *s) {
// //     perror(s);
// //     exit(EXIT_FAILURE);
// // }

// // // ================ ソケット接続作成関数（ホスト）================
// // void create_connection_host(const int port_num, int *hss, int hcs_arr[]) {
// //     struct sockaddr_in server_addr;
// //     struct sockaddr_in client_addr;

// //     *hss = socket(PF_INET, SOCK_STREAM, 0);
// //     if (*hss < 0) {
// //         die("HSS creation failed");
// //     }

// //     server_addr.sin_family = AF_INET;
// //     server_addr.sin_port = htons(port_num);
// //     server_addr.sin_addr.s_addr = INADDR_ANY;

// //     if (bind(*hss, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
// //         die("bind");
// //     }

// //     if (listen(*hss, 10) < 0) {
// //         die("listen");
// //     }

// //     printf("Waiting for client...\n");

// //     int i = 0;
// //     while (i < NUM_SOCKETS) {
// //         socklen_t len = sizeof(struct sockaddr_in);
// //         hcs_arr[i] = accept(*hss, (struct sockaddr *)&client_addr, &len);
// //         if (hcs_arr[i] < 0) {
// //             die("accept");
// //         }
// //         printf("Accepted connection from %s, port=%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
// //         i++;
// //     }
// // }

// // // ================ ソケット接続作成関数（クライアント）================
// // void create_connection_client(const char *ip_address, const int port_num, int *css) {
// //     *css = socket(PF_INET, SOCK_STREAM, 0);
// //     if (*css < 0) {
// //         die("CSS creation failed");
// //     }

// //     struct sockaddr_in server_addr;
// //     server_addr.sin_family = AF_INET;
// //     server_addr.sin_port = htons(port_num);

// //     if (inet_aton(ip_address, &server_addr.sin_addr) == 0) {
// //         die("Invalid IP address");
// //     }

// //     if (connect(*css, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
// //         die("Connection failed");
// //     }
// // }

// // // ================ 録音処理関数 =================
// // void *func_rec(void *sArg) {
// //     int *s = (int *)sArg; //型キャストと代入
// //     FILE *fp_rec = popen(CMD_REC, "r");
// //     if (fp_rec == NULL) {
// //         die("popen");
// //     }

// //     int fd_rec = fileno(fp_rec); //ファイルディスクリプタを取得
// //     int read_byte_rec;
// //     sample_t data_rec[READ_ONCE];

// //     while ((read_byte_rec = read(fd_rec, data_rec, sizeof(sample_t) * READ_ONCE)) > 0) {
// //         if (!mute && send(*s, data_rec, read_byte_rec, 0) < 0) {
// //             die("send");
// //         }
// //     }

// //     pclose(fp_rec);
// //     return 0;
// // }

// // // ================ 再生処理関数 =================
// // void *func_play(void *sArg) {
// //     int *s = (int *)sArg;
// //     FILE *fp_play = popen(CMD_PLAY, "w");
// //     if (fp_play == NULL) {
// //         die("popen");
// //     }

// //     int fd_play = fileno(fp_play);
// //     int read_byte_play;
// //     sample_t data_play[READ_ONCE];

// //     while ((read_byte_play = recv(*s, data_play, sizeof(sample_t) * READ_ONCE, 0)) > 0) {
// //         if (!mute && write(fd_play, data_play, read_byte_play) < 0) {
// //             die("write");
// //         }
// //     }

// //     pclose(fp_play);
// //     return 0;
// // }

// // // ================ 音声の送受信処理関数 =================
// // void func_rec_play(int *s_rec, int *s_play) {
// //     pthread_t pth_rec, pth_play, pth_input;

// //     if (pthread_create(&pth_rec, NULL, func_rec, s_rec)) {
// //         die("pthread_create func_rec");
// //     }
// //     if (pthread_create(&pth_play, NULL, func_play, s_play)) {
// //         die("pthread_create func_play");
// //     }
// //     if (pthread_create(&pth_input, NULL, monitor_input, NULL)) {
// //         die("pthread_create monitor_input");
// //     }

// //     if (pthread_join(pth_rec, NULL)) {
// //         die("pthread_join func_rec");
// //     }
// //     if (pthread_join(pth_play, NULL)) {
// //         die("pthread_join func_play");
// //     }
// //     if (pthread_join(pth_input, NULL)) {
// //         die("pthread_join monitor_input");
// //     }
// // }

// // // ================ ユーザー入力を監視する関数 =================
// // void *monitor_input(void *arg) {
// //     char input;
// //     while (1) {
// //         input = getchar();
// //         if (input == 'm') {
// //             mute = !mute;  // ミュート状態を切り替える
// //             printf("Mute is now %s\n", mute ? "ON" : "OFF");
// //         }
// //     }
// //     return NULL;
// // }

// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <pthread.h>
// #include <signal.h>

// #define CMD_REC "rec -t raw -b 16 -c 1 -e s -r 44100 - highpass 20 lowpass 20000 -"
// #define CMD_PLAY "play -t raw -b 16 -c 1 -e s -r 44100 - highpass 20 lowpass 20000 -"
// #define NUM_SOCKETS 2
// #define READ_ONCE 1

// typedef char sample_t;

// volatile int mute[NUM_SOCKETS] = {0};  // ユーザーごとのミュートフラグ

// void die(const char *s);
// void create_connection_host(const int port_num, int *hss, int hcs_arr[]);
// void create_connection_client(const char *ip_address, const int port_num, int *css);
// void *func_rec(void *sArg);
// void *func_play(void *sArg);
// void func_rec_play(int *s_rec, int *s_play);
// void *monitor_input(void *arg);  // ユーザー入力を監視する関数

// typedef struct {
//     int socket;
//     int user_id;
// } thread_args_t;

// // ================ メイン関数 =================
// int main(int argc, char **argv) {
//     int isHost;  // 1: host, 0: client
//     int port_num;
//     char *ip_address;

//     if (argc == 2) {
//         isHost = 1;
//         port_num = atoi(argv[1]);
//     } else if (argc == 3) {
//         isHost = 0;
//         ip_address = argv[1];
//         port_num = atoi(argv[2]);
//     } else {
//         fprintf(stderr, "Usage: %s [<IPv4 Address (if you are a client)>] <(int) Port No.>\n", argv[0]);
//         exit(EXIT_FAILURE);
//     }

//     // Host
//     int hss, hcs_rec, hcs_play;
//     // Client
//     int css_rec, css_play;

//     if (isHost) {
//         int hcs_arr[NUM_SOCKETS];
//         create_connection_host(port_num, &hss, hcs_arr);
//         hcs_rec = hcs_arr[0];
//         hcs_play = hcs_arr[1];
//         func_rec_play(&hcs_rec, &hcs_play);

//         close(hcs_rec);
//         close(hcs_play);
//         close(hss);
//     } else {
//         create_connection_client(ip_address, port_num, &css_play);
//         create_connection_client(ip_address, port_num, &css_rec);
//         func_rec_play(&css_rec, &css_play);

//         close(css_rec);
//         close(css_play);
//     }

//     return EXIT_SUCCESS;
// }

// // エラー処理
// void die(const char *s) {
//     perror(s);
//     exit(EXIT_FAILURE);
// }

// // ================ ソケット接続作成関数（ホスト）================
// void create_connection_host(const int port_num, int *hss, int hcs_arr[]) {
//     struct sockaddr_in server_addr;
//     struct sockaddr_in client_addr;

//     *hss = socket(PF_INET, SOCK_STREAM, 0);
//     if (*hss < 0) {
//         die("HSS creation failed");
//     }

//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(port_num);
//     server_addr.sin_addr.s_addr = INADDR_ANY;

//     if (bind(*hss, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
//         die("bind");
//     }

//     if (listen(*hss, 10) < 0) {
//         die("listen");
//     }

//     printf("Waiting for client...\n");

//     int i = 0;
//     while (i < NUM_SOCKETS) {
//         socklen_t len = sizeof(struct sockaddr_in);
//         hcs_arr[i] = accept(*hss, (struct sockaddr *)&client_addr, &len);
//         if (hcs_arr[i] < 0) {
//             die("accept");
//         }
//         printf("Accepted connection from %s, port=%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
//         i++;
//     }
// }

// // ================ ソケット接続作成関数（クライアント）================
// void create_connection_client(const char *ip_address, const int port_num, int *css) {
//     *css = socket(PF_INET, SOCK_STREAM, 0);
//     if (*css < 0) {
//         die("CSS creation failed");
//     }

//     struct sockaddr_in server_addr;
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(port_num);

//     if (inet_aton(ip_address, &server_addr.sin_addr) == 0) {
//         die("Invalid IP address");
//     }

//     if (connect(*css, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
//         die("Connection failed");
//     }
// }

// // ================ 録音処理関数 =================
// void *func_rec(void *sArg) {
//     thread_args_t *args = (thread_args_t *)sArg;
//     int s = args->socket;
//     int user_id = args->user_id;

//     int pipe_fds[2];
//     if (pipe(pipe_fds) == -1) {
//         die("pipe");
//     }

//     pid_t pid = fork();
//     if (pid == -1) {
//         die("fork");
//     }

//     if (pid == 0) { // 子プロセス
//         close(pipe_fds[0]); // 読み取り用パイプを閉じる
//         dup2(pipe_fds[1], STDOUT_FILENO); // 書き込み用パイプを標準出力に接続
//         close(pipe_fds[1]);
//         execlp("rec", "rec", "-t", "raw", "-b", "16", "-c", "1", "-e", "s", "-r", "44100", "-", NULL);
//         die("execlp");
//     } else { // 親プロセス
//         close(pipe_fds[1]); // 書き込み用パイプを閉じる
//         int fd_rec = pipe_fds[0];
//         int read_byte_rec;
//         sample_t data_rec[READ_ONCE];

//         while ((read_byte_rec = read(fd_rec, data_rec, sizeof(sample_t) * READ_ONCE)) > 0) {
//             if (!mute[user_id] && send(s, data_rec, read_byte_rec, 0) < 0) {
//                 die("send");
//             }
//         }

//         close(fd_rec);
//         wait(NULL); // 子プロセスの終了を待つ
//     }

//     return 0;
// }

// // ================ 再生処理関数 =================
// void *func_play(void *sArg) {
//     thread_args_t *args = (thread_args_t *)sArg;
//     int s = args->socket;
//     int user_id = args->user_id;

//     int pipe_fds[2];
//     if (pipe(pipe_fds) == -1) {
//         die("pipe");
//     }

//     pid_t pid = fork();
//     if (pid == -1) {
//         die("fork");
//     }

//     if (pid == 0) { // 子プロセス
//         close(pipe_fds[1]); // 書き込み用パイプを閉じる
//         dup2(pipe_fds[0], STDIN_FILENO); // 読み取り用パイプを標準入力に接続
//         close(pipe_fds[0]);
//         execlp("play", "play", "-t", "raw", "-b", "16", "-c", "1", "-e", "s", "-r", "44100", "-", NULL);
//         die("execlp");
//     } else { // 親プロセス
//         close(pipe_fds[0]); // 読み取り用パイプを閉じる
//         int fd_play = pipe_fds[1];
//         int read_byte_play;
//         sample_t data_play[READ_ONCE];

//         while ((read_byte_play = recv(s, data_play, sizeof(sample_t) * READ_ONCE, 0)) > 0) {
//             if (!mute[user_id] && write(fd_play, data_play, read_byte_play) < 0) {
//                 die("write");
//             }
//         }

//         close(fd_play);
//         wait(NULL); // 子プロセスの終了を待つ
//     }

//     return 0;
// }

// // ================ 音声の送受信処理関数 =================
// void func_rec_play(int *s_rec, int *s_play) {
//     pthread_t pth_rec, pth_play, pth_input_rec, pth_input_play;

//     thread_args_t args_rec = {*s_rec, 0};  // 第1ユーザー
//     thread_args_t args_play = {*s_play, 1};  // 第2ユーザー

//     if (pthread_create(&pth_rec, NULL, func_rec, &args_rec)) {
//         die("pthread_create func_rec");
//     }
//     if (pthread_create(&pth_play, NULL, func_play, &args_play)) {
//         die("pthread_create func_play");
//     }
//     if (pthread_create(&pth_input_rec, NULL, monitor_input, &args_rec)) {
//         die("pthread_create monitor_input_rec");
//     }
//     if (pthread_create(&pth_input_play, NULL, monitor_input, &args_play)) {
//         die("pthread_create monitor_input_play");
//     }

//     if (pthread_join(pth_rec, NULL)) {
//         die("pthread_join func_rec");
//     }
//     if (pthread_join(pth_play, NULL)) {
//         die("pthread_join func_play");
//     }
//     if (pthread_join(pth_input_rec, NULL)) {
//         die("pthread_join monitor_input_rec");
//     }
//     if (pthread_join(pth_input_play, NULL)) {
//         die("pthread_join monitor_input_play");
//     }
// }

// // ================ ユーザー入力を監視する関数 =================
// void *monitor_input(void *arg) {
//     thread_args_t *args = (thread_args_t *)arg;
//     int user_id = args->user_id;
//     char input;
//     while (1) {
//         printf("User %d: Enter 'm' to mute/unmute: ", user_id);
//         scanf(" %c", &input);
//         if (input == 'm') {
//             mute[user_id] = !mute[user_id];  // ミュート状態を切り替える
//             printf("Mute for user %d is now %s\n", user_id, mute[user_id] ? "ON" : "OFF");
//         }
//     }
//     return NULL;
// }

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

volatile int mute[NUM_SOCKETS] = {0}; // ユーザーごとのミュートフラグ

void die(const char *s)
{
  perror(s);
  exit(EXIT_FAILURE);
}

void create_connection_host(const int port_num, int *hss, int hcs_arr[])
{
  struct sockaddr_in server_addr, client_addr;

  *hss = socket(PF_INET, SOCK_STREAM, 0);
  if (*hss < 0)
    die("HSS creation failed");

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port_num);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(*hss, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    die("bind");
  if (listen(*hss, 10) < 0)
    die("listen");

  printf("Waiting for client...\n");

  int i = 0;
  while (i < NUM_SOCKETS)
  {
    socklen_t len = sizeof(client_addr);
    hcs_arr[i] = accept(*hss, (struct sockaddr *)&client_addr, &len);
    if (hcs_arr[i] < 0)
      die("accept");
    printf("Accepted connection from %s, port=%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    i++;
  }
}

void create_connection_client(const char *ip_address, const int port_num, int *css)
{
  *css = socket(PF_INET, SOCK_STREAM, 0);
  if (*css < 0)
    die("CSS creation failed");

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port_num);

  if (inet_aton(ip_address, &server_addr.sin_addr) == 0)
    die("Invalid IP address");
  if (connect(*css, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    die("Connection failed");
}

typedef struct
{
  int socket;
  int user_id;
} thread_args_t;

void *func_rec(void *sArg)
{
  thread_args_t *args = (thread_args_t *)sArg;
  int s = args->socket;
  int user_id = args->user_id;

  FILE *fp_rec = popen(CMD_REC, "r");
  if (!fp_rec)
    die("popen");

  int fd_rec = fileno(fp_rec);
  int read_byte_rec;
  sample_t data_rec[READ_ONCE];

  while ((read_byte_rec = read(fd_rec, data_rec, sizeof(sample_t) * READ_ONCE)) > 0)
  {
    if (!mute[user_id] && send(s, data_rec, read_byte_rec, 0) < 0)
      die("send");
  }

  pclose(fp_rec);
  return NULL;
}

void *func_play(void *sArg)
{
  thread_args_t *args = (thread_args_t *)sArg;
  int s = args->socket;
  int user_id = args->user_id;

  FILE *fp_play = popen(CMD_PLAY, "w");
  if (!fp_play)
    die("popen");

  int fd_play = fileno(fp_play);
  int read_byte_play;
  sample_t data_play[READ_ONCE];

  while ((read_byte_play = recv(s, data_play, sizeof(sample_t) * READ_ONCE, 0)) > 0)
  {
    if (!mute[user_id] && write(fd_play, data_play, read_byte_play) < 0)
      die("write");
  }

  pclose(fp_play);
  return NULL;
}

void *monitor_input(void *arg)
{
  thread_args_t *args = (thread_args_t *)arg;
  int user_id = args->user_id;
  char input;

  while (1)
  {
    printf("User %d: Enter 'm' to mute/unmute: ", user_id);
    scanf(" %c", &input);
    if (input == 'm')
    {
      mute[user_id] = !mute[user_id];
      printf("Mute for user %d is now %s\n", user_id, mute[user_id] ? "ON" : "OFF");
    }
  }

  return NULL;
}

void func_rec_play(int *s_rec, int *s_play)
{
  pthread_t pth_rec, pth_play, pth_input_rec, pth_input_play;

  thread_args_t args_rec = {*s_rec, 0};
  thread_args_t args_play = {*s_play, 1};

  if (pthread_create(&pth_rec, NULL, func_rec, &args_rec))
    die("pthread_create func_rec");
  if (pthread_create(&pth_play, NULL, func_play, &args_play))
    die("pthread_create func_play");
  if (pthread_create(&pth_input_rec, NULL, monitor_input, &args_rec))
    die("pthread_create monitor_input_rec");
  if (pthread_create(&pth_input_play, NULL, monitor_input, &args_play))
    die("pthread_create monitor_input_play");

  if (pthread_join(pth_rec, NULL))
    die("pthread_join func_rec");
  if (pthread_join(pth_play, NULL))
    die("pthread_join func_play");
  if (pthread_join(pth_input_rec, NULL))
    die("pthread_join monitor_input_rec");
  if (pthread_join(pth_input_play, NULL))
    die("pthread_join monitor_input_play");
}

int main(int argc, char **argv)
{
  int isHost;
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

  int hss, hcs_rec, hcs_play;
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
