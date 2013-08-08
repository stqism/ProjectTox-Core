/*
 * Toxic -- Tox Curses Client
 */

#include <curses.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef _win32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "../../core/Messenger.h"
#include "../../core/network.h"

#include "configdir.h"
#include "windows.h"

extern ToxWindow new_prompt();
extern ToxWindow new_friendlist();

extern int friendlist_onFriendAdded(int num);
extern void disable_chatwin(int f_num);
extern int add_req(uint8_t *public_key); // XXX

/* Holds status of chat windows */
char WINDOW_STATUS[MAX_WINDOW_SLOTS];

#ifndef TOXICVER
#define TOXICVER "NOVER" //Use the -D flag to set this
#endif

static ToxWindow windows[MAX_WINDOW_SLOTS];
static ToxWindow* prompt;

int w_num;
int active_window;

/* CALLBACKS START */
void on_request(uint8_t *public_key, uint8_t *data, uint16_t length)
{
  int n = add_req(public_key);
  wprintw(prompt->window, "\nFriend request from:\n");

  int i;
  for (i = 0; i < KEY_SIZE_BYTES; ++i) {
    wprintw(prompt->window, "%02x", public_key[i] & 0xff);
  }

  wprintw(prompt->window, "\nWith the message: %s\n", data);
  wprintw(prompt->window, "\nUse \"accept %d\" to accept it.\n", n);

  for (i = 0; i < MAX_WINDOW_SLOTS; ++i) {
    if (windows[i].onFriendRequest != NULL)
      windows[i].onFriendRequest(&windows[i], public_key, data, length);
  }
}

void on_message(int friendnumber, uint8_t *string, uint16_t length)
{
  int i;
  for (i = 0; i < MAX_WINDOW_SLOTS; ++i) {
    if (windows[i].onMessage != NULL)
      windows[i].onMessage(&windows[i], friendnumber, string, length);
  }
}

void on_action(int friendnumber, uint8_t *string, uint16_t length)
{
  int i;
  for (i = 0; i < MAX_WINDOW_SLOTS; ++i) {
    if (windows[i].onAction != NULL)
      windows[i].onAction(&windows[i], friendnumber, string, length);
  }
}

void on_nickchange(int friendnumber, uint8_t *string, uint16_t length)
{
  wprintw(prompt->window, "\n(nickchange) %d: %s!\n", friendnumber, string);
  int i;
  for (i = 0; i < MAX_WINDOW_SLOTS; ++i) {
    if (windows[i].onNickChange != NULL)
      windows[i].onNickChange(&windows[i], friendnumber, string, length);
  }
}

void on_statuschange(int friendnumber, uint8_t *string, uint16_t length)
{
  wprintw(prompt->window, "\n(statuschange) %d: %s\n", friendnumber, string);
  int i;
  for (i=0; i<MAX_WINDOW_SLOTS; ++i) {
    if (windows[i].onStatusChange != NULL)
      windows[i].onStatusChange(&windows[i], friendnumber, string, length);
  }
}

void on_friendadded(int friendnumber)
{
  friendlist_onFriendAdded(friendnumber);
}
/* CALLBACKS END */

static void init_term()
{
  /* Setup terminal */
  initscr();
  cbreak();
  keypad(stdscr, 1);
  noecho();
  timeout(100);

  if (has_colors()) {
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
  }
  refresh();
}

static void init_tox()
{
  /* Init core */
  initMessenger();

  /* Callbacks */
  m_callback_friendrequest(on_request);
  m_callback_friendmessage(on_message);
  m_callback_namechange(on_nickchange);
  m_callback_statusmessage(on_statuschange);
  m_callback_action(on_action);
}

void init_window_status()
{
  /* Default window values decrement from -2 */
  int i;
  for (i = 0; i < N_DEFAULT_WINS; ++i)
    WINDOW_STATUS[i] = -(i+2);

  int j;
  for (j = N_DEFAULT_WINS; j < MAX_WINDOW_SLOTS; j++)
    WINDOW_STATUS[j] = -1;
}

int add_window(ToxWindow w, int n)
{
  if (w_num >= TOXWINDOWS_MAX_NUM)
    return -1;

  if (LINES < 2)
    return -1;

  w.window = newwin(LINES - 2, COLS, 0, 0);
  if (w.window == NULL)
    return -1;

  windows[n] = w;
  w.onInit(&w);
  w_num++;
  return n;
}

/* Deletes window w and cleans up */
void del_window(ToxWindow *w, int f_num)
{
  delwin(w->window);
  int i;
  for (i = N_DEFAULT_WINS; i < MAX_WINDOW_SLOTS; ++i) {
    if (WINDOW_STATUS[i] == f_num) {
      WINDOW_STATUS[i] = -1;
      disable_chatwin(f_num);
      break;
    }
  }
  clear();
  refresh();
}

static void init_windows()
{
  w_num = 0;
  int n_prompt = 0;
  int n_friendslist = 1;
  if (add_window(new_prompt(), n_prompt) == -1 
                        || add_window(new_friendlist(), n_friendslist) == -1) {
    fprintf(stderr, "add_window() failed.\n");
    endwin();
    exit(1);
  }
  prompt = &windows[n_prompt];
}

static void do_tox()
{
  static bool dht_on = false;
  if (!dht_on && DHT_isconnected()) {
    dht_on = true;
    wprintw(prompt->window, "\nDHT connected!\n");
  }
  else if (dht_on && !DHT_isconnected()) {
    dht_on = false;
    wprintw(prompt->window, "\nDHT disconnected!\n");
  }
  doMessenger();
}

