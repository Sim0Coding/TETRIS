#include <stdio.h>
#include <SDL.h>
#include <time.h>
#include "constants.h"
#include "gameFunctions.h"

// STANDARDS VARIABLES
int game_is_running = FALSE;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int last_frame_time = 0;

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
            NULL,                     // TITLE
            SDL_WINDOWPOS_CENTERED,     // X COORDINATES
            SDL_WINDOWPOS_CENTERED,     // Y COORDINATES
            WINDOW_WIDTH,               // WIDTH
            WINDOW_HEIGHT,              // HEIGHT
            SDL_WINDOW_BORDERLESS     // WINDOW CHARACTERISTICS
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
    for (int i = 0; i < NROW; ++i) {
        for (int j = 0; j < NCOL; ++j) {
            dynamic_field[i][j] = 0;
        }
    }

    boundary.width = BLOCK_SIZE * NCOL;
    boundary.height = BLOCK_SIZE * NROW;
    boundary.x = (WINDOW_WIDTH/2)-(boundary.width/2);
    boundary.y = (WINDOW_HEIGHT/2)-(boundary.height/2);

    block.x = boundary.x;
    block.y = boundary.y;
    block.width = BLOCK_COLOR_SIZE;
    block.height = BLOCK_COLOR_SIZE;

    falling_timeout = SDL_GetTicks() + FALLING_TIME;
    next_move_timeout = SDL_GetTicks() + NEXT_MOVE_TIME;

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
                game_is_running = FALSE;
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
 *  26/11/2024
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

    // TODO: CHECK IF IN THE FIRST 4 ROWS THERE ARE SOME PIECES TOUCHING THE ONES WHICH SPAWNS, IF YES THEN THE PLAYER LOSE AND THE GAME CLOSE

}

/*
 * ------------------------|RENDER|------------------------
 *  Date of Creation:
 *  17/11/2024
 *  Parameters:
 *
 *  Description:
 *  It is executed until the variable game_is_running is 1.
 *  Whatever there is in this function is meant to be drawn
 *  on a renderer every frame.
 */
void render(){ // CAN BE CALLED ALSO draw()
    SDL_SetRenderDrawColor(renderer, 0,0,0,255); // SET THE COLOR
    SDL_RenderClear(renderer);

    // FIELD MARGINS RENDERING
    SDL_Rect boundary_rect = {
            (int)boundary.x,
            (int)boundary.y,
            (int)boundary.width + 4,  // VISUAL ADJUSTMENTS FOR THE width OF THE MARGIN
            (int)boundary.height + 4  // VISUAL ADJUSTMENTS FOR THE height OF THE MARGIN
    };
    boundary_rect.x -= 3; // VISUAL ADJUSTMENTS FOR THE x OF THE MARGIN
    boundary_rect.y -= 3; // VISUAL ADJUSTMENTS FOR THE y OF THE MARGIN
    SDL_SetRenderDrawColor(renderer,255,255,255,255);
    SDL_RenderDrawRect(renderer,&boundary_rect);

    // GAME FIELD RENDERING
    SDL_Rect block_rect = {
            (int)block.x,
            (int)block.y,
            (int)block.width,
            (int)block.height
    };
    for (int i = 0; i < NROW; ++i) {
        for (int j = 0; j < NCOL; ++j) {
            if (dynamic_field[i][j] == 1){
                SDL_SetRenderDrawColor(renderer,255,255,255,255);
                SDL_RenderFillRect(renderer, &block_rect);
            }
            block_rect.x += BLOCK_SIZE;
        }
        block_rect.x = (int)boundary.x;
        block_rect.y += BLOCK_SIZE;
    }
    block_rect.x = (int)block.x;
    block_rect.y = (int)block.y;
    for (int i = 0; i < NROW; ++i) {
        for (int j = 0; j < NCOL; ++j) {
            if (static_field[i][j] == 1){
                SDL_SetRenderDrawColor(renderer,0,255,255,255);
                SDL_RenderFillRect(renderer, &block_rect);
            }
            block_rect.x += BLOCK_SIZE;
        }
        block_rect.x = (int)boundary.x;
        block_rect.y += BLOCK_SIZE;
    }

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
