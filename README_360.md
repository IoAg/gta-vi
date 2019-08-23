
# Ground Truth Annotation-Visualization Interface for Eye Movements in 360-degree Videos

**For the documentation of the tool refer to the _Main Page_**

## 1. Introduction

Here we explain the specifics of the 360-degree extension to the original GTA-VI tool. This
extension allows the visualization and annotation of data gathered during 360-degree 
experiments. The experimental stimuli used so far is monocular 360-degree
equirectangular videos. The eye tracking recordings are stored in the previously
explained ARFF format but it is extended in order to be able to represent the
state of the HMD experiment.

## 2. ARFF files

Here the "@relation" is set to gaze_360 to distinguish the recordings from plain
gaze recordings. We also make use of the "%@METADATA" special comments which
describe the field of view of the used headset. The extra metadata names are
*fov_width_px, fov_width_deg, fov_height_px, fov_height_deg*. Although none of
the extra metadata are strictly needed for gaze visualization in the FOV
representation, they bring the representation closer to what was displayed
during the experiment.


### 2.1. ARFF example

```
@RELATION gaze_360

%@METADATA distance_mm 0.00
%@METADATA height_mm 0.00
%@METADATA height_px 1080
%@METADATA width_mm 0.00
%@METADATA width_px 1920

%@METADATA fov_height_deg 100.00
%@METADATA fov_height_px 1440
%@METADATA fov_width_deg 100.00
%@METADATA fov_width_px 1280

@ATTRIBUTE time INTEGER
@ATTRIBUTE x NUMERIC
@ATTRIBUTE y NUMERIC
@ATTRIBUTE confidence NUMERIC
@ATTRIBUTE x_head NUMERIC
@ATTRIBUTE y_head NUMERIC
@ATTRIBUTE angle_deg_head NUMERIC
@ATTRIBUTE labeller_1 {unassigned,fixation,saccade,SP,noise,VOR,OKN}


@DATA
0,960.00,540.00,1.00,960.00,540.00,1.22,fixation
5000,959.00,539.00,1.00,959.00,539.00,1.23,fixation
13000,959.00,539.00,1.00,959.00,539.00,1.23,fixation
18000,959.00,539.00,1.00,959.00,539.00,1.23,fixation
29000,959.00,539.00,1.00,959.00,539.00,1.24,fixation
34000,959.00,539.00,1.00,959.00,539.00,1.24,fixation
45000,959.00,539.00,1.00,959.00,539.00,1.24,fixation
49000,959.00,539.00,1.00,959.00,539.00,1.24,fixation
61000,959.00,539.00,1.00,959.00,539.00,1.24,fixation
66000,959.00,539.00,1.00,959.00,539.00,1.24,fixation
77000,959.00,539.00,1.00,959.00,540.00,1.24,fixation
82000,959.00,539.00,1.00,959.00,540.00,1.24,fixation
94000,959.00,539.00,1.00,960.00,540.00,1.24,fixation
99000,959.00,539.00,1.00,960.00,540.00,1.24,fixation
110000,959.00,539.00,1.00,960.00,540.00,1.25,fixation
114000,959.00,539.00,1.00,960.00,540.00,1.25,fixation
125000,958.00,538.00,1.00,960.00,540.00,1.26,saccade
129000,956.00,537.00,1.00,960.00,540.00,1.27,saccade
141000,948.00,530.00,1.00,960.00,540.00,1.28,saccade
```

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
along with this program.  If not, see <http://www.gnu.org/licenses/>.
license - GPL

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

## 5. Citing GTA-VI 360-degree extension

We would ask you to cite the following paper if you use GTA-VI tool along with
its 360-degree extension

%TODO will be added in the future

## 6. Use with CUDA

The extension supports CUDA accelerated conversion from equirectangular to field-of-view
frames. In order to use this functionality you will have to change the *GTA-VI.pro* file.

