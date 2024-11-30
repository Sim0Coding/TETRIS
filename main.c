#include <stdio.h>
#include <SDL.h>
#include <time.h>
#include "constants.h"
#include "gameFunctions.h"


// STANDARDS VARIABLES
int game_is_running = FALSE;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
int last_frame_time = 0;

// SURFACES & TEXTURE
SDL_Texture *texture[NTEXTURE] = {NULL};
SDL_Surface *bitmapSurface[NTEXTURE] = {NULL};
SDL_Texture *texture_dyn_piece = NULL;
SDL_Texture *texture_stc_piece = NULL;
SDL_Surface *bitmapSurface_piece = NULL;
SDL_Texture *texture_bg = NULL;
SDL_Surface *bitmapSurface_preload = NULL;
SDL_Texture *texture_pause = NULL;


// OBJECTS
struct block {
    float x;
    float y;
    float width;
    float height;
}block;

struct boundary {
    float x;
    float y;
    float width;
    float height;
}boundary;

struct score_bg {
    float x;
    float y;
    float width;
    float height;
}score_bg;

struct pause {
    float x;
    float y;
    float width;
    float height;
}pause;

// STANDARDS FUNCTIONS
int initialize_window(void);
void setup();
void process_input();
void update();
void render();
void destroy_window();


// PROGRAM
int main(int argc, char* argv[]) {
    game_is_running = initialize_window();

    setup();

    while(game_is_running) {
        process_input();
        if (game_is_running != PAUSE)
            update();
        render();
    }

    destroy_window();

    return 0;
}

/*
 * ------------------------|INITIALIZE_WINDOW|------------------------
 *  Date of Creation:
 *  17/11/2024
 *  Parameters:
 *
 *  Description:
 *  It is executed one time at the start of the program.
 *  It is used to create and render the window of the program once.
 *  It is also used to check the statue of the initialization of game.
 */
int initialize_window(){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) { // INITIALIZE ALL OF SDL STUFF LIKE (GRAPHIC, KEYBOARD INPUT, ...)
        fprintf(stderr,"Error initializing SDL.\n");
        return FALSE;
    }
    window = SDL_CreateWindow(             // CREATION OF A WINDOWS
            "TETRIS",                     // TITLE
            SDL_WINDOWPOS_CENTERED,     // X COORDINATES
            SDL_WINDOWPOS_CENTERED,     // Y COORDINATES
            WINDOW_WIDTH,               // WIDTH
            WINDOW_HEIGHT,              // HEIGHT
            0
            //SDL_WINDOW_BORDERLESS     // WINDOW CHARACTERISTICS
    );
    if(!window){
        fprintf(stderr,"Error initializing SDL.\n");
        return FALSE;
    }
    renderer = SDL_CreateRenderer(          // SHOW THE WINDOW
            window,                         // WINDOW
            -1,                       // DISPLAY DRIVER [-1 is the default]
            0                         // SPECIAL CHARACTERISTICS
    );
    if(!renderer){
        fprintf(stderr,"Error initializing SDL.\n");
        return FALSE;
    }

    return TRUE;
}

/*
 * -----------------------|SETUP|-----------------------
 *  Date of Creation:
 *  17/11/2024
 *  Parameters:
 *
 *  Description:
 *  It is executed one time at the start of the program.
 *  All the things which are in the game will be put th-
 *  -ere and then updated and executed later in the upd-
 *  -ate method.
 */
