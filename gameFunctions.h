//
// Created by Simone on 26/11/2024.
//

#include <stdio.h>
#include <SDL.h>
#include <time.h>
#include <string.h>

#ifndef TETRIS_GAMEFUNCTIONS_H
#define TETRIS_GAMEFUNCTIONS_H
// GLOBAL VARIABLES
int key_sym = 0;
int key_sym_pre = 0;

int dynamic_field[NROW][NCOL];  // MATRIX USED FOR FALLING PIECES IN MOVEMENT
int static_field[NROW][NCOL];   // MATRIX USED FOR PLACED PIECES

int fell = 1;
int placed = 1;

int col = 0, row = 0;
int rotation = 1;
int piece_num = 1;
int piece_matrix[NROW_TEMP][NCOL_TEMP] = {0};
int update_dynamic_sprite = TRUE;
int update_static_sprite = FALSE;

Uint32 falling_timeout = 0;
Uint32 next_move_timeout = 0;
Uint32 lock_timeout = 0;

int start_locking_timer = 1;

char score[] = {"000000000"};
int update_score = TRUE;

float falling_modifier = 1.0f;  // x1
int lines_to_next_level = 0;

char levels[] = {"000"};
int update_level = TRUE;

// GAME CUSTOM FUNCTIONS
void spawn_piece();
void move();
void shift(int);
int is_fallen();
void save_pieces();
int is_near_right_border();
int is_near_left_border();
int hindered(char);
void create_piece();
void rotate();
void line_clear();
void compact(int);
int end_game();
void increase_score(int);
void load_texture(char *, SDL_Surface **, SDL_Texture **, SDL_Renderer **);
void increase_level();

#endif //TETRIS_GAMEFUNCTIONS_H


/*
 * ------------------------|SPAWN_PIECE|------------------------
 *  Date of Creation:
 *  18/11/2024
 *  Modified:
 *  24/11/2024
 *  Parameters:
 *
 *  Description:
 *  It is used to place randomly a piece one the matrix of the g
 *  -ame dynamic_field after the RESPAWN_RATE time.
 */
void spawn_piece(){
    // CHOSE THE TYPE OF PIECE TO PLACE
    piece_num = rand()%((7) - (1) + 1) + (1); // TO GENERATE A NUMBER BETWEEN 1 AND 7

    int spawn_point1_x = (NCOL - 1);
    int spawn_point2_x = 0;

    // CLEAR THE TEMPORARY MATRIX
    for (int i = 0; i < NROW_TEMP; ++i) {
        for (int j = 0; j < NCOL_TEMP; ++j) {
            piece_matrix[i][j] = 0;
        }
    }

    create_piece();

    row = 0;

    do {
        if(piece_num == 1){ // I PIECE EXCEPTION
            col = rand() % ((spawn_point1_x - 5) - (spawn_point2_x) + 1) + (spawn_point2_x);
        }else if(piece_num == 4) { // O PIECE EXCEPTION
            col = rand() % ((spawn_point1_x - 3) - (spawn_point2_x) + 1) + (spawn_point2_x);
        }else{
            col = rand() % ((spawn_point1_x - 2) - (spawn_point2_x) + 1) + (spawn_point2_x);
        }
    } while (static_field[row][col] == 1);  // USED TO RECALCULATE THE COL WHEN THERE ARE PIECES HINDERING THE SPAWN ZONE


    for (int i = 0; i < NROW_TEMP; ++i) {
        for (int j = 0; j < NCOL_TEMP; ++j) {
            dynamic_field[row+i][col+j] = piece_matrix[i][j];
        }
    }
    update_dynamic_sprite = TRUE;
}

/*
 * -------------------------|CREATE_PIECE|-------------------------
 *  Date of Creation:
 *  23/11/2024
 *  Modified:
 *  24/11/2024
 *  Parameters:
 *
 *  Description:
 *  It is used to read and copy the pattern of a piece in a matrix.
 */
