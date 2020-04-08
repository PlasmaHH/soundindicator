# soundindicator
Interface to a cheap digital dial indicator and a 3d printer

---

***NOTE:***
This project is not intended to be maintained for public use, it may serve as an inspiration for others though. 
Pull requests are welcome, feature requests and bug reports will only be worked on when they annoy me enough.

---


With it (and gnuplot) you can easily create a graph like this, which shows the error of an ender 3 pro Z axis movement:


![](img/fine_20mm_flat.png)

```
Valid options:
  -h [ --help ]                         produce this help message
  -V [ --version ]                      Show the program version
  -v [ --verbose ] arg                  Increase the initial logging level (may
                                        be overridden by a config file)
  -b [ --banner ] [=arg(=1)]            Indicator output in banner mode
  -m [ --mode ] arg                     Operating mode: plot
  -a [ --average ] arg (=1)             Average over that many runs in plot 
                                        mode
  -A [ --axis ] arg                     The axis to use in plot mode
  -s [ --start ] arg (=0)               Start movement at this point (mm)
  -e [ --end ] arg (=2)                 End movement at this point (mm)
  -S [ --steps ] arg (=0.00999999978)   The steps to be done per measurement 
                                        (mm), the absolute value will be used 
                                        in the proper direction
  -B [ --bidirectional ] [=arg(=1)] (=0)
                                        Do the movement in both directions
  -p [ --preposition ] arg              Before driving to the start, drive to 
                                        this position (simulates auto homing 
                                        and hop movements)
  -F [ --speed ] arg (=10)              The speed to move with, given to the F 
                                        parameter of G1
  -r [ --stable ] arg (=0)              For each reading wait to stabilize for 
                                        that many readings first. Makes 
                                        measurements slower, can prevent them 
                                        totally if you have a too high value
  -d [ --device ] arg (=/dev/ttyACM0)   The serial port device the printer is 
                                        attached to
  -g [ --gcode ] arg                    Some gcode to execute before everyhting
                                        else

```
