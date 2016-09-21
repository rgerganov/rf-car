#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "rf.h"

int main(int argc, char *argv[])
{
    char optc;
    bool invert_steering = false;
    bool invert_throttle = false;
    bool swap_axes = false;

    struct direction_map_t direction_map;

    rf_global_args.FREQ = DEFAULT_FREQ;
    rf_global_args.SAMPLE_RATE = DEFAULT_SAMPLE_RATE;
    rf_global_args.SYMBOL_RATE = DEFAULT_SYMBOL_RATE;

    while ((optc = getopt(argc, argv, "f:s:STA")) > 0) {
        switch (optc) {
        case 'f':
            rf_global_args.FREQ = atoi(optarg);
            break;
        case 's':
            rf_global_args.SAMPLE_RATE = atoi(optarg);
            break;
        case 'S':
            invert_steering = true;
            break;
        case 'T':
            invert_throttle = true;
            break;
        case 'A':
            swap_axes = true;
            break;
        default:
            fprintf(stderr, "%s [args]\n", argv[0]);
            fprintf(stderr, "\t-f FREQUENCY\tset frequency (integer, HZ)\n");
            fprintf(stderr, "\t-s SAMPLE_RATE\tset sample rate\n");
            fprintf(stderr, "\t-S\t\tinvert steering\n");
            fprintf(stderr, "\t-T\t\tinvert throttle\n");
            fprintf(stderr, "\t-A\t\tswap axes (ie. use steering for throttle and throttle for steering)\n");
            exit(EXIT_FAILURE);
        }
    }

    /* initialize direction_map */
    set_direction_map(&direction_map, invert_steering, invert_throttle, swap_axes);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Window *window = SDL_CreateWindow("rf-car", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "CreateWindow Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        fprintf(stderr, "CreateRenderer Error: %s\n", SDL_GetError());
        return 1;
    }
    if (!init_rf()) {
        return 1;
    }

    SDL_Texture *arrows = IMG_LoadTexture(renderer, "arrows.png");
    if (arrows == NULL) {
        fprintf(stderr, "Cannot load arrows Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Event e;
    bool quit = false;
    SDL_Rect up_arr = {0, 0, 100, 100};
    SDL_Rect down_arr = {100, 0, 100, 100};
    SDL_Rect right_arr = {200, 0, 100, 100};
    SDL_Rect left_arr = {300, 0, 100, 100};
    SDL_Rect up_arr_p = {0, 100, 100, 100};
    SDL_Rect down_arr_p = {100, 100, 100, 100};
    SDL_Rect right_arr_p = {200, 100, 100, 100};
    SDL_Rect left_arr_p = {300, 100, 100, 100};

    int center_x = 320;
    int center_y = 270;
    SDL_Rect up_dst = {center_x-50, center_y-200, 100, 100};
    SDL_Rect down_dst = {center_x-50, center_y+100, 100, 100};
    SDL_Rect right_dst = {center_x+100, center_y-50, 100, 100};
    SDL_Rect left_dst = {center_x-200, center_y-50, 100, 100};
    int gain_tx = 20;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_EQUALS) {
                if (e.key.keysym.mod & KMOD_SHIFT) {
                    gain_tx += 1;
                }
            } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_MINUS) {
                gain_tx -= 1;
            } else if (e.type == SDL_MOUSEWHEEL) {
                gain_tx += e.wheel.y;
            }
        }
        SDL_Rect *up_src = &up_arr;
        SDL_Rect *down_src = &down_arr;
        SDL_Rect *right_src = &right_arr;
        SDL_Rect *left_src = &left_arr;
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        Direction dir = direction_map.none;
        if (state[SDL_SCANCODE_UP]) {
            up_src = &up_arr_p;
            dir = direction_map.fwd;
            if (state[SDL_SCANCODE_RIGHT]) {
                right_src = &right_arr_p;
                dir = direction_map.fwd_right;
            } else if (state[SDL_SCANCODE_LEFT]) {
                left_src = &left_arr_p;
                dir = direction_map.fwd_left;
            }
        } else if (state[SDL_SCANCODE_DOWN]) {
            down_src = &down_arr_p;
            dir = direction_map.back;
            if (state[SDL_SCANCODE_RIGHT]) {
                right_src = &right_arr_p;
                dir = direction_map.back_right;
            } else if (state[SDL_SCANCODE_LEFT]) {
                left_src = &left_arr_p;
                dir = direction_map.back_left;
            }
        } else if (state[SDL_SCANCODE_RIGHT]) {
            right_src = &right_arr_p;
            dir = direction_map.right;
        } else if (state[SDL_SCANCODE_LEFT]) {
            left_src = &left_arr_p;
            dir = direction_map.left;
        }

        gain_tx = (gain_tx > 0) ? gain_tx : 0;
        gain_tx = (gain_tx < 47) ? gain_tx : 47;
        state_change(dir, gain_tx);
        SDL_SetRenderDrawColor(renderer, 0xdc, 0xdc, 0xdc, 0xFF);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, arrows, up_src, &up_dst);
        SDL_RenderCopy(renderer, arrows, down_src, &down_dst);
        SDL_RenderCopy(renderer, arrows, right_src, &right_dst);
        SDL_RenderCopy(renderer, arrows, left_src, &left_dst);
        SDL_Rect gain_border = {0, 0, 580, 40};
        SDL_Rect gain_one = {10, 10, 10, 20};
        SDL_SetRenderDrawColor(renderer, 0x70, 0x80, 0x90, 0xFF);
        SDL_RenderFillRect(renderer, &gain_border);
        gain_border.x += 5;
        gain_border.y += 5;
        gain_border.w -= 10;
        gain_border.h -= 10;
        SDL_SetRenderDrawColor(renderer, 0xdc, 0xdc, 0xdc, 0xFF);
        SDL_RenderFillRect(renderer, &gain_border);
        SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xFF);
        for (int i =  0 ; i < gain_tx ; i++) {
            SDL_RenderFillRect(renderer, &gain_one);
            gain_one.x += 12;
        }
        SDL_RenderPresent(renderer);
    }

    close_rf();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