void setup(){
    srand(time(NULL));  // RANDOMIZE THE SEED OF RAND

    // SETTING TO 0 THE MATRIX
    for (int i = 0; i < NROW; ++i) {
        for (int j = 0; j < NCOL; ++j) {
            dynamic_field[i][j] = 0;
        }
    }

    // SETTING THE PROPRIETIES OF THE STRUCTS
    boundary.width = BLOCK_SIZE * NCOL + 4;
    boundary.height = BLOCK_SIZE * NROW + 4;
    boundary.x = (WINDOW_WIDTH/2)-(boundary.width/2);
    boundary.y = (WINDOW_HEIGHT/2)-(boundary.height/2);

    block.x = boundary.x;
    block.y = boundary.y;
    block.width = BLOCK_COLOR_SIZE;
    block.height = BLOCK_COLOR_SIZE;

    score_bg.width = (BLOCK_SIZE/2)*9;  // 16px * 9 --> SCORE: 999.999.999
    score_bg.height = (BLOCK_SIZE/2) + 4;   // 16px + 4 px for padding
    score_bg.x = (WINDOW_WIDTH/2)+(boundary.width/2)+((WINDOW_WIDTH/2-boundary.width/2)/2)-score_bg.width/2;
    score_bg.y = (WINDOW_HEIGHT/2)-(score_bg.height/2);

    pause.width = 300;          // 300 px
    pause.height = 100;         // 100 px
    pause.x = boundary.x + 10;  // 10 px of padding
    pause.y = (WINDOW_HEIGHT/2)-(pause.height/2);

    // TIMERS SETUP
    falling_timeout = SDL_GetTicks() + FALLING_TIME;
    next_move_timeout = SDL_GetTicks() + NEXT_MOVE_TIME;


    // UPLOAD OF THE BACKGROUND BITMAP/IMAGE
    char filename_preload[50];

    strcpy(filename_preload,"ASSETS/Sprite_Bg.bmp");

    // LOAD BITMAP IMAGE
    bitmapSurface_preload = SDL_LoadBMP(filename_preload);
    if (!bitmapSurface_preload) {
        printf("Unable to load bitmap! SDL_Error: %s\n", SDL_GetError());
    }

    // CREATE A TEXTURE FORM THE SURFACE
    texture_bg = SDL_CreateTextureFromSurface(renderer, bitmapSurface_preload);
    SDL_FreeSurface(bitmapSurface_preload); // Surface no longer needed
    if (!texture_bg) {
        printf("Unable to create texture! SDL_Error: %s\n", SDL_GetError());
    }

    // UPLOAD OF THE PAUSE BITMAP/IMAGE
    strcpy(filename_preload,"ASSETS/Sprite_Pause.bmp");

    // LOAD BITMAP IMAGE
    bitmapSurface_preload = SDL_LoadBMP(filename_preload);
    if (!bitmapSurface_preload) {
        printf("Unable to load bitmap! SDL_Error: %s\n", SDL_GetError());
    }

    // CREATE A TEXTURE FORM THE SURFACE
    texture_pause = SDL_CreateTextureFromSurface(renderer, bitmapSurface_preload);
    SDL_FreeSurface(bitmapSurface_preload); // Surface no longer needed
    if (!texture_pause) {
        printf("Unable to create texture! SDL_Error: %s\n", SDL_GetError());
    }


}

/*
 * -----------------------|PROCESS_INPUT|-----------------------
 *  Date of Creation:
 *  17/11/2024
 *  Parameters:
 *
 *  Description:
 *  It is executed until the variable game_is_running is 1.
 *  There will be handled the events, which could be key pressed,
 *  mouse inputs or even other inputs.
 */
void process_input(){
    SDL_Event event;
    SDL_PollEvent(&event); // PROCESS INPUT PUTTING IT IN event

    switch (event.type) {
        case SDL_QUIT:  // EVENT WHEN YOU CLICK THE X BUTTON
            game_is_running = FALSE;
            break;
        case SDL_KEYDOWN: // EVENT WHEN YOU CLICK A KEY ON THE KEYBOARD
            if (event.key.keysym.sym == SDLK_ESCAPE) // key pressed down == esc
                game_is_running = PAUSE * game_is_running;
            else if (event.key.keysym.sym == SDLK_w
                     || event.key.keysym.sym == SDLK_s
                     || event.key.keysym.sym == SDLK_d
                     || event.key.keysym.sym == SDLK_a)
                key_sym = event.key.keysym.sym;
            break;
        case SDL_KEYUP:
            key_sym_pre = key_sym;
            key_sym = 0;
            break;
    }

}

/*
 * ------------------------|UPDATE|------------------------
 *  Date of Creation:
 *  17/11/2024
 *  Modified:
 *  30/11/2024
 *  Parameters:
 *
 *  Description:
 *  It is executed until the variable game_is_running is 1.
 *  In this function is implemented the cap of frames shown
 *  in a second, so the interval of time between every upd-
 *  -ate and render. Also, the delta time which is the fac-
 *  -tor with pixel can change position in a second.
 *  Whatever there is in this function is meant to update
 *  something, for example the position of an object.
 */
