#include "A_Application.h"
#include "R_Rendering.h"
#include "U_Utilities.h"
#include <math.h>

// ----------------------------------------------------
// Given an X pixel, draw the whole column
// ----------------------------------------------------
void R_DrawFullColumn(int x, int color)
{
    for(int y = 0; y < SCREEN_HEIGHT; y++)
        pixels[x + y * width] = color;
}

// ----------------------------------------------------
// Clear the screen
// ----------------------------------------------------
void R_ClearCanvas(void)
{
    for(int y = 0; y < SCREEN_HEIGHT; y++)
    for(int x = 0; x < SCREEN_WIDTH; x++)
        pixels[x + y * width] = SDL_MapRGBA(window_surface->format, 0, 0, 0, 0);
}


// ----------------------------------------------------
// Draws the Palette
// ----------------------------------------------------
void R_DrawPalette()
{
    // Draw line and background
    R_DrawFullColumn(SCREEN_WIDTH+1, SDL_MapRGBA(window_surface->format, 255, 255, 255, 255));
    int paletteX = SCREEN_WIDTH+2;
    
    while(paletteX < SCREEN_WIDTH + PALETTE_WIDTH)
        R_DrawFullColumn(paletteX++, SDL_MapRGBA(window_surface->format, 128, 128, 128, 128));


    R_DrawAllButtons();

    R_UpdateColorPicker();
}

// ----------------------------------------------------
// Draws all the Palette Buttons
// ----------------------------------------------------
void R_DrawAllButtons()
{
    SDL_Surface* surface;
    SDL_Rect r1,r2;
    for(int i = 0; i < PALETTE_BUTTONS_COUNT; i++)
    {
        if(paletteButtons[i].render == true)
        {
            surface = SDL_LoadBMP(paletteButtons[i].fileName);
            SDL_Rect_Set(&r1, 0, 0, 50, 50);
            SDL_Rect_Set(&r2, paletteButtons[i].box.x, paletteButtons[i].box.y, paletteButtons[i].box.w, paletteButtons[i].box.h);
            SDL_BlitSurface(surface, &r1,window_surface, &r2);
            SDL_FreeSurface(surface);
        }
    }
}


// ----------------------------------------------------
// Updates the Color Picker when brightness changes.
// ----------------------------------------------------
void R_UpdateColorPicker(void)
{
    // Draw RGB scheme
    int startingPixelXOffset = SCREEN_WIDTH + 7;
    int startingPixelYOffset = 10;

    static ColorRGB_t color;
    for(int x = 0; x <128; x++)
    for(int y = 0; y < 64; y++)
    {
        ColorHSV_t hsv;
        hsv.h = x*2;
        hsv.s = y * 4;
        hsv.v = paletteBrightness;

        color = HSVtoRGB(hsv);
        pixels[(x+startingPixelXOffset) + (y + startingPixelYOffset) * width] = SDL_MapRGB(window_surface->format, color.r, color.g, color.b);
    }
    for(int x = 128; x < 144; x++)
    for(int y = 0; y < 64; y++)
    {
        pixels[(x+startingPixelXOffset) + (y + startingPixelYOffset) * width] = SDL_MapRGB(window_surface->format, y * 4, y * 4, y * 4);
    }

    R_DrawCurrentColor();
}

// ----------------------------------------------------
// Updates the Current Color when changes.
// ----------------------------------------------------
void R_DrawCurrentColor(void)
{
    // Draw current color
    int startingPixelXOffset = SCREEN_WIDTH + 7 + 46;
    int startingPixelYOffset = 102;

    for(int x = 0; x < 48; x++)
        for(int y = 0; y < 48; y++)
            pixels[(x+startingPixelXOffset) + (y + startingPixelYOffset) * width] = currentMainColor;
}

// ----------------------------------------------------
// Paint
// ----------------------------------------------------
void R_Paint(int x1, int y1, int x2, int y2)
{
    // Draw a simple line if bushSize is 1
        if(bushSize <= 1) {
             if( x1 >= 0 && x1 < SCREEN_WIDTH && y1 >= 0 && y1 < SCREEN_HEIGHT)    // To not go outside of boundaries
            pixels[x1 + y1 * width] = drawing ? currentMainColor : currentAltColor;
        }
        else // Otherwise keep drawing circles
            R_DrawCircle(x1, y1, bushSize);
            
    // Creates the path from Old Mouse coords and current
    int repx = 1;
    int repy = 1;
    int i = 0;
    int j = 0;
    while(x1 != x2 || y1 != y2)
    {
        if(i < repx){
            if(x1 != x2)
                x1 += x1 > x2 ? -1 : 1;
            else
                repx = 0;
            i++;
        }
        if(j < repy){
            if(y1 != y2)
                y1 += y1 > y2 ? -1 : 1;
            else
                repy = 0;
            j++;
        }
        if (i >= repx && j >= repy){
            if(x1 != x2 && y1 != y2){
                float slope = fabsf(((float)(y2-y1))/((float)(x2-x1)));
                if(slope > 128.f) slope = 128.f;
                if(slope < 0.0078125f) slope = 0.0078125f;
                if(slope >= 1){
                    repy=round(slope);
                }
                else{
                    repx=round(1.f/slope);
                }
            }
            i = 0;
            j = 0;
        }
        // Draw a simple line if bushSize is 1
        if(bushSize <= 1) {
            if( x1 >= 0 && x1 < SCREEN_WIDTH && y1 >= 0 && y1 < SCREEN_HEIGHT)    // To not go outside of boundaries
                pixels[x1 + y1 * width] = drawing ? currentMainColor : currentAltColor;
        }
        else // Otherwise keep drawing circles
            R_DrawCircle(x1, y1, bushSize);
    }
}