static void load_data(char *path)
{
  FILE *fd;
  size_t len;
  uint8_t *buf;

  if ((fd = fopen(path, "r")) != NULL) {
    fseek(fd, 0, SEEK_END);
    len = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    buf = malloc(len);
    if (buf == NULL) {
      fprintf(stderr, "malloc() failed.\n");
      fclose(fd);
      endwin();
      exit(1);
    }
    if (fread(buf, len, 1, fd) != 1){
      fprintf(stderr, "fread() failed.\n");
      free(buf);
      fclose(fd);
      endwin();
      exit(1);
    }
    Messenger_load(buf, len);
  }
  else {
    len = Messenger_size();
    buf = malloc(len);
    if (buf == NULL) {
      fprintf(stderr, "malloc() failed.\n");
      endwin();
      exit(1);
    }
    Messenger_save(buf);

    fd = fopen(path, "w");
    if (fd == NULL) {
      fprintf(stderr, "fopen() failed.\n");
      free(buf);
      endwin();
      exit(1);
    }

    if (fwrite(buf, len, 1, fd) != 1){
      fprintf(stderr, "fwrite() failed.\n");
      free(buf);
      fclose(fd);
      endwin();
      exit(1);
    }
  }
  free(buf);
  fclose(fd);
}

static void draw_bar()
{
  static int odd = 0;
  int blinkrate = 30;

  attron(COLOR_PAIR(4));
  mvhline(LINES - 2, 0, '_', COLS);
  attroff(COLOR_PAIR(4));

  move(LINES - 1, 0);

  attron(COLOR_PAIR(4) | A_BOLD);
  printw(" TOXIC " TOXICVER " |"); 
  attroff(COLOR_PAIR(4) | A_BOLD);

  int i;
  for (i = 0; i < (MAX_WINDOW_SLOTS); ++i) {
    if (WINDOW_STATUS[i] != -1) {
      if (i == active_window)
        attron(A_BOLD);

      odd = (odd+1) % blinkrate;
      if (windows[i].blink && (odd < (blinkrate/2)))
        attron(COLOR_PAIR(3));

      printw(" %s", windows[i].title);
      if (windows[i].blink && (odd < (blinkrate/2)))
        attroff(COLOR_PAIR(3));

      if (i == active_window) {
        attroff(A_BOLD);
      }
    }
  }
  refresh();
}

void prepare_window(WINDOW *w)
{
  mvwin(w, 0, 0);
  wresize(w, LINES-2, COLS);
}

/* Shows next window when tab or back-tab is pressed */
void set_active_window(int ch)
{
  int f_inf = 0;
  int max = MAX_WINDOW_SLOTS-1;
  if (ch == '\t') {
    int i = (active_window + 1) % max;
    while (true) {
      if (WINDOW_STATUS[i] != -1) {
        active_window = i;
        return;
      }
      i = (i  + 1) % max;
      if (f_inf++ > max) {    // infinite loop check
        endwin();
        exit(2);
      }
    }
  }else {
    int i = active_window - 1;
    if (i < 0) i = max;
    while (true) {
      if (WINDOW_STATUS[i] != -1) {
        active_window = i;
        return;
      }
      if (--i < 0) i = max;
      if (f_inf++ > max) {
        endwin();
        exit(2);
      }
    }
  }
}

int main(int argc, char *argv[])
{
  int ch;
  int f_flag = 0;
  char *user_config_dir = get_user_config_dir();
  char *filename;
  int config_err = create_user_config_dir(user_config_dir);
  if(config_err) {
    filename = "data";
  } else {
    filename = malloc(strlen(user_config_dir) + strlen(CONFIGDIR) + strlen("data") + 1);
    strcpy(filename, user_config_dir);
    strcat(filename, CONFIGDIR);
    strcat(filename, "data");
  }
  
  ToxWindow* a;
  int i = 0;
  for (i = 0; i < argc; ++i) {
    if (argv[i] == NULL)
      break;
    else if (argv[i][0] == '-') {
      if (argv[i][1] == 'f') {
        if (argv[i + 1] != NULL)
          filename = argv[i + 1];
        else
          f_flag = -1;
      }
    }
  }

  init_term();
  init_tox();
  load_data(filename);
  free(filename);
  init_windows();
  init_window_status();

  if (f_flag == -1) {
    attron(COLOR_PAIR(3) | A_BOLD);
    wprintw(prompt->window, "You passed '-f' without giving an argument!\n"
                            "defaulting to 'data' for a keyfile...\n");
    attroff(COLOR_PAIR(3) | A_BOLD);
  }

  if(config_err) {
    attron(COLOR_PAIR(3) | A_BOLD);
    wprintw(prompt->window, "Unable to determine configuration directory!\n"
                            "defaulting to 'data' for a keyfile...\n");
    attroff(COLOR_PAIR(3) | A_BOLD);
  }
  
  while(true) {
    /* Update tox */
    do_tox();

    /* Draw */
    a = &windows[active_window];
    prepare_window(a->window);
    a->blink = false;
    draw_bar();
    a->onDraw(a);

    /* Handle input */
    ch = getch();
    if (ch == '\t' || ch == KEY_BTAB)
      set_active_window(ch);
    else if (ch != ERR)
      a->onKey(a, ch);
  }
  return 0;
}