void create_piece(){

    int w,h;

    FILE *fp = NULL;
    char *line[4], *number, filename[50];;

    h = 0;

    // VALIDATE ROTATION
    if (rotation < 1 || rotation > 4) {
        printf("Invalid rotation: %d\n", rotation);
    }

    switch (piece_num) {
        case 1: snprintf(filename, sizeof(filename), "Pieces/I_piece_%d.txt", rotation); break;
        case 2: snprintf(filename, sizeof(filename), "Pieces/J_piece_%d.txt", rotation); break;
        case 3: snprintf(filename, sizeof(filename), "Pieces/L_piece_%d.txt", rotation); break;
        case 4: snprintf(filename, sizeof(filename), "Pieces/O_piece.txt"); break; // NO ROTATION
        case 5: snprintf(filename, sizeof(filename), "Pieces/S_piece_%d.txt", rotation); break;
        case 6: snprintf(filename, sizeof(filename), "Pieces/T_piece_%d.txt", rotation); break;
        case 7: snprintf(filename, sizeof(filename), "Pieces/Z_piece_%d.txt", rotation); break;
        default:
            printf("Piece not found for piece_num: %d\n", piece_num);
    }

    fp = fopen(filename, "r");
    if (!fp) {
        printf("Error opening file: %s\n", filename);
    }

    while(!feof(fp)){
        w = 0;
        fscanf(fp, "%s", line);
        number = strtok(line,",");
        while(number != NULL){
            piece_matrix[h][w] = atoi(number);
            w++;
            number = strtok(NULL, ",");
        }
        h++;
    }
    fclose(fp);
}

/*
 * -------------------------|ROTATE|-------------------------
 *  Date of Creation:
 *  24/11/2024
 *  Modified:
 *  27/11/2024
 *  Parameters:
 *
 *  Description:
 *  It is used to pick a piece based on the rotation and then
 *  paste it in the dynamic field.
 *  There are also calculated the offsets for specific cases.
 */
void rotate() {
    int length, skip = FALSE;

    create_piece();

    // I PIECE OFFSETS
    if (piece_num == 1) {
        if (col + 2 == 0) {
            col++;
        } else if (col + 2 == NCOL - 1) {
            col--;
        } else if (col + 1 == NCOL - 1) {
            col -= 2;
        }else if(row + 1 == NROW-1){
            row -= 2;
        }else if(row + 2 == NROW-1){
            row--;
        }
    }
    // GENERAL OFFSETS (except O piece)
    if(piece_num != 4) {
        if (col + 1 == 0) {
            col++;
        } else if (col + 1 == NCOL - 1) {
            col--;
        } else if (row + 1 == NROW - 1) {
            row--;
        }
    }

    length = sizeof(piece_matrix) / sizeof(piece_matrix[0]);

    // CHECK IF THE ROTATION IS POSSIBLE
    for (int i = 0; i < length; ++i) {
        for (int j = 0; j < length; ++j) {
            if (piece_matrix[i][j] == 1 && static_field[row + i][col + j] == 1){
                skip = TRUE;
            }
        }
    }

    if(!skip){
        // PASTE THE PIECE ROTATED
        for (int i = 0; i < NROW_TEMP; ++i) {
            for (int j = 0; j < NCOL_TEMP; ++j) {
                dynamic_field[row + i][col + j] = piece_matrix[i][j];
            }
        }
    }else{
        rotation--; // CANCELING THE ROTATION
    }
}

/*
 * ------------------------|MOVE|------------------------
 *  Date of Creation:
 *  17/11/2024
 *  Parameters:
 *
 *  Description:
 *  It is used to update the position of a struct, based
 *  on the key pressed.
 */
void move(){
    switch (key_sym) {
        case SDLK_w:    // ROTATE
            rotation++;
            if (rotation > MAX_ROTATIONS)
                rotation = 1;
            rotate();
            break;
        case SDLK_s:    // MOVE DOWN
            if (!fell && row != (NROW-1)){
                increase_score(1);
                shift(0);
                row++;
            }
            break;
        case SDLK_a:    // MOVE LEFT
            if(!is_near_left_border() && !hindered('l')){
                shift(3);
                col--;
            }
            break;
        case SDLK_d:    //MOVE RIGHT
            if(!is_near_right_border() && !hindered('r')) {
                shift(2);
                col++;
            }
            break;
        default:
            break;
    }
}

/*
 * ---------------------------|SHIFT|---------------------------
 *  Date of Creation:
 *  20/11/2024
 *  Parameters:
 *  - n:int = it is the kind of shift done to the matrix.
 *  Description:
 *  It is used to shift in various ways the matrix of the falling pieces.
 *  Kind of shifts:
 *  - 0 = downward shift
 *  - 1 = upward shift
 *  - 2 = right shift
 *  - 3 = left shift
 */