First you have to find the compute architecture of you GPU card, which is provided
in <https://developer.nvidia.com/cuda-gpus>. The compute architecture number is 
derived from the compute capability by removing the dot between the 2 numbers.

Second in the *GTA-VI.pro* file uncomment the *CONFIG += USE_CUDA*, place the number from
the previous step in the *CUDA_COMPUTE_ARCH* variable and finally provide the installation path
of CUDA in the *CUDA_DIR* variable.

Now you are ready to recompile your project.

```
qmake
make clean
make -j4
```

If things do not compile or work as expected you can take a look at *QMAKE_LIBDIR* and 
*INCLUDEPATH* in the *GTA-VI.pro* and fix possible errors.

If you still cannot make it work you can use the CPU implementation, by commenting 
the "#CONFIG += USE_CUDA" line, which provides the same funcitonality but slower.

## 7. Examples

If you have installed the tool succesfully you can try to run the following
examples.

The 360-degree extension of the tool is only available through the command line
interface.

Move to the GTA-VI directory and run ./GTA-VI -h for help. Some example
commands are given below:

```
./GTA-VI --vf 360_video.mp4 --af recoding1.arff --pl ioannis_pl --plv "{unassigned, fix, sacc, sp, noise}" --sl ioannis_sl --slv "{unassigned, okn, vor, noise}" --sf test1.arff --fov -f
```

```
./GTA-VI --vf 360_video.mp4 --af recoding1.arff --pl ioannis_pl --plv "{unassigned, fix, sacc, sp, noise}" --sf test1.arff -f
```

The explanation of the command line a arguments as return from the tool is given below:

| Short Option | Long Option | Explanation |
| ------ | ------ | ------ |
|-h| \-\-help                            | Displays this help.					|
|-v| \-\-version                         | Displays version information.			|
|\-\-vf| \-\-video-file <file>             | Video file.							|
|\-\-af| \-\-arff-file <file>              | ARFF file.								|
|\-\-sf| \-\-save-file <file>              | Save file.								|
|\-\-pl| \-\-primary-label <name>          | Primary hand labelling attribute name.	|
|\-\-plv| \-\-primary-label-value <name>   | (Optional) Create a nominal primary labelling attribute. Ex. "{fix,sacc,sp}".|
|\-\-sl| \-\-secondary-label <name>        | (Optional) Secondary hand labelling attribute name.|
|\-\-slv| \-\-secondary-label-value <name> | (Optional) Create a nominal secondary labelling attribute. Ex. "{fix,sacc,sp}".|
|\-\-sw| \-\-speed-window \<double\> | The duration of the window over which we filter the speed signal. The default value is 100000 us.|                     
|-f| \-\-full-screen                     | Start window in full screen mode.|
|\-\-fov| \-\-field-of-view                | Convert Equirectangular video to Field Of View|
|\-\-head| \-\-head-only-motion            | Display only head motion in the equirectangular video|

## 8. GTA-VI 360-degree functionality

The current extension displays gaze/head coordinates along with their speed.
Without any argument the head+eye gaze trace is displayed on all 4 panels along
with its overlay on the equirectangular. 

By passing the \-\-fov argument the gaze within head (FOV) is diplayed, which
is equivalent to the movement of the eye within the eye socket. Also with the
\-\-fov option the video panel displays only the part of the video that was
displayed during the experiment in the head mounted display. With this option 
you can toggle to the head+eye view and back by pressing the "t" key.

With the \-\-head argument only the head coordinates in the equirectangular
video is displayed. This last option is useful mostly for visualization of head
movement and to a lesser extend for eye movement classification.

## 9. References

[1] Agtzidis, Ioannis, Mikhail Startsev, and Michael Dorr. "In the pursuit of
(ground) truth: A hand-labelling tool for eye movements recorded during dynamic
scene viewing." Eye Tracking and Visualization (ETVIS), IEEE Second Workshop on.
IEEE, 2016.

## 10. Contact

For feedback please contact Ioannis Agtzidis at ioannis.agtzidis@tum.de

