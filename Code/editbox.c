/* This is a very simple editor by "Freder".
 * There are some bugs, and some features are not implemented.
 *
 * ------------------------------------------------------------
 * It's my account as a freshman for college.
 *                                         --- February 3, 2018
 */

#include "editbox.h"

struct Word
{
	wchar_t ch;
	struct Word *last;
	struct Word *next;
};

struct EditBoxConfig
{
	char *filename;
	short cursor_x, cursor_y;
	short screen_rows, screen_cols;
	struct Word *head_word;
	// Head_word has no characters(WEOF) which follows by the text.
};

static struct EditBoxConfig editor;

/* ================================ Edit box display control ======================= */

// Get window size.
void get_window_size(short *rows, short *columns)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    *columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

// Control cursor style.
// --- b : only TRUE and FALSE representatives of true and false.
void cursor_info(BOOL b)
{
   CONSOLE_CURSOR_INFO info;
   info.dwSize = 100;
   info.bVisible = b;
   SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

// Move cursor to (x, y).
void move_cursor_to(short x, short y)
{
	COORD coord = { x,y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void update_cursor(short x, short y)
{
	editor.cursor_x = x % editor.screen_cols;
	editor.cursor_y = y + x / editor.screen_cols;
}

// Get char width.
short get_word_width(wchar_t ch)
{
	return (ch & 0xFF00) == 0 ? 1 : 2;
}

/* =========================== Low level handling of text ============================== */

struct Word *allocate_word(wchar_t ch, struct Word *last_word, struct Word *next_word)
{
	struct Word *word = (struct Word *)malloc(sizeof(struct Word));
	word->ch = ch;
	word->last = last_word;
	word->next = next_word;
	return word;
}

// Delete the word which as formal parameter,
// and return the last word of it.
struct Word *free_word(struct Word *word)
{
	struct Word *w = word->last;
	if (w != NULL) w->next = word->next;
	free(word);
	return w;
}

struct Word *find_last(struct Word *word, short num)
{
	short i;
	for (i = 0; i < num && word != NULL; i++)
	{
		word = word->last;
		if (word->ch == '\r')
		{
			word = word->last;
			break;
		}
	}
	return word;
}

struct Word *find_next(struct Word *word, short num)
{
	short i;
	for (i = 0; i < num && word != NULL; i++)
	{
		word = word->next;
		if (word->ch == '\r')
		{
			word = word->next;
			break;
		}
	}
	return word;
}

/* ========================== Edit box block implementation ============================ */

/* It points to the current operation character */
struct Word *present_word;
/* It points to the head of the content that needs to be printed */
struct Word *print_head_word;

void insert_word(wchar_t ch)
{
	struct Word *word = allocate_word(ch, present_word, present_word->next);
	present_word->next = word;
	present_word = word;
}

void delete_word()
{
	if (present_word->ch == WEOF) return;
	if (present_word->ch == '\n') present_word = free_word(present_word);
	present_word = free_word(present_word); 
}

void move_cursor(wchar_t key)
{
	struct Word *word = NULL;
	switch (key)
	{
		case ARROW_UP:
			word = find_last(present_word, editor.screen_cols);
			break;
		case ARROW_DOWN:
			word = find_next(present_word, editor.screen_cols);
			break;
		case ARROW_LEFT:
			word = find_last(present_word, 1);
			break;
		case ARROW_RIGHT:
			word = find_next(present_word, 1);
			break;
		default:
			break;
	}
	if (word != NULL) present_word = word;
}

void save(char *filename)
{
	if (filename == NULL) return;
	struct Word *word;
	FILE * fp = fopen(filename, "wb,ccs=UNICODE");
	_setmode(_fileno(stdin), _O_U16TEXT);
	_setmode(_fileno(fp), _O_U16TEXT);
	for (word = editor.head_word->next; word != NULL; word = word->next)
		fputwc(word->ch, fp);
	fclose(fp);
	_setmode(_fileno(stdin), _O_TEXT);
}

/* ========================== Edit box events handling ============================ */

void refresh_screen()
{
	short i, j, cw; // cw is character width.
	struct Word *word = print_head_word->next;
	cursor_info(FALSE);
	move_cursor_to(X0, Y0);
	if (present_word == print_head_word) update_cursor(X0, Y0);
	for (i = 0; i < editor.screen_rows - 1; i++)
	{
		for (j = 0; j < editor.screen_cols && word != NULL; j += cw, word = word->next)
		{
			if (word->ch == '\r') cw = 0;
			else if (word->ch == '\n')
			{
				if (word == present_word) update_cursor(X0, i + 1);
				word = word->next;
				break;
			}
			else
			{
				cw = get_word_width(word->ch);
				if (word == present_word) update_cursor(j + cw, i);
				_putwch(word->ch);
			}
		}
		for (; j < editor.screen_cols; j++)
			_putwch(' ');
	}
	printf("x: %d, y: %d       ", editor.cursor_x, editor.cursor_y); // Space prevents misplaced characters.
	move_cursor_to(editor.cursor_x, editor.cursor_y);
	cursor_info(TRUE);
}

int process_keypress()
{
	wchar_t ch;
	switch (ch = _getwch())
	{
		case CTRL_S:
			save(editor.filename);
			return 0;
		case ARROW_KEY:
			move_cursor(_getwch());
			break;
		case ENTER:
			insert_word('\r');
			insert_word('\n');
			break;
		case BACKSPACE:
			delete_word();
			break;
		default: // Font
			insert_word(ch);
			break;
	}
	return 1;
}

void init_editor(char *filename)
{
	editor.filename = filename;
	editor.cursor_x = X0;
	editor.cursor_y = Y0;
	editor.head_word = allocate_word(WEOF, NULL, NULL);
	get_window_size(&editor.screen_rows, &editor.screen_cols);
	move_cursor_to(editor.cursor_x, editor.cursor_y);
	cursor_info(TRUE);
}

int edit_box(char *filename)
{
	init_editor(filename);
	print_head_word = present_word = editor.head_word;
	do {
		refresh_screen();
	} while (process_keypress());
	return 0;
}