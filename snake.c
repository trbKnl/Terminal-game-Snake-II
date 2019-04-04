#include <ncurses.h>			/* ncurses.h includes stdio.h */  
#include <stdbool.h> 
#include <string.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <time.h>

#define LENGTH_ARRAY(x) ((int)(sizeof(x) / sizeof(x[0])))

/* Game parameters */
#define LIFETIME_MONSTER 10
/* wait GAME_SPEED microseconds until next input is processed */
#define GAME_SPEED 50000 

/* Global variables */
const int movesDirection[] = {'h', 'j', 'k', 'l', KEY_LEFT, KEY_DOWN, KEY_UP, KEY_RIGHT}; /* left down up right */
const int movesOppositeDirection[] = {'l', 'k', 'j', 'h', KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_LEFT}; 
const int nMoves = LENGTH_ARRAY(movesDirection);
char *monsters[5] = {"(>'.')>", "<('.'<)", "(0_0)", "{>.<}", "<(*-*)>"};
int highscore = 0;
int currentScore = 0;

/* structures needed for the game */
struct point {
    int x, y;
};

struct node {
    char symbol;
    struct point location;
    struct node *next;
};

/* Function prototypes */
/* Functions for the game */
void play_game(int *col, int *row);
int mod(int a, int b);
void move_head(int *col, int *row, int *ch, struct point *head);
bool determine_action(int *col, int *row, struct node **snake_start, struct node **monster_start, struct point *head, struct point *food);
void place_food(int *col, int *row, struct node *snake_start, struct point *food);
void place_monster(int *col, int *row, struct node *snake_start, struct node **monster_start, struct point *food);
bool handle_input(int *ch, int *ch_prev);
void print_status_bar(int *col, int *row, bool monsterInPlay, int timeElapsed);

/* Linked list function */
void add_to_list(struct node **list, struct point location, char symbol);
void print_list(struct node *start);
void delete_from_end_of_list(struct node **list);
bool is_equal(struct point *a, struct point *b);
bool in_list(struct node *start,  struct point *a);
int length_linked_list(struct node *start);
void empty_list(struct node **list); 
 

int main() {
 
 int row, col;
 time_t t1;
 srand ((unsigned) time (&t1));

 /* Initialize ncurses */
 initscr();				        /* start the curses mode */
 getmaxyx(stdscr, row, col);	/* get the number of rows and columns */
 curs_set(0);                   /* No cursor */
 noecho();                      /* Do not print input from keyboard */
 keypad(stdscr, true);

 while (1) {
     print_status_bar(&col, &row, false, 0);
     /* Play the game */
     play_game(&col, &row);
 }

 clear();
 getch();
 endwin();

 return 0;
}



void play_game(int *col, int *row) {

     /* Initialize playing field */
     struct point food = {};
     struct node *snake_start = NULL; 
     struct node *monster_start = NULL; 
     struct point head = {.x = *row/2, .y = *col/2};
     currentScore = 0;

     add_to_list(&snake_start, head, '#'); 
     mvprintw(head.x, head.y, "#");
     place_food(col, row, snake_start, &food);
     
     int ch = '\0', ch_prev = '\0';
     bool gameOngoing = true;

    /* handle_input returns true if input is invalid
     * This function is here, to flush invalid input at the start of the game */
    while (handle_input(&ch, &ch_prev)) {
        ;
    }

    nodelay(stdscr, true); 
    while (gameOngoing) {
    /* Game loop:
     * 1. move the head of the sneak to new location
     * 2. check if head collided
     * 3. retrieve new input 
     */
        move_head(col, row, &ch, &head); 
        gameOngoing = determine_action(col, row, &snake_start, &monster_start, &head, &food);
        handle_input(&ch, &ch_prev);
    }
    nodelay(stdscr, false); 
}


int mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

bool handle_input(int *ch, int *ch_prev) {

    /* Read input */
    usleep(GAME_SPEED);
    *ch_prev = *ch;
    *ch = getch();

    bool invalidChar = true;
    for (int i = 0; i < nMoves; i++) {
        if (*ch == movesDirection[i]
                && *ch_prev != movesOppositeDirection[i] ) {
            invalidChar = false;
            break ;
        } 
    }
    if (invalidChar) {
        *ch = *ch_prev;
    }
    return invalidChar;
}