void shift(int n){
    switch (n) {
        case 0: // DOWNWARD SHIFT
            for (int i = NROW-1; i >= 0 ; --i) {
                for (int j = 0; j < NCOL; ++j) {
                    dynamic_field[i][j] = dynamic_field[i - 1][j];
                }
            }
            break;
        case 1: // UPWARD SHIFT
            for (int i = 0; i < NROW ; ++i) {
                for (int j = 0; j < NCOL; ++j) {
                    dynamic_field[i][j] = dynamic_field[i + 1][j];
                }
            }
            break;
        case 2: // RIGHT SHIFT
            for (int i = 0; i < NROW ; ++i) {
                for (int j = NCOL-1; j >= 0; --j) {
                    if (j == 0)
                        dynamic_field[i][j] = 0;
                    else
                        dynamic_field[i][j] = dynamic_field[i][j - 1];
                }
            }
            break;
        case 3: // LEFT SHIFT
            for (int i = 0; i < NROW ; ++i) {
                for (int j = 0; j < NCOL; ++j) {
                    if (j == NCOL-1)
                        dynamic_field[i][j] = 0;
                    else
                        dynamic_field[i][j] = dynamic_field[i][j + 1];
                }
            }
            break;
        default:        // ERROR MSG
            printf("Shift not possible.\n");
            break;
    }
}

/*
 * -------------------------|IS_FALLEN|-------------------------
 *  Date of Creation:
 *  20/11/2024
 *  Modified:
 *  26/11/2024
 *  Parameters:
 *
 *  Description:
 *  It is used to check if a piece is fallen to the bottom or on
 *  to another block.
 */
int is_fallen(){

    // SAVE THE PIECES
    for (int i = 0; i < NROW; ++i) {
        for (int j = 0; j < NCOL; ++j) {
            if (dynamic_field[NROW-1][j] == 1 || (dynamic_field[i][j] == 1 && static_field[i+1][j] == 1)){
                return TRUE;
            }
        }
    }

    return FALSE;
}

/*
 * -------------------------|SAVE_PIECES|-------------------------
 *  Date of Creation:
 *  20/11/2024
 *  Parameters:
 *
 *  Description:
 *  It is used to place the pieces of the dynamic field on the sta-
 *  -tic field.
 */
void save_pieces(){
    for (int i = 0; i < NROW; ++i) {
        for (int j = 0; j < NCOL; ++j) {
            if (static_field[i][j] != 1)
                static_field[i][j] = dynamic_field[i][j];
        }
    }
    update_static_sprite = TRUE;
}

/*
 * -------------------------|IS_NEAR_RIGHT_BORDER|-------------------------
 *  Date of Creation:
 *  20/11/2024
 *  Parameters:
 *
 *  Description:
 *  It is used to check if a piece is near the right border.
 */
int is_near_right_border(){
    for (int i = NROW-1; i >= 0; --i) {
        if(dynamic_field[i][NCOL-1] == 1)
            return TRUE;
    }
    return FALSE;
}

/*
 * -------------------------|IS_NEAR_LEFT_BORDER|-------------------------
 *  Date of Creation:
 *  20/11/2024
 *  Parameters:
 *
 *  Description:
 *  It is used to check if a piece is near the left border.
 */
int is_near_left_border(){
    for (int i = 0; i < NROW; ++i) {
        if(dynamic_field[i][0] == 1)
            return TRUE;
    }
    return FALSE;
}

/*
 * -------------------------|HINDERED|-------------------------
 *  Date of Creation:
 *  21/11/2024
 *  Parameters:
 *  - dir:char = it is used to specify the direction controlled
 *  Description:
 *  It is used to check if there are pieces hindering the dire-
 *  -ction of a falling piece, to its right or left.
 */
