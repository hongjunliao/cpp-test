/*! This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2022/6/13
 *
 * test for sdl
 *
 * */
#ifdef CPP_TEST_WITH_SDL2
///////////////////////////////////////////////////////////////////////////////////////////

#include <assert.h> /* assert */
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////

#include "SDL2/SDL.h"


int main(int argc, char *argv[])
{
    SDL_bool done = SDL_FALSE;

    InitVideo();
    /* ... */

    SDL_StartTextInput();
    while (!done) {
        SDL_Event event;
        if (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    /* Quit */
                    done = SDL_TRUE;
                    break;
                case SDL_TEXTINPUT:
                    /* Add new text onto the end of our text */
                    strcat(text, event.text.text);
                    break;
                case SDL_TEXTEDITING:
                    /*
                    Update the composition text.
                    Update the cursor position.
                    Update the selection length (if any).
                    */
                    composition = event.edit.text;
                    cursor = event.edit.start;
                    selection_len = event.edit.length;
                    break;
            }
        }
        Redraw();
    }

    SDL_Quit();

    return 0;
}
#endif