void update(){
    // CAPPING THE FRAMERATE
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME){
        SDL_Delay(time_to_wait);
    }

    last_frame_time = SDL_GetTicks();

    // SPAWN OF PIECES
    if(placed){
        placed = 0;
        for (int i = 0; i < NROW; ++i) {
            for (int j = 0; j < NCOL; ++j) {
                dynamic_field[i][j] = 0;
            }
        }
        // SPAWN A NEW PIECE
        spawn_piece();
    }

    // CLEAR THE LINES WITH ALL 1
    line_clear();


    // CHECK IF THE PIECE HAS FALLEN
    fell = is_fallen();

    // START THE TIMER TO LOCK A FALLEN PIECE
    if (fell && start_locking_timer) {
        start_locking_timer = 0;
        lock_timeout = SDL_GetTicks() + LOCKING_TIME;
    }

    // PLACE THE PIECE ON THE STATIC FIELD
    if (SDL_TICKS_PASSED(SDL_GetTicks(), lock_timeout) && fell) {
        save_pieces();
        placed = 1;
        start_locking_timer = 1;
        rotation = 1;
    }

    // MAKES FALL PIECES EVERY 1s
    if (SDL_TICKS_PASSED(SDL_GetTicks(), falling_timeout) && !fell) {
        falling_timeout = SDL_GetTicks() + FALLING_TIME;
        shift(0);
        row++;
    }

    // TOGGLED ROTATION
    if (key_sym == 119){    // KEY w
        if(key_sym_pre != key_sym)
            move();
    // HOLD MOVEMENTS
    }else{                  // OTHER KEYS
        if (SDL_TICKS_PASSED(SDL_GetTicks(), next_move_timeout) && !placed){
            next_move_timeout = SDL_GetTicks() + NEXT_MOVE_TIME;
            move();
        }
    }
    key_sym_pre = key_sym;

    // END THE GAME IF THERE IS A PIECE IN THE FIRST ROW
    if (end_game())
        game_is_running = FALSE;
}

/*
 * ------------------------|RENDER|------------------------
 *  Date of Creation:
 *  17/11/2024
 *  Modified:
 *  30/11/2024
 *  Parameters:
 *
 *  Description:
 *  It is executed until the variable game_is_running is 1.
 *  Whatever there is in this function is meant to be drawn
 *  on a renderer every frame.
 */
