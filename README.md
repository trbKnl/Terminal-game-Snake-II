### Snake II for the terminal

A fun game to play in the terminal, to kill some time. It is build using the standard libraries and the ncurses library for the user interface. You can compile the program with, for example, the gcc compiler linking with the ncurses library as follows: `gcc snake.c -o snake -lncurses`.

I wrote this program to learn some C. I used a linked list to store the location of the snake, mainly because I wanted to implement a linked list. The code works as follows:
1. Initialize playing field
2. retrieve input and validate keyboard input
3. determine the next location of the snake 
4. collision detection and back to 2.

  
