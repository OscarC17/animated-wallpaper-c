#define _POSIX_C_SOURCE 199309L
//#define DEBUG

#include <Imlib2.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>

typedef struct {
  Window root;
  Pixmap pixmap;
  Imlib_Context *render_context;
  int width, height;
} Monitor;

typedef struct {
	SDL_Texture** texture;
	unsigned size;
} Texture;

void setRootAtoms(Display *display, Monitor *monitor) {
  Atom atom_root, atom_eroot, type;
  unsigned char *data_root, *data_eroot;
  int format;
  unsigned long length, after;

  atom_root = XInternAtom(display, "_XROOTMAP_ID", True);
  atom_eroot = XInternAtom(display, "ESETROOT_PMAP_ID", True);

  // doing this to clean up after old background
  if (atom_root != None && atom_eroot != None) {
    XGetWindowProperty(display, monitor->root, atom_root, 0L, 1L, False,
                       AnyPropertyType, &type, &format, &length, &after,
                       &data_root);

    if (type == XA_PIXMAP) {
      XGetWindowProperty(display, monitor->root, atom_eroot, 0L, 1L, False,
                         AnyPropertyType, &type, &format, &length, &after,
                         &data_eroot);

      if (data_root && data_eroot && type == XA_PIXMAP &&
          *((Pixmap *)data_root) == *((Pixmap *)data_eroot))
        XKillClient(display, *((Pixmap *)data_root));
    }
  }

  atom_root = XInternAtom(display, "_XROOTPMAP_ID", False);
  atom_eroot = XInternAtom(display, "ESETROOT_PMAP_ID", False);

  // setting new background atoms
  XChangeProperty(display, monitor->root, atom_root, XA_PIXMAP, 32,
                  PropModeReplace, (unsigned char *)&monitor->pixmap, 1);
  XChangeProperty(display, monitor->root, atom_eroot, XA_PIXMAP, 32,
                  PropModeReplace, (unsigned char *)&monitor->pixmap, 1);
}

int main(int argc, char *argv[]) {

	char * filedir = "./sample";
	float speed = 1;

    int opt;
    while ((opt = getopt(argc, argv, "i:s:")) != -1) {
        switch (opt) {
        case 'i': filedir = optarg; break;
        case 's': speed = atof(optarg); break;
        default:
            fprintf(stderr, "Usage: %s -i folder -s speed\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

	//printf("filedir = %s\nspeed = %f\nloop = %d\n", filedir, speed, loop);

	//printf("%s\n", filedir);

	int dirsize = 0;
	struct dirent *de; 

	DIR *dr = opendir(filedir);

	if (dr == NULL) {
		printf("Could not open current directory" );
		return 0;
	}
	while ((de = readdir(dr)) != NULL) {
		if (de->d_name[0] != '.') {
			dirsize++;
		}
	}
	closedir(dr);    

	Imlib_Image images[dirsize];

	SDL_Surface surfaces[dirsize];


	for (int i = 0; i < dirsize; i++) {
		char imgstring[500] = " ";
		sprintf(imgstring, "%s/%d.bmp", filedir, i + 1);
		images[i] = imlib_load_image(imgstring);
		//surfaces[i] = IMG_Load(imgstring);
	}


	int images_count = dirsize;

#ifdef DEBUG
  fprintf(stdout, "Loading images");
#endif

#ifdef DEBUG
  fprintf(stdout, "Loading monitors\n");
#endif

  Display *display = XOpenDisplay(NULL);
  if (!display) {
    fprintf(stderr, "Could not  open XDisplay\n");
    exit(42);
  }

  const int screen_count = ScreenCount(display);
#ifdef DEBUG
  fprintf(stdout, "Found %d screens\n", screen_count);
#endif

  Monitor *monitors = malloc(sizeof(Monitor) * screen_count);
  for (int current_screen = 0; current_screen < screen_count;
       ++current_screen) {
#ifdef DEBUG
    fprintf(stdout, "Running screen %d\n", current_screen);
#endif

    const int width = DisplayWidth(display, current_screen);
    const int height = DisplayHeight(display, current_screen);
    const int depth = DefaultDepth(display, current_screen);
    Visual *vis = DefaultVisual(display, current_screen);
    const int cm = DefaultColormap(display, current_screen);

#ifdef DEBUG
    fprintf(stdout, "Screen %d: width: %d, height: %d, depth: %d\n",
            current_screen, width, height, depth);
#endif

    Window root = RootWindow(display, current_screen);
    Pixmap pixmap = XCreatePixmap(display, root, width, height, depth);

    monitors[current_screen].width = width;
    monitors[current_screen].height = height;
    monitors[current_screen].root = root;
    monitors[current_screen].pixmap = pixmap;
    monitors[current_screen].render_context = imlib_context_new();
    imlib_context_push(monitors[current_screen].render_context);
    imlib_context_set_display(display);
    imlib_context_set_visual(vis);
    imlib_context_set_colormap(cm);
    imlib_context_set_drawable(pixmap);
    imlib_context_set_color_range(imlib_create_color_range());
    imlib_context_pop();
  }

#ifdef DEBUG
  fprintf(stdout, "Loaded %d screens\n", screen_count);
#endif

#ifdef DEBUG
  fprintf(stdout, "Starting render loop");
#endif

  struct timespec timeout;
  timeout.tv_sec = 0;
  timeout.tv_nsec = 33000000 * speed; //~30fps by default

  while (true) {
	  for (int cycle = 0; cycle < images_count; ++cycle) {
		Imlib_Image current = images[cycle % images_count];
		for (int monitor = 0; monitor < screen_count; ++monitor) {
		  Monitor *c_monitor = &monitors[monitor];
		  imlib_context_push(c_monitor->render_context);
		  imlib_context_set_dither(1);
		  imlib_context_set_blend(1);
		  imlib_context_set_image(current);

		  imlib_render_image_on_drawable(0, 0);

		  setRootAtoms(display, c_monitor);
		  XKillClient(display, AllTemporary);
		  XSetCloseDownMode(display, RetainTemporary);
		  XSetWindowBackgroundPixmap(display, c_monitor->root, c_monitor->pixmap);
		  XClearWindow(display, c_monitor->root);
		  XFlush(display);
		  XSync(display, False);
		  imlib_context_pop();
		}
		nanosleep(&timeout, NULL);
	  }
  }
}
