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

// SURFACES
SDL_Surface *bitmapSurface[NTEXTURESC] = {NULL};
SDL_Surface *bitmapSurface_piece = NULL;
SDL_Surface *bitmapSurface_preload = NULL;

// TEXTURE
SDL_Texture *texture_sc[NTEXTURESC] = {NULL};
SDL_Texture *texture_lv[NTEXTURELV] = {NULL};
SDL_Texture *texture_stc_piece = NULL;
SDL_Texture *texture_dyn_piece = NULL;
SDL_Texture *texture_bg = NULL;
SDL_Texture *texture_pause = NULL;
SDL_Texture *texture_controls = NULL;
SDL_Texture *texture_score = NULL;

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

struct s_score {
    float x;
    float y;
    float width;
    float height;
}s_score;

struct pause {
    float x;
    float y;
    float width;
    float height;
}pause;

struct controls {
    float x;
    float y;
    float width;
    float height;
}controls;

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

    // LOAD THE ICON IMAGE OF THE WINDOW
    char filename_Icon[] = {"ASSETS/Sprite_Window_Icon.bmp"};
    bitmapSurface_preload = SDL_LoadBMP(filename_Icon);
    if (!bitmapSurface_preload) {
        printf("Unable to load bitmap! SDL_Error: %s\n", SDL_GetError());
    }
    SDL_SetWindowIcon(window,bitmapSurface_preload);
    SDL_FreeSurface(bitmapSurface_preload); // Surface no longer needed

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

    s_score.width = 318;        // 318 px
    s_score.height = 400;       // 400 px
    s_score.x = (WINDOW_WIDTH/2)+(boundary.width/2)+((WINDOW_WIDTH/2-boundary.width/2)/2)-s_score.width/2;
    s_score.y = (WINDOW_HEIGHT/2)-(s_score.height/2);

    pause.width = 300;          // 300 px
    pause.height = 100;         // 100 px
    pause.x = boundary.x + 10;  // 10 px of padding
    pause.y = (WINDOW_HEIGHT/2)-(pause.height/2);

    controls.width = 294;          // 200 px
    controls.height = 400;         // 272 px
    controls.x = ((((WINDOW_WIDTH - boundary.width)/2) - controls.width)/2) ;  //
    controls.y = (WINDOW_HEIGHT/2)-(controls.height/2);

    // TIMERS SETUP
    falling_timeout = SDL_GetTicks() + FALLING_TIME;
    next_move_timeout = SDL_GetTicks() + NEXT_MOVE_TIME;


    // UPLOAD OF THE BACKGROUND BITMAP/IMAGE
    load_texture("ASSETS/Sprite_Bg.bmp", &bitmapSurface_preload, &texture_bg,&renderer);

    // UPLOAD OF THE PAUSE BITMAP/IMAGE
    load_texture("ASSETS/Sprite_Pause.bmp", &bitmapSurface_preload, &texture_pause,&renderer);

    // UPLOAD OF THE CONTROLS BITMAP/IMAGE
    load_texture("ASSETS/Sprite_Controls.bmp", &bitmapSurface_preload, &texture_controls,&renderer);

    // UPLOAD OF THE CONTROLS BITMAP/IMAGE
    load_texture("ASSETS/Sprite_Score&Level.bmp", &bitmapSurface_preload, &texture_score,&renderer);

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

    // DECREASE THE FALLING MODIFIER MAKING THE PIECE FALL FASTER
    if(NEXT_LEVEL(lines_to_next_level) && falling_modifier > MAX_FALL_MOD) {
        falling_modifier -= 0.05f;
        lines_to_next_level = 0;
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
        falling_timeout = SDL_GetTicks() + (int)(FALLING_TIME * falling_modifier);
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
 *  06/11/2024
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
    SDL_Rect controls_dest = {
            (int)controls.x,
            (int)controls.y,
            (int)controls.width,
            (int)controls.height
    };

    SDL_RenderCopy(renderer, texture_controls, NULL, &controls_dest);

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

                load_texture(filename_piece, &bitmapSurface_piece, &texture_dyn_piece,&renderer);
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

                load_texture("ASSETS/Sprite_Base.bmp", &bitmapSurface_piece, &texture_stc_piece,&renderer);
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

    //--------------------PAUSE_TEXT--------------------
    if (game_is_running == PAUSE){
        SDL_Rect pause_dest = {
                (int)pause.x,
                (int)pause.y,
                (int)pause.width,
                (int)pause.height
        };
        SDL_RenderCopy(renderer, texture_pause, NULL, &pause_dest);
    }

    //--------------------SCORE_SPRITE--------------------
    SDL_Rect s_score_dest = {
            (int)s_score.x,
            (int)s_score.y,
            (int)s_score.width,
            (int)s_score.height,
    };

    SDL_RenderCopy(renderer, texture_score, NULL, &s_score_dest);

    //--------------------SCORE--------------------
    SDL_Rect score_dest = {
            (int)s_score.x + 15,
            (int)s_score.y + 120,
            BLOCK_SIZE,
            BLOCK_SIZE*2
    };
    int length_sc = sizeof(score) / sizeof(score[0]);
    char filename_char_sc[50];

    for (int i = 0; i < length_sc; ++i) {
        snprintf(filename_char_sc, sizeof(filename_char_sc), "BMP_FONT/Number_%c.bmp", score[i]);

        if (update_score) {
            load_texture(filename_char_sc, &bitmapSurface[i], &texture_sc[i], &renderer);
        }

        // COPY THE TEXTURE TO RENDER
        SDL_RenderCopy(renderer, texture_sc[i], NULL, &score_dest);

        // NEXT POSITION
        score_dest.x += (BLOCK_SIZE);
    }
    update_score = FALSE;

    //--------------------LEVELS--------------------
    SDL_Rect levels_dest = {
            (int)s_score.x + 111,
            (int)s_score.y + 285,
            BLOCK_SIZE,
            BLOCK_SIZE*2
    };
    int length_lv = sizeof(levels) / sizeof(levels[0]);
    char filename_char_lv[50];

    for (int i = 0; i < length_lv-1; ++i) {
        snprintf(filename_char_lv, sizeof(filename_char_lv), "BMP_FONT/Number_%c.bmp", levels[i]);

        if (update_level) {
            load_texture(filename_char_lv, &bitmapSurface[i], &texture_lv[i], &renderer);
        }

        // COPY THE TEXTURE TO RENDER
        SDL_RenderCopy(renderer, texture_lv[i], NULL, &levels_dest);

        // NEXT POSITION
        levels_dest.x += (BLOCK_SIZE);
    }
    update_level = FALSE;

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
