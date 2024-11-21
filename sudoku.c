#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#endif

// Структура поля судоку
typedef struct {
    int size;
    int **cells;
    int **fixed;
} SudokuField;

// Задержка для брутфорса. Чем ниже число, тем быстрее будет брутфорс (в милисекундах)
#define DELAY 100

// Функция для очистки экрана
void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[H\033[J");
#endif
}

// Функция для задержки выполнения программы
void sleep_ms(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

// Функция для инициализации поля судоку
SudokuField* initialize_field(int size) {
    SudokuField *field = (SudokuField*)malloc(sizeof(SudokuField));
    field->size = size;
    field->cells = (int**)malloc(size * sizeof(int*));
    field->fixed = (int**)malloc(size * sizeof(int*));
    for (int i = 0; i < size; i++) {
        *(field->cells + i) = (int*)calloc(size, sizeof(int));
        *(field->fixed + i) = (int*)calloc(size, sizeof(int));
    }
    return field;
}

// Функция для освобождения памяти, занятой полем судоку
void free_field(SudokuField *field) {
    for (int i = 0; i < field->size; i++) {
        free(*(field->cells + i));
        free(*(field->fixed + i));
    }
    free(field->cells);
    free(field->fixed);
    free(field);
}

// Функция для проверки, безопасно ли поставить число в блоке
int is_safe_in_box(SudokuField *field, int startRow, int startCol, int num) {
    int box_size = (field->size == 9) ? 3 : 2;
    for (int i = 0; i < box_size; i++) {
        for (int j = 0; j < box_size; j++) {
            if (*(*(field->cells + (i + startRow)) + (j + startCol)) == num) {
                return 0;
            }
        }
    }
    return 1;
}

// Функция для проверки, безопасно ли поставить число в клетку с учетом строки, столбца и блока
int is_safe_to_place(SudokuField *field, int row, int col, int num) {
    for (int i = 0; i < field->size; i++) {
        if (*(*(field->cells + row) + i) == num || *(*(field->cells + i) + col) == num) {
            return 0;
        }
    }
    int box_size = (field->size == 9) ? 3 : 2;
    return is_safe_in_box(field, row - row % box_size, col - col % box_size, num);
}

// Рекурсивная функция для решения судоку с использованием бэктрекинга
int solve_sudoku(SudokuField *field, int row, int col) {
    if (row == field->size - 1 && col == field->size) return 1;
    if (col == field->size) {
        row++;
        col = 0;
    }
    if (*(*(field->cells + row) + col) != 0) return solve_sudoku(field, row, col + 1);

    for (int num = 1; num <= field->size; num++) {
        if (is_safe_to_place(field, row, col, num)) {
            *(*(field->cells + row) + col) = num;
            if (solve_sudoku(field, row, col + 1)) return 1;
            *(*(field->cells + row) + col) = 0;
        }
    }
    return 0;
}

// Функция для перемешивания массива чисел (обеспечивает рандомность генерации поля судоку)
void shuffle(int *arr, int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = *(arr + i);
        *(arr + i) = *(arr + j);
        *(arr + j) = temp;
    }
}

// Функция для инициализации случайного поля судоку
void initialize_random_field(SudokuField *field) {
    int *numbers = (int*)malloc(field->size * sizeof(int));
    for (int i = 0; i < field->size; i++) {
        *(numbers + i) = i + 1;
    }
    shuffle(numbers, field->size);

    for (int i = 0; i < field->size; i++) {
        *(*(field->cells) + i) = *(numbers + i);
    }

    free(numbers);
}

// Функция для генерации судоку
void generate_puzzle(SudokuField *field) {
    srand(time(NULL));
    initialize_random_field(field);
    solve_sudoku(field, 0, 1);

    int clues = field->size == 9 ? 20 : 5;
    int total_cells = field->size * field->size;
    int removed = total_cells - clues;

    while (removed > 0) {
        int row = rand() % field->size;
        int col = rand() % field->size;
        if (*(*(field->cells + row) + col) != 0) {
            *(*(field->cells + row) + col) = 0;
            *(*(field->fixed + row) + col) = 0;
            removed--;
        }
    }

    for (int i = 0; i < field->size; i++) {
        for (int j = 0; j < field->size; j++) {
            if (*(*(field->cells + i) + j) != 0) {
                *(*(field->fixed + i) + j) = 1;
            }
        }
    }
}


// Функция для проверки победы
int check_win(SudokuField *field) {
    int size = field->size;
    for (int i = 0; i < size; i++) {
        int row[10] = {0}, col[10] = {0};
        for (int j = 0; j < size; j++) {
            int num_row = *(*(field->cells + i) + j);
            int num_col = *(*(field->cells + j) + i);

            if (num_row == 0 || *(row + num_row)  == 1) return 0;
            if (num_col == 0 || *(col + num_col) == 1) return 0;

            *(row + num_row) = 1;
            *(col + num_col) = 1;
        }
    }
    return 1;
}

// Функция для проверки поражения
int check_loss(SudokuField *field) {
    int size = field->size;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (*(*(field->cells + i) + j) == 0) {
                return 0;
            }
        }
    }

    for (int i = 0; i < size; i++) {
        int row[10] = {0}, col[10] = {0};
        for (int j = 0; j < size; j++) {
            int num_row = *(*(field->cells + i) + j);
            int num_col = *(*(field->cells + j) + i);

            if ((num_row != 0 && *(row + num_row) == 1) || (num_col != 0 && *(col + num_col) == 1)) {
                return 1; 
            }
            *(row + num_row) = 1;
            *(col + num_col) = 1;
        }
    }

    int box_size = (size == 9) ? 3 : 2;
    for (int boxRow = 0; boxRow < size; boxRow += box_size) {
        for (int boxCol = 0; boxCol < size; boxCol += box_size) {
            int block[10] = {0};
            for (int r = 0; r < box_size; r++) {
                for (int c = 0; c < box_size; c++) {
                    int num = *(*(field->cells + (boxRow + r)) + (boxCol + c));
                    if (num != 0 && *(block + num) == 1) {
                        return 1;
                    }
                    *(block + num) = 1;
                }
            }
        }
    }

    return 0;
}

