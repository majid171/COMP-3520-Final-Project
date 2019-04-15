/*
    Demonstrating 2D objects and clipping
*/

#include <cstdio> 
#include <iostream>
#include <unistd.h>
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include <cstdlib>
#include <ctime> 

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define BLACK 0x000000FF
#define WHITE 0xFFFFFFFF

using namespace std;

// Making life easier
struct Resources{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Surface *canvas;
    TTF_Font *font;
};

// Chose to represent the polygon as a vector of Points
struct Point{
    int x, y;
    Point(int x, int y) {this->x = x; this->y = y;}
    Point(){}
};

// Function prototypes
void update(Resources);
void erase(uint32_t (*)[SCREEN_WIDTH], Resources);
void drawLine(uint32_t (*)[SCREEN_WIDTH], Point, Point);
void end(Resources res);
void drawPoly(uint32_t (*)[SCREEN_WIDTH], Resources, vector<Point>);
void liangBarskyHelper(vector<Point>, uint32_t (*)[SCREEN_WIDTH], Resources);
void liangBarsky(float, float, float, float, Point, Point, uint32_t (*)[SCREEN_WIDTH]);
float maxi(float[],int);
float mini(float[], int);
vector<Point> drawSquare(Point, int);
vector<Point> drawTriangle(Point, int);
int random(int, int);
void clean(uint32_t (*)[SCREEN_WIDTH], Resources);

int main(){

    // VERY IMPORTANT: Ensure SDL2 is initialized
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "could not initialize sdl2: %s\n", SDL_GetError());
        return 1;
    }

    // VERY IMPORTANT: if using text in your program, ensure SDL2_ttf library is
    // initialized
    if (TTF_Init() < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "could not initialize SDL2_ttf: %s\n", TTF_GetError());
    }

    Resources res;

    res.window = SDL_CreateWindow("60-352 Project", SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (res.window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not create window!\n");
        return 1;
    }

    res.renderer = SDL_CreateRenderer(res.window, -1,
                                    0); // don't force hardware or software
                                        // accel -- let SDL2 choose what's best

    if (res.renderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "could not create renderer: %s\n", SDL_GetError());
        return 1;
    }

    res.canvas = SDL_CreateRGBSurfaceWithFormat(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
                                               SDL_PIXELFORMAT_RGBA8888);
    if (res.canvas == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "could not create surface: %s\n",
                 SDL_GetError());
        return 1;
    }

    res.texture =
        SDL_CreateTexture(res.renderer, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    if (res.texture == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "could not create texture: %s\n",
                 SDL_GetError());
        return 1;
    }

    TTF_Font* font = TTF_OpenFont("iosevka-regular.ttf", 20);
    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_OpenFont: %s\n", TTF_GetError());
        return 1;
    }

    SDL_FillRect(res.canvas, NULL, SDL_MapRGB(res.canvas->format, BLACK, BLACK, BLACK));
    uint32_t (*pixels)[SCREEN_WIDTH] = (uint32_t(*)[SCREEN_WIDTH]) res.canvas->pixels;

    srand(time(NULL));

    // Drawing the clipping window
    vector<Point> clipper;
    clipper.push_back(Point(20, 100));
    clipper.push_back(Point(620, 100));
    clipper.push_back(Point(620, 460));
    clipper.push_back(Point(20, 460));
    drawPoly(pixels, res, clipper);

    static SDL_Color const colour = {100, 100, 100};
    static char str[128];

    SDL_snprintf(str, sizeof(str), "<Space> -> Clear || <Up> --> Square || <Down> --> Triangle");
    SDL_Surface* sur1 = TTF_RenderText_Blended(font, str, colour);
    SDL_BlitSurface(sur1, nullptr, res.canvas, nullptr);
    SDL_FreeSurface(sur1);

    update(res);
    SDL_RenderClear(res.renderer);
    SDL_RenderCopy(res.renderer, res.texture, nullptr, nullptr);
    SDL_RenderPresent(res.renderer);

    bool isquit = false;
    SDL_Event event;
    while (!isquit) {
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isquit = true;
            }
            else if(event.type == SDL_KEYDOWN){ // draw a random square
                if(event.key.keysym.sym == SDLK_DOWN){
                    Point start;
                    start.x = random(20, 620);
                    start.y = random(100, 460); 
                    vector<Point> s = drawTriangle(start, random(40, 100));
                    liangBarskyHelper(s, pixels, res);
                    clean(pixels, res);
                }
                else if(event.key.keysym.sym == SDLK_UP){ // Draw a random Triangle
                    Point start;
                    start.x = random(20, 620);
                    start.y = random(100, 460); 
                    vector<Point> s = drawSquare(start, random(40, 100));
                    liangBarskyHelper(s, pixels, res);
                    clean(pixels, res);
                }
                else if(event.key.keysym.sym == SDLK_SPACE){ // Clear the drawing canvas
                    erase(pixels, res);
                }
            }
        }
    }

    end(res);
    SDL_Quit();
    return 0;
}

void clean(uint32_t (*pixels)[SCREEN_WIDTH], Resources res){

    for(int i = 0; i < SCREEN_WIDTH; i++){
        for(int j = 100; j < SCREEN_HEIGHT; j++){
            if(!(i >= 20 && i <= 620 && j >= 100 && j <= 460)){
                pixels[j][i] = WHITE;
            }
        }
    }

    update(res);
}

int random(int min, int max){
    return rand() % (max + 1 - min) + min;
}

vector<Point> drawSquare(Point start, int len){
    vector<Point> s;

    s.push_back(Point(start.x, start.y));
    s.push_back(Point(start.x + len, start.y));
    s.push_back(Point(start.x + len, start.y + len));
    s.push_back(Point(start.x, start.y + len));

    return s;
}

