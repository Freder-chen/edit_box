#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <fcntl.h>

#define X0  0
#define Y0  0
#define CTRL_S 19
#define ARROW_KEY 224
#define ARROW_UP    72
#define ARROW_DOWN  80
#define ARROW_LEFT  75
#define ARROW_RIGHT 77
#define ENTER 13
#define BACKSPACE  8

/* Return Error number, but I didn't realize it. =.= */
int edit_box(char *filename);