void render(){ // CAN BE CALLED ALSO draw()
    //--------------------BASE_BACKGROUND--------------------
    SDL_SetRenderDrawColor(renderer, 0,0,0,255); // SET THE COLOR
    SDL_RenderClear(renderer);

    //--------------------IMAGE_BACKGROUND--------------------

    SDL_RenderCopy(renderer, texture_bg, NULL, NULL);

    //--------------------FIELD_BACKGROUND--------------------
    SDL_Rect boundary_rect = {
            (int)boundary.x,
            (int)boundary.y,
            (int)boundary.width,  // VISUAL ADJUSTMENTS FOR THE width OF THE MARGIN
            (int)boundary.height  // VISUAL ADJUSTMENTS FOR THE height OF THE MARGIN
    };

    boundary_rect.x -= 3; // VISUAL ADJUSTMENTS FOR THE x OF THE MARGIN
    boundary_rect.y -= 3; // VISUAL ADJUSTMENTS FOR THE y OF THE MARGIN
    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_RenderFillRect(renderer,&boundary_rect);

    //--------------------FIELD_MARGINS--------------------
    SDL_SetRenderDrawColor(renderer,255,255,255,255);
    SDL_RenderDrawRect(renderer,&boundary_rect);

    //--------------------COMMANDS_IMAGE--------------------


    if (game_is_running != PAUSE){
        //--------------------GAME_FIELD--------------------
        SDL_Rect block_rect = {
                (int) block.x,
                (int) block.y,
                (int) block.width,
                (int) block.height
        };

        char filename_piece[50];

        // RENDER DYNAMIC FIELD PIECES
        for (int i = 0; i < NROW; ++i) {
            for (int j = 0; j < NCOL; ++j) {

                snprintf(filename_piece, sizeof(filename_piece), "ASSETS/Sprite_%d.bmp", piece_num);

                if (update_dynamic_sprite) {
                    // LOAD BITMAP IMAGE
                    bitmapSurface_piece = SDL_LoadBMP(filename_piece);
                    if (!bitmapSurface_piece) {
                        printf("Unable to load bitmap! SDL_Error: %s\n", SDL_GetError());
                    }

                    // CREATE A TEXTURE FORM THE SURFACE
                    texture_dyn_piece = SDL_CreateTextureFromSurface(renderer, bitmapSurface_piece);
                    SDL_FreeSurface(bitmapSurface_piece); // Surface no longer needed
                    if (!texture_dyn_piece) {
                        printf("Unable to create texture! SDL_Error: %s\n", SDL_GetError());
                    }
                    update_dynamic_sprite = FALSE;
                }

                if (dynamic_field[i][j] == 1) {
                    // COPY THE TEXTURE TO RENDER
                    SDL_RenderCopy(renderer, texture_dyn_piece, NULL, &block_rect);
                }
                block_rect.x += BLOCK_SIZE;
            }
            // UPDATE THE COORDINATES
            block_rect.x = (int) boundary.x;
            block_rect.y += BLOCK_SIZE;
        }
        // RESET THE COORDINATE
        block_rect.x = (int) block.x;
        block_rect.y = (int) block.y;

        // RENDER STATIC FIELD PIECES

        for (int i = 0; i < NROW; ++i) {
            for (int j = 0; j < NCOL; ++j) {

                if (update_static_sprite) {

                    strcpy(filename_piece, "ASSETS/Sprite_Base.bmp");

                    // LOAD BITMAP IMAGE
                    bitmapSurface_piece = SDL_LoadBMP(filename_piece);
                    if (!bitmapSurface_piece) {
                        printf("Unable to load bitmap! SDL_Error: %s\n", SDL_GetError());
                    }

                    // CREATE A TEXTURE FORM THE SURFACE
                    texture_stc_piece = SDL_CreateTextureFromSurface(renderer, bitmapSurface_piece);
                    SDL_FreeSurface(bitmapSurface_piece); // Surface no longer needed
                    if (!texture_stc_piece) {
                        printf("Unable to create texture! SDL_Error: %s\n", SDL_GetError());
                    }
                    update_static_sprite = FALSE;
                }

                if (static_field[i][j] == 1) {
                    // COPY THE TEXTURE TO RENDER
                    SDL_RenderCopy(renderer, texture_stc_piece, NULL, &block_rect);
                }
                block_rect.x += BLOCK_SIZE;
            }
            // UPDATE THE COORDINATES
            block_rect.x = (int) boundary.x;
            block_rect.y += BLOCK_SIZE;
        }
    }else{
        SDL_Rect pause_dest = {
                (int)pause.x,
                (int)pause.y,
                (int)pause.width,
                (int)pause.height
        };
        SDL_RenderCopy(renderer, texture_pause, NULL, &pause_dest);
    }

    //--------------------SCORE_BACKGROUND--------------------
    SDL_Rect score_bg_rect = {
            (int)score_bg.x,
            (int)score_bg.y,
            (int)score_bg.width,
            (int)score_bg.height
    };
    SDL_SetRenderDrawColor(renderer,255,255,255,255);
    SDL_RenderFillRect(renderer,&score_bg_rect);

    //--------------------SCORE--------------------
    SDL_Rect dest = {
            (int)score_bg.x,
            (int)score_bg.y + 2,
            16,
            16
    };
    int length = sizeof(score) / sizeof(score[0]);
    char filename_char[50];

    for (int i = 0; i < length; ++i) {
        if ((int) score[i] >= '0' && (int) score[i] <= '9')
            snprintf(filename_char, sizeof(filename_char), "BMP_FONT/Number_%c.bmp", score[i]);
        else
            snprintf(filename_char, sizeof(filename_char), "BMP_FONT/Letter_%c.bmp", score[i]);

        if (update_score) {
            // LOAD BITMAP IMAGE
            bitmapSurface[i] = SDL_LoadBMP(filename_char);
            if (!bitmapSurface[i]) {
                printf("Unable to load bitmap! SDL_Error: %s\n", SDL_GetError());
            }

            // CREATE A TEXTURE FORM THE SURFACE
            texture[i] = SDL_CreateTextureFromSurface(renderer, bitmapSurface[i]);
            SDL_FreeSurface(bitmapSurface[i]); // Surface no longer needed
            if (!texture[i]) {
                printf("Unable to create texture! SDL_Error: %s\n", SDL_GetError());
            }
        }

        // COPY THE TEXTURE TO RENDER
        SDL_RenderCopy(renderer, texture[i], NULL, &dest);

        // NEXT POSITION
        dest.x += (BLOCK_SIZE / 2);
    }
    update_score = FALSE;

    SDL_RenderPresent(renderer); // SWAPPING FORM BACK TO FRONT BUFFER TO PREVENT FLICKERING FRAMES
}

/*
 * ------------------------|DESTROY_WINDOW|------------------------
 *  Date of Creation:
 *  17/11/2024
 *  Parameters:
 *
 *  Description:
 *  It is executed one time at the start of the program.
 *  It is used to destroy in order: the renderer, the window and
 *  then quit the external library SDL.
 */
void destroy_window(){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
