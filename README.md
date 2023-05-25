# animated-wallpaper-c
___Original proof of concept by [AlecsFerra](https://gist.github.com/AlecsFerra/ef1cc008990319f3b676eb2d8aa89903)___

Draws a series of bmp images to the window root to create an animated wallpaper, very resource intensive.  

Building:  
```make```

Install:  
```sudo make install```

Uninstall:  
```sudo make uninstall```

Usage:  
```animatedwallpaper -i [folder with bmp images] -s [speed]```

Default speed is 30 fps, lower speed values will increase playback speed. ('-s 0.5' for 60 fps, '-s 2' for 15 fps, etc.)

The images must be in bmp format named 1.bmp, 2.bmp, ... n.bmp; stored in the same directory *with no other files*. An example of this can be seen in sample/. 
The images should be the same resolution as the monitor as the images will not scale. A series of images can be generated with the following ffmpeg command (This example is for a 1080p monitor with a 30fps video):  
  
```ffmpeg -i input.mp4 -vf scale=1920x1080,fps=30 %d.bmp```  



