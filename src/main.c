#define FONT_SIZE 2
#define FONT_WIDTH 8
#define FONT_HEIGHT 8

#define COLS 80
#define ROWS 24
#define WIDTH (FONT_WIDTH*FONT_SIZE*COLS)
#define HEIGHT (FONT_HEIGHT*FONT_SIZE*ROWS)

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_surface.h>

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <sterm/sterm.h>
#include <pty.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Surface *surface;
static struct framebuf fb;
static struct term term;

int execvpe(const char *file, char *const argv[], char *const envp[]);

SDL_AppResult SDL_AppIterate(void *appstate)
{
	char buffer[512];

	ssize_t nread = 0;
	while ((nread = read(term.fd, buffer, sizeof(buffer))) > 0) {
		term_write(&term, buffer, nread);
	}

	if (nread < 0 && errno != EWOULDBLOCK) {
		perror("read");
		return SDL_APP_FAILURE;
	}

	if (nread == 0) {
		printf("pty closed");
		return SDL_APP_SUCCESS;
	}

	(void)appstate;
	SDL_UpdateWindowSurface(window);
	return SDL_APP_CONTINUE;
}

static char uppercase(char ch)
{
	switch (ch) {
	case '\\':
		return '|';
	case ';':
		return ':';
	case '1':
		return '!';
	case '/':
		return '?';
	default:
		return toupper(ch);
	}
}

static SDL_AppResult app_term_write(const void *data, size_t n)
{
	ssize_t rc = write(term.fd, data, n);
	if (rc < 0 && errno != EWOULDBLOCK) {
		perror("write");
		return SDL_APP_FAILURE;
	}
	if (rc == 0) {
		printf("pty closed");
		return SDL_APP_SUCCESS;
	}
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	switch (event->type) {
	case SDL_EVENT_QUIT:
		return SDL_APP_SUCCESS;
	case SDL_EVENT_KEY_DOWN: {
		unsigned key = event->key.key;

		if (key & 0x40000000) {
			switch (key) {
			case SDLK_UP:
				return app_term_write("\033[A", 3);
			case SDLK_DOWN:
				return app_term_write("\033[B", 3);
			case SDLK_RIGHT:
				return app_term_write("\033[C", 3);
			case SDLK_LEFT:
				return app_term_write("\033[D", 3);
			default:
				fprintf(stderr, "unknown special key 0x%x\n", key);
				break;
			}
		}

		if (key == SDLK_RETURN)
			key = '\n';
		if (key == SDLK_LSHIFT)
			break;

		unsigned char ch = (unsigned char) key;
		if (event->key.mod & SDL_KMOD_SHIFT) {
			ch = uppercase(key);
		}

		printf("got key '0x%02x'\n", ch);
		return app_term_write(&ch, 1);

		/*if (key == SDLK_RETURN)
			sterm_write(&term, "\r\n", 2);
		else
			sterm_write(&term, &key, 1);*/
		break;
	}
	}
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
	int rc = forkpty(&term.fd, NULL, NULL, NULL);
	if (rc < 0) {
		perror("forkpty");
		return SDL_APP_FAILURE;
	} else if (rc == 0) {
		const char *prog = "/bin/bash";
                char *const args[] = {
                    NULL,
                };
                char *const env[] = {
			"TERM=vt220",
		};

		execvpe(prog, args, env);
		perror("execvpe");
		return SDL_APP_FAILURE;
	}

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
	fb.font_size = FONT_SIZE;

	rc = font_read_from(&fb.font, psf2_default_font, psf2_default_font_len);
	assert(!rc);

	printf("pixel format: '%s'\n", SDL_GetPixelFormatName(surface->format));

	term_init(&term, &fb_ops, &fb);

	rc = fcntl(term.fd, F_SETFL, O_NONBLOCK);
	if (rc < 0) {
		perror("fcntl");
		return SDL_APP_FAILURE;
	}

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}