// ----------------------------------------------------
// Used to draw at 2+px bush size.
// ----------------------------------------------------
void R_DrawCircle(int x0, int y0, int r)
{
    for(int y=-r; y<=r; y++)
        for(int x=-r; x<=r; x++)
            if(x*x+y*y <= r*r)
                if( x0+x >= 0 && x0+x < SCREEN_WIDTH && y0+y >= 0 && y0+y < SCREEN_HEIGHT)    // To not go outside of boundaries
                    pixels[(x0+x) + (y0+y) * width] = drawing ? currentMainColor : currentAltColor;
}

// ----------------------------------------------------
// Line Flood Fill, for the bucket tool
// ----------------------------------------------------
void R_LineFloodFill(int x, int y, int color, int ogColor)
{
    if(color == ogColor)
        return;

    printf("Flood filling... \n");
    
    transform2d_t stack[100];
    transform2d_t curElement;
    int stackTop = -1;

    boolean_t mRight; // 
    boolean_t alreadyCheckedAbove, alreadyCheckedBelow;

    // Push the first element
    FF_StackPush(stack, x, y, &stackTop);
    while(stackTop >= 0)    // While there are elements
    {
        // Take the first one
        curElement = FF_StackPop(stack, &stackTop);

        mRight = false;
        int leftestX = curElement.x;

        // Find leftest
        while(leftestX >= 0 && pixels[leftestX + curElement.y * width] == ogColor) 
            leftestX--;
        leftestX++;

        alreadyCheckedAbove = false;
        alreadyCheckedBelow = false;

        // While this line has not finsihed to be drawn
        while(mRight == false)
        {
            // Fill right
            if(leftestX < SCREEN_WIDTH && pixels[leftestX + curElement.y * width] == ogColor) 
            {
                pixels[leftestX + curElement.y * width] = color;

                // Check above this pixel
                if (alreadyCheckedBelow == false && (curElement.y-1) >= 0 && (curElement.y-1) < SCREEN_HEIGHT &&
                    pixels[leftestX + ((curElement.y-1) * width)] == ogColor)
                    {
                        // If we never checked it, add it to the stack
                        FF_StackPush(stack, leftestX, curElement.y-1, &stackTop);
                        alreadyCheckedBelow = true;
                    }
                else if(alreadyCheckedBelow == true && (curElement.y-1) > 0 && pixels[leftestX + (curElement.y-1) * width] != ogColor)
                {
                    // Skip now, but check next time
                    alreadyCheckedBelow = false;
                }
                
                // Check below this pixel
                if (alreadyCheckedAbove == false && (curElement.y+1) >= 0 && (curElement.y+1) < SCREEN_HEIGHT &&
                    pixels[leftestX + ((curElement.y+1) * width)] == ogColor) 
                    {
                        // If we never checked it, add it to the stack
                        FF_StackPush(stack, leftestX, curElement.y+1, &stackTop);
                        alreadyCheckedAbove = true;
                    }
                else if (alreadyCheckedAbove == true && (curElement.y+1) < SCREEN_WIDTH && pixels[leftestX + (curElement.y+1) * width] != ogColor)
                {
                    // Skip now, but check next time
                    alreadyCheckedAbove = false;
                }

                // Keep going on the right
                leftestX++;
            }
            else // Done
                mRight = true;
        }
    }
}

// ----------------------------------------------------
// Converts from HSV to RGB
// ----------------------------------------------------
ColorRGB_t HSVtoRGB(ColorHSV_t colorHSV)
{
    float r, g, b, h, s, v; //this function works with floats between 0 and 1
    h = colorHSV.h / 256.0;
    s = colorHSV.s / 256.0;
    v = colorHSV.v / 256.0;

    //If saturation is 0, the color is a shade of gray
    if(s == 0) r = g = b = v;
    //If saturation > 0, more complex calculations are needed
    else
    {
        float f, p, q, t;
        int i;
        h *= 6; //to bring hue to a number between 0 and 6, better for the calculations
        i = (int)(floor(h));  //e.g. 2.7 becomes 2 and 3.01 becomes 3 or 4.9999 becomes 4
        f = h - i;  //the fractional part of h
        p = v * (1 - s);
        q = v * (1 - (s * f));
        t = v * (1 - (s * (1 - f)));
        switch(i)
        {
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            case 5: r = v; g = p; b = q; break;
        }
    }

    
    ColorRGB_t colorRGB;
    colorRGB.r = (int)(r * 255.0);
    colorRGB.g = (int)(g * 255.0);
    colorRGB.b = (int)(b * 255.0);
    return colorRGB;
}