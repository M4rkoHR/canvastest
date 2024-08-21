#include <stdlib.h>
#include <SDL2/SDL.h>

// paint: Utility function that paints colors to a surface.
void paint(SDL_Renderer *renderer, int x, int y, Uint8 red, Uint8 green, Uint8 blue);

int main() {
    // Initialize the SDL library
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);

    // Create a window and renderer
    SDL_Window *window = SDL_CreateWindow("Paint Program",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          800, 600, SDL_WINDOW_RESIZABLE);
    if (!window) {
        fprintf(stderr, "Unable to create window: %s\n", SDL_GetError());
        exit(2);
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Unable to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        exit(3);
    }

    // Main loop
    int painting = 0; // variable to keep track of mouse-down state
    Uint8 red, green, blue; // variables to keep track of our current color

    while (1) {
        SDL_Event event;

        // Wait for the user to do something
        while (SDL_PollEvent(&event)) {
            // Act on the various event types
            if (event.type == SDL_QUIT) { // the user wants to quit
                exit(0);
            } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) { // our window was resized
                // Clear the renderer when resized
                SDL_RenderClear(renderer);
            } else if (event.type == SDL_MOUSEBUTTONDOWN) { // a button is being pressed
                // Start painting with a random color
                painting = 1;
                red = (rand() % 7 + 1) * 32;
                green = (rand() % 7 + 1) * 32;
                blue = (rand() % 7 + 1) * 32;
                paint(renderer, event.button.x, event.button.y, red, green, blue);
            } else if (event.type == SDL_MOUSEMOTION) { // the mouse moved
                if (painting) {
                    // Continue painting
                    paint(renderer, event.motion.x, event.motion.y, red, green, blue);
                }
            } else if (event.type == SDL_MOUSEBUTTONUP) { // a button is being released
                // Stop painting
                painting = 0;
            }
        }

        // Update the screen
        SDL_RenderPresent(renderer);
    }

    // Cleanup SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

/*
paint: Utility function that paints colors to a surface.
The location to paint is given by x and y, the color to paint is
a mixture of red, green, and blue values in the range 0 to 255.
*/
void paint(SDL_Renderer *renderer, int x, int y, Uint8 red, Uint8 green, Uint8 blue) {
    // Set the drawing color
    SDL_SetRenderDrawColor(renderer, red, green, blue, 255);

    // Draw a point at (x, y)
    SDL_RenderDrawPoint(renderer, x, y);
}