void place_monster(int *col, int *row, struct node *snake_start, struct node **monster_start, struct point *food) {

    empty_list(monster_start);
    struct point monster; /* Starting place of monster */
    bool monsterCannotSpawn = true; 
    char *ptr_monster = monsters[(rand() % LENGTH_ARRAY(monsters))]; /* load random monster */
    int monster_size = strlen(ptr_monster);
    struct node *ptr_m; /* this will point to the first char of the monster */ 

    while (monsterCannotSpawn) {

        monster.x = (rand() % (*row-1)) + 1; 
        monster.y = (rand() % (*col-1-monster_size)) + 1; 

        /* Put monster in linked list */
        for (int i = 0; i < monster_size; i++) {
            add_to_list(monster_start, monster, ptr_monster[i]);
            monster.y += 1;
        }

        /* check if monster collides with snake or food
         * If so, choose a new location of the monster */
        ptr_m = *monster_start;
        while (ptr_m != NULL) {
            if (in_list(snake_start, &(ptr_m->location)) || 
                    is_equal(&(ptr_m->location), food)) {
                empty_list(monster_start);
                break;
            }
            mvprintw(ptr_m->location.x, ptr_m->location.y, "%c", ptr_m->symbol);
            ptr_m = ptr_m->next;
        }

        /* If monster did not collide with food or snake */
        if (ptr_m == NULL) {
            monsterCannotSpawn = false;
            /* Reprint the snake and the food.
             * They could have been overwritten by nonplaceble monsters */
            print_list(snake_start);
            mvprintw(food->x, food->y, "@");
        }
    }
}


void place_food(int *col, int *row, struct node *snake_start, struct point *food) {
    food->x = (rand() % (*row-1)) + 1;
    food->y = (rand() % (*col-1)) + 1; 
    
    /* If food spawns in the snake itself, place food elsewhere */
    while (in_list(snake_start, food)) {
        food->x = (rand() % (*row-1)) + 1;
        food->y = (rand() % (*row-1)) + 1;
    }
    mvprintw(food->x, food->y, "@");
}

void blink_snake(struct node *snake_start) {
    for (int i = 0; i < 5; i++) {
        for (struct node *ptr_snake = snake_start;
                ptr_snake != NULL;
                ptr_snake = ptr_snake->next) {
                    mvprintw(ptr_snake->location.x, ptr_snake->location.y,  " ");
        }
        getch();
        usleep(80000);
        for (struct node *ptr_snake = snake_start;
                ptr_snake != NULL;
                ptr_snake = ptr_snake->next) {
                    mvprintw(ptr_snake->location.x, ptr_snake->location.y,  "#");
        }
        getch();
        usleep(80000);
    }
}

bool determine_action(int *col, int *row, struct node **snake_start, struct node **monster_start, struct point *head, struct point *food) {

    /* These variables need to remain between function calls */
    static bool monsterInPlay = false; /* flag if monster is in play */
    static struct timespec ts_monsterStart, ts_monsterEnd; /* time keeping */
    static int timeElapsed; /* length of monster's life */

    /* determine what to do upon collision, itself, food, monster */ 
        if (in_list(*snake_start, head)) {
            blink_snake(*snake_start);
            empty_list(snake_start);   /* Delete and erase snake */
            empty_list(monster_start); /* Delete and erase monster */
            mvprintw(food->x, food->y, " "); /* Delete remaning food */
            monsterInPlay = false;
            highscore = (currentScore > highscore) ? currentScore : highscore;
            return false;   /* Game lost exit function */
        } else if (is_equal(head, food)) {
            currentScore += 1;
            place_food(col, row, *snake_start, food);
            mvprintw(food->x, food->y, "@");
            if (!monsterInPlay) {
                place_monster(col, row, *snake_start, monster_start, food);
                clock_gettime(CLOCK_MONOTONIC, &ts_monsterStart);
                monsterInPlay = true;
            }
        } else if (monsterInPlay && in_list(*monster_start, head)) {
            currentScore += 10;
            empty_list(monster_start);
            monsterInPlay = false;
        } else {
            delete_from_end_of_list(snake_start);
        }

    /* if monster is in play, decrease monster life timer */ 
        if(monsterInPlay) {
            clock_gettime(CLOCK_MONOTONIC, &ts_monsterEnd);
            timeElapsed = ts_monsterEnd.tv_sec - ts_monsterStart.tv_sec;
            if (LIFETIME_MONSTER - timeElapsed < 0) {
                empty_list(monster_start);
                monsterInPlay = false;
            } 
        }

    /* Print the next positiion of the head of the snake and print status bar */
        add_to_list(snake_start, *head, '#'); 
        mvprintw(head->x, head->y, "#");
        print_status_bar(col, row, monsterInPlay, LIFETIME_MONSTER - timeElapsed);
        return true;
}


