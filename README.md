Program creates png series of game of life, that could be easy turned to gif animation, requires libpng

1) create gamein.png file with black background and white pixels, could be any size
2) run the code
3) make a gif with command: convert -delay 10 -loop 0 gameout_*.png animation.gif

And in the animation.gif is a game of life result.
