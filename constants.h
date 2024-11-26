//
// Created by Simone on 17/11/2024.
//

#ifndef GAME1_CONSTANTS_H
#define GAME1_CONSTANTS_H

#define FALSE 0
#define TRUE 1

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 800

#define FPS 144
#define FRAME_TARGET_TIME (1000 / FPS)

#define SDL_TICKS_PASSED(A, B)  ((Sint32)((B) - (A)) <= 0) // RETURN 0 = FALSE OR 1 = TRUE, IF A PASSED B

#define BLOCK_COLOR_SIZE 30
#define BLOCK_SIZE 32
#define NCOL 10
#define NROW 22
#define NCOL_TEMP 4
#define NROW_TEMP 4
#define MAX_ROTATIONS 4

#define FALLING_TIME 1000   // 1s
#define NEXT_MOVE_TIME 50   // 0.05s
#define LOCKING_TIME 1500   // 1.5s

#endif //GAME1_CONSTANTS_H
