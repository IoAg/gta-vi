
# Ground Truth Annotation-Visualization Interface for Eye Movements

**For the 360 video extension documentation click [here](README_360.md)**

## 1. Introduction

In eye movement research with dynamic natural scenes there is a great need for
ground thruth since the stimuli are not artificially created and their
properties are not known. Even though many automatic algorithms exist for
annotating samples from streams to fixation, saccades, smooth pursuit etc., hand
labelling is still considered the best for those tasks especially when we are
talking about complex eye movements such as smooth pursuits. This interface
combines the results from many automatic algorithms, placed in columns of an
ARFF file (explained below), and allows the user to fine tune the intervals.

## 2. ARFF files

The ARFF format is widely used in the pattern recognition community and
documentation is available at
`https://waikato.github.io/weka-wiki/arff_stable/`. A brief overview with some
tool-specific variations is given below:

- All the data keywords are case insensitive

- Lines starting with "%" are comments

- Lines starting with "%" and followed by "@METADATA" are considered special
comments "%@METADATA" followed by the metadata name followed by the value. The
available metadata names are *width_px, height_px, distance_mm, width_mm, 
height_mm*.

- Lines starting with "@" followed by a word, without space, are considered
keywords. The available keywords are the following: 
    - "@ATTRIBUTE" followed by the attribute name followed by the type of data.
    The tool curently support *INTEGER*, *NUMERIC* and *NOMINAL* attributes. The
    nominal attributes are handled internally as enumerations. Thus when you
    acces the loaded you will see integer values in their place. 
    - "@DATA" denotes that all the lines from next line onwards are data. The 
    data should follow the order that the attributes were presented.

- Each automatic algorithm is assigned to an attribute

### 2.1. ARFF example

Part of example ARFF file with added explanation.

```
@RELATION gaze_labels <-data included in arff

%@METADATA width_px 1280 <-recording metadata
%@METADATA height_px 720
%@METADATA distance_mm 450
%@METADATA width_mm 400
%@METADATA height_mm 230

@ATTRIBUTE time NUMERIC
@ATTRIBUTE x NUMERIC
@ATTRIBUTE y NUMERIC
@ATTRIBUTE dbscan NUMERIC <-smooth pursuit detection algorithm
@ATTRIBUTE i-vvt NUMERIC <-saccade detection algorithm
@ATTRIBUTE i-vdt NUMERIC <-fixation detection algorithm
@ATTRIBUTE i-dt NUMERIC <-fixation detection algorithm
@ATTRIBUTE expert1 NUMERIC <-hand-tuned output


@DATA <- after this point the actual data start
2000,620.80,289.20,0,0,0,0,0
6000,622.70,291.30,0,0,0,1,1 
10000,622.90,292.10,0,0,0,1,1
14000,623.00,290.90,0,0,1,1,1
18000,623.00,289.70,0,0,1,1,1
```

### 2.2. ARFF conversion from .coord

See the coord2arff subdirectory for a MATLAB/Octave tool to convert
the GazeCom .coord gaze file format to ARFF.

## 3. License

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see `http://www.gnu.org/licenses/`
license - GPL.

## 4. Citing GTA-VI

We would ask you to cite the following paper if you use GTA-VI

Ioannis Agtzidis, Mikhail Startsev and Michael Dorr,
In the pursuit of (ground) truth: A hand-labelling tool for eye movements
recorded during dynamic scene viewing, ETVIS 2016

> \@article{Agtzidis2016InTP,<br/>
> title={In the pursuit of (ground) truth: a hand-labelling tool for eye movements recorded during dynamic scene viewing},<br/> 
> author={Ioannis Agtzidis and Mikhail Startsev and Michael Dorr},<br/>
> journal={2016 IEEE Second Workshop on Eye Tracking and Visualization (ETVIS)},<br/>
> year={2016},<br/>
> pages={65-68}<br/>
> }  

## 5. Installation

The code of this interface was tested with Ubuntu 16.04 LTS and it should work
with other Linux environments. Before you start, make sure the following
packages are installed:

```
apt-get install qt5-default
apt-get install libavutil-dev
apt-get install libavformat-dev
apt-get install libavcodec-dev
apt-get install libswscale-dev
```

