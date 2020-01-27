#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <algorithm>

#define WIDTH 1024
#define HEIGHT 768
#define FPS 60
#define BGS 2
#define ENEMIES 4
using namespace std;

class Bullet
{
public: // Properties
	bool m_active = true;
	SDL_Rect m_dst, m_src;
	static const int speed = 8; // Shared property for ALL bullet objects.

public: // Methods.
	Bullet(int x, int y)
	{
		m_dst = { x + 5, y, 25, 25 };
	}
	void update()
	{
		m_dst.x += speed;
		if (m_dst.x > WIDTH) // If bullet goes off-screen.
			m_active = false;
	}
};

//class EnemyBullet
//{
//public:
//bool m_active = true;
//	EnemyBullet(SDL_Rect d)
//	{
//		m_dst2 = d;
//	}
//	void update()
//	{
//		m_dst2.x -= 4; // Just a literal speed. You may want a variable.
//		if (m_dst2.x < 0 - m_dst2.w) // Off-screen.
//			m_active = false;
//	}
//};

 class Enemy
{
public: // Properties
	bool active = true;
	SDL_Rect m_dst, m_src;

private:
	int respawnCtr = 0;
	int respawnMax = 180;

public: // Public methods.
	void update()
	{ // Just simulates a respawn timer.
		if (!active)
		{
			respawnCtr++;
			if (respawnCtr == respawnMax)
			{
				respawnCtr = 0;
				active = true;
			}
		}
	}
};

struct Background
{
	SDL_Rect m_src, m_dst1;
};

// Global engine variables
bool g_bRunning = false; // Loop control flag.
int g_iSpeed = 5; // Speed of box.
const Uint8* g_iKeystates; // Keyboard state container.
Uint32 g_start, g_end, g_delta, g_fps; // Fixed timestep variables.
SDL_Window* g_pWindow; // This represents the SDL window.
SDL_Renderer* g_pRenderer; // This represents the buffer to draw to.

Background bgArray[BGS];

// New variables for sprite.
SDL_Rect g_src, g_dst;
SDL_Rect m_src, m_dst;

int g_frame = 0,
	g_frameMax = 3,
	g_sprite = 0,
	g_spriteMax = 6;

SDL_Texture* g_pTexture;
SDL_Texture* g_pBackground;
SDL_Texture* g_pBullet;
SDL_Texture* g_pEnemy;

// Vectors for bullets.
vector<Bullet*> bulletVec;// Holds pointers to dynamically-created bullet objects.
Enemy enemyArray[ENEMIES];



bool init(const char* title, int xpos, int ypos, int width, int height, int flags)
{
	cout << "Initializing game." << endl;
	// Attempt to initialize SDL.
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		// Create the window.
		g_pWindow = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
		if (g_pWindow != nullptr) // Window init success.
		{
			g_pRenderer = SDL_CreateRenderer(g_pWindow, -1, 0);
			if (g_pRenderer != nullptr) // Renderer init success.
			{
				if (IMG_Init(IMG_INIT_PNG))
				{
					g_pBackground = IMG_LoadTexture(g_pRenderer, "Hills.png");
					g_pTexture = IMG_LoadTexture(g_pRenderer, "Idle.png");
					g_pBullet = IMG_LoadTexture(g_pRenderer, "nebula.png");
					g_pEnemy = IMG_LoadTexture(g_pRenderer, "ghostyboi.png");
				}
				else return false; // Init init fail.
			}
			else return false; // Renderer init fail.
		}
		else return false; // Window init fail.
	}
	else return false; // SDL init fail.
	g_fps = (Uint32)round((1 / (double)FPS) * 1000); // Sets FPS in milliseconds and rounds.
	g_iKeystates = SDL_GetKeyboardState(nullptr);
	bgArray[0] = { {0,0,1024,768}, {0, 0, 1024, 768} }; // Src and dst rectangle objects.
	bgArray[1] = { {0,0,1024,768}, {1024, 0, 1024, 768} };
	// Create the sprite.
	g_src = { 0, 0, 190, 190};
	g_dst = { width/2 - g_src.w/2, height - g_src.h, g_src.w, g_src.h };
	m_src = { 0, 0, 80, 80 };
	/*enemyArray[0].m_dst = { 768, 64, 100, 100 };
	enemyArray[1].m_dst = { 640, 256, 100, 100 };
	enemyArray[2].m_dst = { 640, 448, 100, 100 };*/
	enemyArray[3].m_dst = { 768, 640, 100, 100 };
	g_bRunning = true; // Everything is okay, start the engine.
	cout << "Success!" << endl;
	return true;
}

void wake()
{
	g_start = SDL_GetTicks();
}

void sleep()
{
	g_end = SDL_GetTicks();
	g_delta = g_end - g_start;
	if (g_delta < g_fps) // Engine has to sleep.
		SDL_Delay(g_fps - g_delta);
}

void handleEvents()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT: // User pressed window's 'x' button.
			g_bRunning = false;
			break;
		case SDL_KEYDOWN: // Try SDL_KEYUP instead.
			if (event.key.keysym.sym == SDLK_ESCAPE)
				g_bRunning = false;
			else if (event.key.keysym.sym == SDLK_SPACE)
				bulletVec.push_back(new Bullet(g_dst.x + 125,g_dst.y + g_dst.h/2));
			break;
		}
	}
}