void move_head(int *col, int *row, int *ch, struct point *head) {
   if (*ch == 'j' || *ch == KEY_DOWN) { 
        head->x += 1;
        head->x = mod(head->x, *row); 
        head->x = (head->x == 0) ? 1 : head->x;  /* remove first row */
   } else if (*ch == 'k' || *ch == KEY_UP) {
        head->x -= 1; 
        if (head->x == 0) { head->x -= 1; /* remove first row */}
        head->x = mod(head->x, *row); 
   } else if (*ch == 'h' || *ch == KEY_LEFT) {
        head->y -= 1; 
        head->y = mod(head->y, *col); 
   } else if (*ch == 'l' || *ch == KEY_RIGHT) {
        head->y += 1; 
        head->y = mod(head->y, *col); 
   } else {
       ;
   }
}

void add_to_list(struct node **list, struct point location, char symbol) {

    struct node *new_node;
    new_node = malloc(sizeof(struct node));

    new_node->symbol = symbol;
    new_node->location = location;
    new_node->next = *list;
    *list = new_node;
}


void empty_list(struct node **list) {
    struct node *prev = *list;
    while (*list) {
        mvprintw((*list)->location.x, (*list)->location.y, " ");
        *list = (*list)->next;
        free(prev);
        prev = *list;
    }
}

void delete_from_end_of_list(struct node **list) {
    struct node *cur = *list, *prev = NULL;
    for (;  cur->next != NULL; 
            prev = cur, cur = cur->next) {
        ;
    }
    if (cur == NULL) { /* list is empty */ 
        return ;
    }
    if (prev == NULL) {
        *list = cur->next; /* list is deleted */
    } else {
        prev->next = NULL; 
    }
    mvprintw(cur->location.x, cur->location.y,  " ");
    free(cur);
}

void print_list(struct node *start) {
    for (struct node *node_ptr = start;
            node_ptr != NULL;
            node_ptr = node_ptr->next) {
        mvprintw(node_ptr->location.x, node_ptr->location.y, "%c", node_ptr->symbol);
    } 

}

int length_linked_list(struct node *start) {
    struct node *node_ptr = start;
    int i = 0;

    for (; node_ptr != NULL; node_ptr = node_ptr->next) {
        i++;
    } 
    return i;
}

bool in_list(struct node *start,  struct point *a) {
    if ( start != NULL ) {
        for (struct node *node_ptr = start; node_ptr != NULL; node_ptr = node_ptr->next) {
            if (is_equal(&(node_ptr->location), a)) {
                return true;
            }
        } 
    }
    return false;
}

bool is_equal(struct point *a, struct point *b) {
    if (a->x == b->x && a->y == b->y) {
        return true;
    } else {
        return false;
    }
}

void print_status_bar(int *col, int *row, bool monsterInPlay, int timeElapsed) {
    char status_bar[1000];
    char score_str[] = "Score:";
    char highscore_str[] = "High:";
    if (monsterInPlay) {
        int first_padding = *col/2 - (strlen(score_str) + 2 + 5);
        int second_padding = *col - first_padding -
            (strlen(score_str) + 2 + 5) - (4 + 5);
        sprintf(status_bar, " %s %05d %*d %*s %05d ", score_str,
                currentScore, first_padding, timeElapsed,
                second_padding, highscore_str, highscore);
    } else {
        int first_padding = *col - (strlen(score_str) + 2 + 5) -  (3 + 5) ;
        sprintf(status_bar, " %s %05d %*s %05d ", score_str,
                currentScore, first_padding, highscore_str, highscore);
    }
    if (strlen(status_bar) > *col) {
        status_bar[*col] = '\0';
        status_bar[*col-1] = '.';
        status_bar[*col-2] = '.'; 
        status_bar[*col-3] = '.'; 
    }
    mvprintw(0,0, "%s", status_bar);
}