If all of the above were installed successfully and the Qt version is 5.3 or
above, move to the root directory within the repository that you cloned and run
the following:

```
cd GTA-VI
qmake
make
```

Good luck! ;-)

## 6. Examples

If the above installation steps exited without error, you are ready to use the
interface.
Move to the GTA-VI directory and run ./GTA-VI -h for help. Three example
commands are given below (samples from [1] are provided in the samples directory):

- Start interface and load data with mouse
```
./GTA-VI
```  

- Start interface already preloaded with labelled data and video
```
./GTA-VI --vf ../samples/breite_strasse.m2t --af ../samples/MKB_breite_strasse_1_20s_1.arff --sf test.arff --pl myname
```

- Start interface as above which custom nominal labels
```
./GTA-VI --vf ../samples/breite_strasse.m2t --af ../samples/MKB_breite_strasse_1_20s_1.arff --sf test.arff --pl myname --plv {unassigned, fixation, saccade, noise}
```

| Short Option | Long Option | Explanation |
| ------ | ------ | ------ |
|-h| \-\-help                            | Displays this help.                  |   
|-v| \-\-version                         | Displays version information.            |
|\-\-vf| \-\-video-file \<file\>             | Video file.                            |
|\-\-af| \-\-arff-file \<file\>              | ARFF file.                             |
|\-\-sf| \-\-save-file \<file\>              | Save file.                             |
|\-\-pl| \-\-primary-label \<name\>          | Primary hand labelling attribute name. |
|\-\-plv| \-\-primary-label-value \<name\>   | (Optional) Create a nominal primary labelling attribute. Ex.     "{fix,sacc,sp}".|
|\-\-sl| \-\-secondary-label \<name\>        | (Optional) Secondary hand labelling attribute name.|
|\-\-slv| \-\-secondary-label-value \<name\> | (Optional) Create a nominal secondary labelling attribute. E    x. "{fix,sacc,sp}".|                     
|-f| \-\-full-screen                     | Start window in full screen mode.|   
|\-\-fov| \-\-field-of-view                | Convert Equirectangular video to Field Of View|
|\-\-head| \-\-head-only-motion            | Display only head motion in the equirectangular video|

## 7. GTA-VI functionality

Most of the user interaction is achieved through the interactive panels 
of the interface and keyboard shortcuts. If a secondary label is used a a third
color coded row appears at the bottom of the interface representing the eye
movements of the secondary label. The X,Y and speed panels change the primary
label and the two optional bottom panels the secondary label. A list of actions is
presented below:

- Right-clicking and dragging moves the current position in time (backwards or
forwards according to the direction of the mouse movement)

- Scrolling the mouse wheel changes the temporal scale, i.e.  increases or
decreases (according to the scroll direction) the temporal window represented by
the plots on the right-side panels

- Left-clicking and dragging moves the closest border

- Holding the left-click on an interval and pressing a number on the keyboard
changes the label of the interval. The legend provides information on the
correspondence between numbers and the assigned labels

- The sequence of a left-click and  pressing the
Insert key inserts a new interval of the selected type spanning a temporal
window of +/- 40 ms around the current time; this interval can then be
adjusted as above. The type of inserted interval is selected through the
keyboard number press

- Double left click on the secondary label panels adds an interval that matches
the borders of the primary interval above it. It does not overwrite secondary
labels if they exist. The type of inserted interval is selected through the 
keyboard number press

- The sequence of a left-click and pressing the Delete key unassigns the label
of the selected interval

- Pressing the Space key starts playing or pauses the video

- Pressing Ctrl-Z reverts the last change

- Pressing Ctrl-Shift-Z acts as “redo” (reapplies the last cancelled change)

## 8. References

[1] Michael Dorr, Thomas Martinetz, Karl Gegenfurtner, and Erhardt Barth.
Variability of eye movements when viewing dynamic natural scenes. Journal of
Vision, 10(10):1-17, 2010    

http://jov.arvojournals.org/article.aspx?articleid=2121333

## 9. Contact

For feedback please contact Ioannis Agtzidis at ioannis.agtzidis@tum.de