// Keyboard utility function.
bool keyDown(SDL_Scancode c)
{
	if (g_iKeystates != nullptr)
	{
		if (g_iKeystates[c] == 1)
			return true;
		else
			return false;
	}
	return false;
}

void checkCollision()
{
	for (int i = 0; i < (int)bulletVec.size(); i++)
	{
		for (int j = 0; j < ENEMIES; j++)
		{
			if (!enemyArray[j].active)
				continue; //  Skip this enemy and go to next iteration of loop.
			// Now check collision. To simplify, make temporary bounding boxes.
			SDL_Rect b = bulletVec[i]->m_dst; // Bullet bounding box.
			SDL_Rect e = enemyArray[j].m_dst; // Enemy bounding box.
			if (!((b.x > e.x + e.w) ||	// bullet.left > enemy.right 
				(b.x + b.w < e.x) ||	// bullet.right < enemy.left
				(b.y + b.h < e.y) ||	// bullet.bottom < enemy.top
				(b.y > e.y + e.h)))	// bullet.top > enemy.bottom
			{
				bulletVec[i]->m_active = false;
				enemyArray[j].active = false;
				break; // Skip the rest of the enemies for this bullet.
			}
		}
	}
}


void update()
{
		// This is the main game stuff.


		if (keyDown(SDL_SCANCODE_W))
			g_dst.y -= g_iSpeed;
		if (keyDown(SDL_SCANCODE_S))
			g_dst.y += g_iSpeed;
		if (keyDown(SDL_SCANCODE_A))
			g_dst.x -= g_iSpeed;
		if (keyDown(SDL_SCANCODE_D))
			g_dst.x += g_iSpeed;

	// Bullet handling. Update bullets.
	for (int i = 0; i < (int)bulletVec.size(); i++)
	{
		bulletVec[i]->update();
		if (bulletVec[i]->m_active == false)
		{
			delete bulletVec[i];
			bulletVec[i] = nullptr;
		}
	}
	bulletVec.erase(remove(bulletVec.begin(), bulletVec.end(), nullptr), bulletVec.end());
	// Update enemies.
	for (int i = 0; i < ENEMIES; i++)
		enemyArray[i].update();
	// Collision check.
	checkCollision();
	// Player box.

		if (g_dst.x < 0)
		{
			g_dst.x = 0;
		}
		else
			g_dst.x - g_dst.x;
		if (g_dst.x > 400)
		{
			g_dst.x = 400;
		}
		else
			g_dst.x = g_dst.x;
		if (g_dst.y < 0)
		{
			g_dst.y = 0;
		}
		else
			g_dst.y - g_dst.y;
		if (g_dst.y > 670)
		{
			g_dst.y = 670;
		}
		else
			g_dst.y = g_dst.y;
	// Scroll the backgrounds.
	for (int i = 0; i < BGS; i++)
		bgArray[i].m_dst1.x -= 1;
	// Check if they need to snap back. I chose the 2nd one.
	if (bgArray[1].m_dst1.x <= 0) // Or bgArray[0].m_dst1.x <= -1024
	{
		bgArray[0].m_dst1.x = 0;
		bgArray[1].m_dst1.x = 1024;
	}
}

void render()
{
	SDL_SetRenderDrawColor(g_pRenderer, 0, 0, 0, 255);
	SDL_RenderClear(g_pRenderer); // Clear the screen with the draw color.
	// Render Background.
	for (int i = 0; i < BGS; i++)
		SDL_RenderCopy(g_pRenderer, g_pBackground, &bgArray[i].m_src, &bgArray[i].m_dst1);
	// Render Player.
	SDL_RenderCopy(g_pRenderer, g_pTexture, &g_src, &g_dst);
	// Render Bullet.
	for (int i = 0; i < (int)bulletVec.size(); i++)
	{
		SDL_RenderCopy(g_pRenderer, g_pBullet, &g_src, &bulletVec[i]->m_dst);
	}
	// Render Enemies
	for (int i = 0; i < ENEMIES; i++)
	{
		if (enemyArray[i].active)
			SDL_RenderCopy(g_pRenderer, g_pEnemy, &m_src, &enemyArray[i].m_dst);
	}
	// Draw anew.
	SDL_RenderPresent(g_pRenderer);
}

void clean()
{
	cout << "Cleaning game." << endl;
	for (int i = 0; i < (int)bulletVec.size(); i++)
	{ // Proper way to empty a vector of pointers.
		delete bulletVec[i];
		bulletVec[i] = nullptr;
	}
	bulletVec.clear(); // Empty all elements.
	bulletVec.shrink_to_fit(); // Shrink down capacity to 0.
	SDL_DestroyTexture(g_pBackground);
	SDL_DestroyTexture(g_pTexture);
	SDL_DestroyTexture(g_pEnemy);
	SDL_DestroyTexture(g_pEnemy);
	SDL_DestroyRenderer(g_pRenderer);
	SDL_DestroyWindow(g_pWindow);
	SDL_Quit();
}

// Main function.
int main(int argc, char* args[]) // Main MUST have these parameters for SDL.
{
	if (init("GAME1007_M3", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0) == false)
		return 1;
	while (g_bRunning)
	{
		wake();
		handleEvents();
		update();
		render();
		if (g_bRunning)
			sleep();
	}
	clean();
	return 0;
}