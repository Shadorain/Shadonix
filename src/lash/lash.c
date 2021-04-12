#include "../include/libshado.h"
#include "../include/printf.h"
#include <fcntl.h>
#include <unistd.h>
#include <linux/input-event-codes.h>

unsigned long console_fd = 0;

struct event_file {
    struct event_file *next;
    int fd;
    char name[64];
};

struct input_event {
    struct timeval time;
    unsigned short type;
    unsigned short code;
    unsigned int value;
};

struct mouse_pos_info {
    int x;
    int y;
    int max_x;
    int max_y;
};

struct event_file *event_list_head;
struct mouse_pos_info mouse_pos;

void load_event_devs () {
    event_list_head = NULL;
    mouse_pos.x = 0;
    mouse_pos.y = 0;
    mouse_pos.max_x = 1920;
    mouse_pos.max_y = 1080;

    for (int i=0; i<20; i++) {
        char name[64];
        sprintf(name, "/dev/input/event%d", i);
        int fd = file_open(name, O_RDONLY);
        if (fd < 0) break; /* not found */

        struct event_file *e = malloc(sizeof(struct event_file));
        str_cpy(e->name, name);
        e->fd = fd;
        e->next = event_list_head;
        event_list_head = e;
    }
}

bool handle_event (struct event_file *e, struct input_event *event) {
    if (event->type == EV_REL) { /* mouse event */
        if (event->code == REL_X) {
            int new_x = mouse_pos.x + event->value;
            if (new_x >= 0 && new_x <= mouse_pos.max_y)
                mouse_pos.x = new_x;
        }
        if (event->code == REL_Y) {
            int new_y = mouse_pos.y + event->value;
            if (new_y >= 0 && new_y <= mouse_pos.max_x)
                mouse_pos.y = new_y;
        }
        printf("\rMOUSE_POS: %d - %d             ", mouse_pos.x, mouse_pos.y);
    }
    if (event->type == EV_KEY) {
        printf("INPUT: %s - %d - %d - %d\n", e->name, event->code,
                event->type, event->value);
        if (event->code == KEY_END) { 
            printf("Exiting...\n");
            return true;
        }
    }
    return false;
}

void handle_events () {
    printf("Listening for events...\n");
    while (true) {
        fd_set fds;
        FD_ZERO(&fds);

        struct event_file *e = event_list_head;
        while (e) {
            FD_SET(e->fd, &fds);
            e = e->next;
        }

        int ret = sys_select(event_list_head->fd+1, &fds, NULL, NULL, NULL);
        if (ret < 0) {
            printf("SELECT FAILED!\n");
            return;
        }

        e = event_list_head;
        while (e) {
            if (FD_ISSET(e->fd, &fds)) {
                char buf[1024];
                int r = file_read(e->fd, buf, sizeof(buf));
                int pos = 0;
                while (pos < r) {
                    struct input_event *event = (struct input_event*)(buf+pos);
                    pos += sizeof(struct input_event);
                    
                    if (handle_event(e, event)) return;
                }
            }
            e = e->next;
        }
    }
}

void console_open () {
    console_fd = file_open("/dev/console", O_RDWR | O_NDELAY);
}

char console_read () {
    char c = 0;
    while (!file_read(console_fd, &c, 1)) {
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 10000;
        sys_nanosleep(&ts, NULL);
    }
    return c;
}

void console_write (char c) {
    file_write(console_fd, &c, 1);
}

int line_read (char *buf, int max) {
    int i=0;

    for (; i<max; i++) {
        char c = console_read();
        if (c == 0) {
            i--;
            continue;
        }
        console_write(c);
        buf[i] = c;
        if (c == '\b') {
            i--;
            buf[i] = 0;
        }

        if (c == '\n') {
            buf[i] = 0;
            return i;
        }
    }
    return i;
}

unsigned long cur_brk = 0;

char **cmd_to_args(char *cmd) {
    int num_spaces = 0;
    int len = str_len(cmd);
    for (int i=0; i<len; i++) {
        if (cmd[i] == ' ') {
            cmd[i] = 0;
            num_spaces++;
        }
    }
    char **argv = malloc(sizeof(char*) * (num_spaces+1));
    int cur_item = 0;
    argv[0] = cmd;
    for (int i=0; i<len; i++)
        if (cmd[i] == 0)
            argv[++cur_item] = cmd + i + 1;
    argv[++cur_item] = 0;
    return argv;
}

bool find_cmd (char *full_path) {
    struct stat statbuf;
    char bin_path[64];
    if (file_stat(bin_path, &statbuf) == 0) return true;
    str_cpy(full_path, bin_path);
    sprintf(full_path, "/bin/%s", full_path);
    if (file_stat(full_path, &statbuf) == 0) return true;
    return false;
}

bool proc_cmd (char *cmd) {
    char **argv = cmd_to_args(cmd);

    if (str_eq(cmd, "reboot")) {
        str_print("\n\n*** REBOOTING ***\n");
        sys_reboot();
    } else if (str_eq(cmd, "alloc")) {
        int size = 0;
        if (argv[1]) size = str_to_int(argv[1]);

        void *p = malloc(size);
        printf("Returned ptr: %lX\n", p);
        print_heap();
    } else if (str_eq(cmd, "free")) {
        /* free ADDR */
        unsigned long addr = hex_str_to_ulong(argv[1]);
        free((void*)addr);
        print_heap();
    } else if (str_eq(cmd, "store")) {
        char *val = argv[2]; /* second arg */
        unsigned long addr = hex_str_to_ulong(argv[1]);
        int n = str_to_int(val);
        printf("Storing %d at %X\n", n , addr); 

        int *p = (int*)addr;
        *p = n;
    } else if (str_eq(cmd, "fetch")) {
        unsigned long addr = hex_str_to_ulong(argv[1]);
        int *p = (int*)addr;

        printf("Fetched %d from %X\n", *p, addr);
    } else if (str_eq(cmd, "events")) handle_events();
    else {
        struct stat statbuf;
        char full_path[64];
        str_cpy(full_path, cmd);
        if (find_cmd(full_path)) {
            long pid = ps_fork();
            if (pid == 0) {
                char *envp[1];
                envp[0] = 0;
                int ret =ps_execve(full_path, argv, envp);
                printf("Ret: %d\n", ret);
                free(argv);
                _exit(ret);
                return false;
            } else {
                siginfo_t info;
                int status = 0;
                int ret = sys_waitid(P_PID, pid, &info, WEXITED);
                printf("Process returned: %s\n", ret);
            }
        }
    }
    free(argv);
    return true;
}

int main () {
    str_print("\033[H\033[J");
    str_print("Sh v0.0.0.5\n");

    /* print_heap(); */
    /* console_open(); */
    /* load_event_devs(); */
    /* print_heap(); */

    str_print("|=-> ");
    while (1) {
        char buf[1024];
        line_read(buf, sizeof(buf));

        if (str_eq("", buf)) {
            str_print("|=-> ");
            continue;
        }
        if (!proc_cmd(buf))
            return 0;

        str_print ("|=-> ");
    }
}
