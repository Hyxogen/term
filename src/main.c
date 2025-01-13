#define WIDTH 400
#define HEIGHT 400

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_surface.h>

#include <stdio.h>
#include <assert.h>
#include <sterm/sterm.h>

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Surface *surface;
static struct framebuf fb;
static struct sterm term;

SDL_AppResult SDL_AppIterate(void *appstate)
{
	(void)appstate;
	SDL_UpdateWindowSurface(window);
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	switch (event->type) {
	case SDL_EVENT_QUIT:
		return SDL_APP_SUCCESS;
	case SDL_EVENT_KEY_DOWN: {
		unsigned char key = event->key.key;
		printf("got key '0x%02x'\n", key);
		if (key == SDLK_RETURN)
			sterm_write(&term, "\r\n", 2);
		else
			sterm_write(&term, &key, 1);
		break;
	}
	}
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
	if (!SDL_SetAppMetadata("Simple terminal", "1.0", "hyxo"))
		return SDL_APP_FAILURE;

	if (!SDL_CreateWindowAndRenderer("sterm", WIDTH, HEIGHT, 0, &window, &renderer))
		return SDL_APP_FAILURE;

	surface = SDL_GetWindowSurface(window);
	if (!surface)
		return SDL_APP_FAILURE;

	fb.width = (unsigned)surface->w;
	fb.height = (unsigned)surface->h;
	fb.pitch = (unsigned)surface->pitch;
	fb.pixels = surface->pixels;
	fb.bpp = 32;
	fb.red_mask_size = 8;
	fb.red_mask_shift = 16;
	fb.green_mask_size = 8;
	fb.green_mask_shift = 8;
	fb.blue_mask_size = 8;
	fb.blue_mask_shift = 0;

	int rc = font_read_from(&fb.font, psf2_default_font, psf2_default_font_len);
	assert(!rc);

	printf("pixel format: '%s'\n", SDL_GetPixelFormatName(surface->format));

	sterm_init(&term, &fb_ops, &fb);

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}