// Вывод поля судоку на экран
void print_field(SudokuField *field, int sel_row, int sel_col) {
    clear_screen();
    for (int i = 0; i < field->size; i++) {
        for (int j = 0; j < field->size; j++) {
            if (i == sel_row && j == sel_col) {
                printf("[");
            } else {
                printf(" ");
            }

            if (*(*(field->cells + i) + j) == 0) {
                printf(".");
            } else if (*(*(field->fixed + i) + j)) {
                printf("\033[1;37m%d\033[0m", *(*(field->cells + i) + j));
            } else {
                printf("\033[0;32m%d\033[0m", *(*(field->cells + i) + j));
            }

            if (i == sel_row && j == sel_col) {
                printf("] ");
            } else {
                printf("  ");
            }
        }
        printf("\n");
    }
}

// Показ финального поля - результат
void result(SudokuField *field, int won) {
    for (int i = 0; i < 3; i++) {
        clear_screen();
        for (int r = 0; r < field->size; r++) {
            for (int c = 0; c < field->size; c++) {
                int cell = *(*(field->cells + r) + c);
                if (*(*(field->fixed + r) + c)) {
                    printf("\033[1;37m%d\033[0m ", cell);
                } else {
                    printf("%s%d\033[0m ", won ? "\033[0;32m" : "\033[0;31m", cell);
                }
            }
            printf("\n");
        }
        fflush(stdout);
        sleep_ms(500);
    }

    printf("\n%s\n", won ? "\033[0;32mWIN\033[0m" : "\033[0;31mLOSE\033[0m");
    printf("Press Enter to exit...");
    while (getchar() != '\n');
}

// Незамедлительный ввод и принятие нажатой клавиши
char get_input() {
#ifdef _WIN32
    return _getch();
#else
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
#endif
}

// Нажатие клавиш
void user_input(SudokuField *field) {
    int row = 0, col = 0;
    generate_puzzle(field);

    while (1) {
        print_field(field, row, col);
        printf("Use WASD to move, 1-9 to enter number, Q to quit\n");
        printf("Press Delete/Backspace to clear cell, Space to clear all user entries\n");

        char command = get_input();

        if (command == 'w' && row > 0) row--;
        else if (command == 's' && row < field->size - 1) row++;
        else if (command == 'a' && col > 0) col--;
        else if (command == 'd' && col < field->size - 1) col++;
        
        else if (command >= '1' && command <= '9' && !*(*(field->fixed + row) + col)) {
            int num = command - '0';
            if (num > 0 && num <= field->size) *(*(field->cells + row) + col) = num;
        }

        else if ((command == 127 || command == 8) && !*(*(field->fixed + row) + col)) {
            *(*(field->cells + row) + col) = 0;
        }

        else if (command == ' ') {
            for (int i = 0; i < field->size; i++) {
                for (int j = 0; j < field->size; j++) {
                    if (!*(*(field->fixed + i) + j)) *(*(field->cells + i) + j) = 0;
                }
            }
        }

        else if (command == 'q') {
            result(field, 0);
            return;
        }

        if (check_win(field)) {
            result(field, 1);
            return;
        } else if (check_loss(field)) {
            result(field, 0);
            return;
        }
    }
}

// Функция для визуализации решения
int bruteforce_input(SudokuField *field, int row, int col, int delay) {
    if (row == field->size && col == 0) return 1;

    if (col == field->size) {
        row++;
        col = 0;
    }

    if (*(*(field->fixed + row) + col)) 
        return bruteforce_input(field, row, col + 1, delay);

    for (int num = 1; num <= field->size; num++) {
        if (is_safe_to_place(field, row, col, num)) {
            *(*(field->cells + row) + col) = num;
            print_field(field, -1, -1); 
            sleep_ms(delay); 
            
            if (bruteforce_input(field, row, col + 1, delay))
                return 1;

            *(*(field->cells + row) + col) = 0;  
            print_field(field, -1, -1);  
            sleep_ms(delay);  
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int size;
    printf("Choose Sudoku field size (4 or 9): ");
    scanf("%d", &size);
    getchar(); 

    if (size != 4 && size != 9) {
        printf("Invalid size\n");
        return 1;
    }

    int AUTO_SOLVE = 0;
    if (argc > 1) {
        AUTO_SOLVE = atoi(argv[1]); 
    }

    SudokuField *field = initialize_field(size);
    generate_puzzle(field);

    if (AUTO_SOLVE == 1) {
        user_input(field); 
    } else if (AUTO_SOLVE == 2) {
        bruteforce_input(field, 0, 0, DELAY);
    } else {
        printf("Invalid AUTO_SOLVE value. Please use 1 for manual input or 2 for brute force.\n");
    }

    free_field(field);
    return 0;
}