int hindered(char dir){
    for (int i = 0; i < NROW; ++i) {
        for (int j = 0; j < NCOL; ++j) {
            // LEFT HINDRANCE CHECK
            if (dir == 'l'
                && dynamic_field[i][j] == 1
                && static_field[i][j-1] == 1){
                return 1;
            }
            // RIGHT HINDRANCE CHECK
            if (dir == 'r'
                && dynamic_field[i][j] == 1
                && static_field[i][j+1] == 1){
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*
 * -------------------------|LINE_CLEAR|-------------------------
 *  Date of Creation:
 *  03/12/2024
 *  Parameters:

 *  Description:
 *  It is used to clear the lines with all 1 in the static field.
 */
void line_clear(){
    int clear;
    for (int i = 0; i < NROW; ++i) {
        // CHECK IF THE LINE CAN BE CLEANED
        clear = 1;
        for (int j = 0; j < NCOL; ++j) {
            if (static_field[i][j] != 1)
                clear = 0;
        }
        // CLEAN THE LINE
        if(clear) {
            for (int j = 0; j < NCOL; ++j) {
                static_field[i][j] = 0;
            }
            compact(i);
            increase_score(100);
            lines_to_next_level++;
            increase_level();
        }
    }
}

/*
 * -------------------------|COMPACT|-------------------------
 *  Date of Creation:
 *  26/11/2024
 *  Parameters:
 *  line:int = It contains the line where to end the shift.
 *  Description:
 *  It is used to compact all the lines, by partially shifting
 *  downwards.
 */
void compact(int line){
    // PARTIAL SHIFT TO COMPACT ALL THE LINES
    for (int i = line; i >= 0 ; --i) {
        for (int j = 0; j < NCOL; ++j) {
            static_field[i][j] = static_field[i - 1][j];
        }
    }
}

/*
 * -------------------------|END_GAME|-------------------------
 *  Date of Creation:
 *  27/11/2024
 *  Parameters:
 *
 *  Description:
 *  It is used to control if the player placed a piece in the
 *  first row of the static field.
 *  If the case, then it end the game.
 */
int end_game(){
    for (int i = 0; i < NCOL; ++i) {
        if (static_field[0][i] == 1)
            return TRUE;
    }
    return FALSE;
}

/*
 * --------------------------|INCREASE_SCORE|--------------------------
 *  Date of Creation:
 *  28/11/2024
 *  Parameters:
 *  - points:int = It is used to store the points which will be added.
 *  Description:
 *  It is used to increase the score of a certain amount of points.
 */
void increase_score(int points){
    int lenght = sizeof(score)/sizeof(score[0]);
    char string[lenght];
    for (int i = 0; i < lenght; ++i) {
        string[i] = '\000';
    }
    int temp_score = atoi(score);
    temp_score += points;
    itoa(temp_score,string,10);

    int j = lenght-2;
    for (int i = lenght-1; i >= 0 ; --i) {
        if ((int)string[i] > 0) {
            score[j] = string[i];
            j--;
        }
    }
    update_score = TRUE;
}

/*
 * --------------------------|INCREASE_LEVEL|--------------------------
 *  Date of Creation:
 *  03/12/2024
 *  Parameters:
 *  Description:
 *  It is used to increase the level of 1.
 */
void increase_level(){
    int lenght = sizeof(levels)/sizeof(levels[0]);
    char string[lenght];
    for (int i = 0; i < lenght; ++i) {
        string[i] = '\000';
    }
    int temp_score = atoi(levels);
    temp_score += 1;
    itoa(temp_score,string,10);

    int j = lenght-2;
    for (int i = lenght-1; i >= 0 ; --i) {
        if ((int)string[i] > 0) {
            levels[j] = string[i];
            j--;
        }
    }
    update_level = TRUE;
}

/*
 * --------------------------|LOAD_TEXTURE|--------------------------
 *  Date of Creation:
 *  30/11/2024
 *  Parameters:
 *
 *  Description:
 *  It is used to upload the texture_sc of a bitmap image.
 */
void load_texture(char filename[], SDL_Surface **bitmapSurface, SDL_Texture **texture, SDL_Renderer **renderer){
    // LOAD BITMAP IMAGE
    *bitmapSurface = SDL_LoadBMP(filename);
    if (!*bitmapSurface) {
        printf("Unable to load bitmap! SDL_Error: %s\n", SDL_GetError());
    }

    // CREATE A TEXTURE FORM THE SURFACE
    *texture = SDL_CreateTextureFromSurface(*renderer, *bitmapSurface);
    SDL_FreeSurface(*bitmapSurface); // Surface no longer needed
    if (!*texture) {
        printf("Unable to create texture_sc! SDL_Error: %s\n", SDL_GetError());
    }
}