vector<Point> drawTriangle(Point start, int len){
    vector<Point> s;
    
    s.push_back(Point(start.x, start.y));
    s.push_back(Point(start.x + (len/2), start.y - len));
    s.push_back(Point(start.x + len, start.y));

    return s;
}

// Returns max of an array
float maxi(float arr[], int n) {
    float m = 0;
    for (int i = 0; i < n; ++i){
        if (m < arr[i]){
            m = arr[i];
        }
    }

    return m;
}

// Returns min of an array
float mini(float arr[], int n){
    float m = 1;
    for(int i = 0; i < n; ++i){
        if (m > arr[i]){
            m = arr[i];
        }
    }

    return m;
}

void liangBarskyHelper(vector<Point> poly, uint32_t (*pixels)[SCREEN_WIDTH], Resources res){

    for(int i = 0; i < poly.size(); i++){
        int k = (i + 1) % poly.size();
        
        liangBarsky(20, 100, 620, 460, poly[i], poly[k], pixels);
    }

    update(res);

} 

void liangBarsky(float xmin, float ymin, float xmax, float ymax, Point pi, Point pf, uint32_t (*pixels)[SCREEN_WIDTH]){
  
    float x1, y1, x2, y2;
    x1 = pi.x; y1 = pi.y; x2 = pf.x; y2 = pf.y;

    //defining variables
    float p1 = -(x2 - x1);
    float p2 = -p1;
    float p3 = -(y2 - y1);
    float p4 = -p3;

    float q1 = x1 - xmin;
    float q2 = xmax - x1;
    float q3 = y1 - ymin;
    float q4 = ymax - y1;

    float posarr[5], negarr[5];
    int posind = 1, negind = 1;
    posarr[0] = 1;
    negarr[0] = 0;

    if ((p1 == 0 && q1 < 0) || (p3 == 0 && q3 < 0)) {
        //Line is parallel to clipping window!
        return;
    }
    if (p1 != 0) {
        float r1 = q1 / p1;
        float r2 = q2 / p2;
        
        if (p1 < 0) {
            negarr[negind++] = r1; // for negative p1, add it to negative array
            posarr[posind++] = r2; // and add p2 to positive array
        }else{
            negarr[negind++] = r2;
            posarr[posind++] = r1;
        }
    }

    if (p3 != 0) {
        float r3 = q3 / p3;
        float r4 = q4 / p4;
        
        if (p3 < 0){
            negarr[negind++] = r3;
            posarr[posind++] = r4;
        }else{
            negarr[negind++] = r4;
            posarr[posind++] = r3;
        }
    }

    float xn1, yn1, xn2, yn2;
    float rn1, rn2;
    rn1 = maxi(negarr, negind); // maximum of negative array
    rn2 = mini(posarr, posind); // minimum of positive array

    if(rn1 > rn2){ // reject
        return;
    }

    // Computing the new points
    xn1 = x1 + p2 * rn1;
    yn1 = y1 + p4 * rn1; 
    xn2 = x1 + p2 * rn2;
    yn2 = y1 + p4 * rn2;

    // Directly drawing the line made by the 2 (new) points
    drawLine(pixels, Point(xn1, yn1), Point(xn2, yn2));
}

// Used to update the canvas
void update(Resources res){
    SDL_UpdateTexture(res.texture, NULL, res.canvas->pixels, res.canvas->pitch);
    SDL_RenderCopy(res.renderer, res.texture, NULL, NULL);
    SDL_RenderPresent(res.renderer);
}

// Used to erase the canvas
void erase(uint32_t (*pixels)[SCREEN_WIDTH], Resources res){
    
    // Paint every pixel on the screen white
    for(int i = 21; i < 620; i++){
        for(int j = 101; j < 460; j++){
            pixels[j][i] = WHITE;
        }
    }

    update(res);
}   

// Clean up at the end
void end(Resources res){
    SDL_DestroyTexture(res.texture);
    SDL_FreeSurface(res.canvas);
    SDL_DestroyRenderer(res.renderer);
    SDL_DestroyWindow(res.window); 
}

// Draws the polygon given the representation of it
void drawPoly(uint32_t (*pixels)[SCREEN_WIDTH], Resources res, vector<Point> poly){

    for(int i = 0; i < poly.size() - 1; i++){
        Point p1 = poly[i];
        Point p2 = poly[i + 1];
        drawLine(pixels, p1, p2);
    }
    drawLine(pixels, poly[0], poly[poly.size() - 1]);

    update(res);
}

// Given two points, it draws a connecting line
// Uses Breesenham's line drawing algorithm
void drawLine(uint32_t (*pixels)[SCREEN_WIDTH], Point p1, Point p2){
    
    int x1, y1, x2, y2;
    x1 = p1.x; y1 = p1.y; x2 = p2.x; y2 = p2.y;

    bool steep = (fabs(y2 - y1) > fabs(x2 - x1));
    if(steep){
        swap(x1, y1);
        swap(x2, y2);
    }

    if(x1 > x2){
        swap(x1, x2);
        swap(y1, y2);
    }

    float dx = x2 - x1;
    float dy = fabs(y2 - y1);

    float error = dx / 2.0f;
    int ystep = (y1 < y2) ? 1 : -1;
    int y = (int)y1;

    int maxX = (int)x2;

    for(int x=(int)x1; x<maxX; x++){

        if(steep){
            pixels[x][y] = BLACK;
        }
        else{
            pixels[y][x] = BLACK;
        }

        error -= dy;
        if(error < 0){
            y += ystep;
            error += dx;
        }
    }
}