=====
Setup
=====

Data folder
-----------

Create a symlink from the shared MIT Dropbox `CORL217` folder to `spartan/src/CorlDev/data`.
For example::

    ln -s $HOME/Dropbox/CORL2017 ./data

If Dropbox is hosted on another computer, you can use sshfs to mount the data from the remote computer::

    mkdir -p ./data  # create mount point
    sshfs user@hostname:Dropbox/CORL2017 ./data


Environment
-----------

Source the `setup_environment.sh` file.  This file appends CorlDev/modules
to the `PYTHONPATH` so that Python scripts can import the corl module.
The corl module contains reusable functions and utilities that are used by
the scripts.

Director
--------

Checkout the branch :code:`corl-master` from the repository :code:`github.com/manuelli/director.git`. This will serve as our internal director branch. The idea is that these changes will make their way back to director/master eventually, but that process shouldn't slow down our workflow.


ElasticFusion
-------------

- Install CUDA (we have verified that ElasticFusion works on CUDA 7.5 and 8.0)
- Install ElasticFusion::

	git clone git@github.com:peteflorence/ElasticFusion.git
	cd ElasticFusion
	./build.sh

The build script is great, but if it doesn't work, follow the build instructions in the README.md.

SegNet
------

- Install SegNet scripts::

	cd /
	git clone git@github.com:peteflorence/SegNet-Tutorial.git
	mv SegNet-Tutorial SegNet && cd SegNet
	git checkout pf-moving-camera

- Install caffe-segnet inside of SegNet and test, and install Python bindings::

	git clone git@github.com:alexgkendall/caffe-segnet.git
	cp Makefile.config.example Makefile.config
	# Adjust Makefile.config (for example, if using Anaconda Python, or if cuDNN is desired)
	make all
	make test
	make runtest
	make pycaffe

These are the key commands, but if you need full instructions, see http://mi.eng.cam.ac.uk/projects/segnet/tutorial.html and http://caffe.berkeleyvision.org/installation.html.  (Note that caffe-segnet is particular distribution of segnet, you do not need to separately install caffe.)


========
Pipeline
========

1. Collect RGBD data
--------------------
In first terminal, :code:`use_spartan` and then launch:

- :code:`kuka_iiwa_procman`
- Ctrl+R on vision-drivers --> openni-driver (unplug-replug if not working)
- Ctrl+R on bot-spy
- Verify that OPENNI_FRAME traffic is coming over lcm

In second terminal, :code:`use_spartan` and then:

- :code:`lcm-logger`
- Ctrl+C when done logging

Your data should now be saved as :code:`lcmlog-*`

2. Run RGBD data through ElasticFusion
--------------------------------------

Navigate to ElasticFusion executable (in :code:`ElasticFusion/GUI/build`) and then run::

	./ElasticFusion -l ~/Desktop/moving-camera.lcmlog  -f
	
Where :code:`~Desktop/moving-camera.lcmlog` is the full path to RGBD lcm data.  Note that :code:`-f` option flips the blue/green, which is needed.

When ElasticFusion is done running, the two key files it will save are:

- :code:`*.posegraph`
- :code:`*.ply`

3. Convert ElasticFusion .ply output to .vtp
--------------------------------------------

First, open the .ply file in Meshlab, and save it (this will convert to an ASCII .ply file)

Next, convert to .vtp using the command::

  directorPython scripts/convertPlyToVtp.py /path/to/data.ply

4. Global Object Pose Fitting
-----------------------------

The class that handles segmentation and registration is in :code:`modules/corl/registration.py` and :code:`modules/corl/objectalignmenttool.py`. Launch the standard :code:`corlApp` to run it::

	directorPython scripts/corlApp.py --logFolder logs/test --bot-config $SPARTAN_SOURCE_DIR/apps/iiwa/iiwaManip.cfg

The :code:`GlobalRegistration` object is in the global namespace as :code:`globalRegistration`. Run::

	globalRegistration.launchObjectAlignment(<objectName>)

where :code:`<objectName>` is a string like :code:`"oil_bottle"`. This launches a new window. Click the same three points in model and on pointcloud. Using :code:`shift + click` to do this. After you do this the affordance should appear in main window using the transform that was just computed. You can crop the pointcloud using the alignments we just got by calling::

	globalRegistration.cropPointCloudUsingAlignedObject(objectName=<objectName>)

.. commented out below
.. We need environment variables in order for the scripts to be able to find the binaries for these global fitting routines. Please fill in the variables like :code:`FGR_BASE_DIR` in :code:`setup_environment.sh` to point to your local binaries. The relevant python file is :code:`module/corl/registration.py`. To run an example::

.. 	drake-visualizer --script scripts/registration/testRegistration.py

.. Fitting phone using GlobalRegistration tool

.. 1. Launch :code:`kuka_iiwa_app`.
.. 2. open measurement panel and enable.
.. 3. shift + click on center of phone.
.. 4. execute :code:`globalRegistration.testPhoneFit()`. WARNING THIS IS SLOW.

.. This creates a cropped pointcloud of 8cm around your click point. Then it runs SuperPCS4 algorithm to fit phone mesh to this pointcloud. By default the phone mesh is downsampled.


5. Extract Images from LCM log
------------------------------
The class that is used is is :code:`modules/corl/imagecapture.py`. To extract rgb images from the lcm log run::

	directorPython scripts/extractImagesFromLog.py --logFolder logs/moving-camera --bot-config $SPARTAN_SOURCE_DIR/apps/iiwa/iiwaManip.cfg

This will save the images in :code:`data/logFolder`. The original images will be in the form :code:`uid_rbg.png`. Each image also has :code:`uid_utime.txt` which contains the utime associated with that image. Note that it will overwrite anything that is already there.


6. Generate Labeled Images
--------------------------

The class that is used to render labeled images is :code:`modules/corl/rendertrainingimages.py`. Usage::

  directorPython scripts/renderTrainingImages.py --bot-config $SPARTAN_SOURCE_DIR/apps/iiwa/iiwaManip.cfg --logFolder logs/moving-camera

Optionally you can pass :code:`--logFolder <logFolder>` on the command line where :code:`<logFolder>` is the path to the lcm log folder relative to the data folder.  For example :code:`--logFolder logs/moving-camera`. This will generate :code:`uid_labels.png` and :code:`uid_color_labels.png` which are the labeled images.

====
Misc
====

Director with Corl Modules
--------------------------
There is a standalone app for launching a director with corl modules::

	directorPython scripts/corlApp.py --logFolder logs/moving-camera --bot-config $SPARTAN_SOURCE_DIR/apps/iiwa/iiwaManip.cfg

The :code:`--logFolder` option specifies which logFolder to use relative to Corl data directory.

Visualizing RGBD Data
---------------------

You can launch director with imageviewapp. You need to pass the :code:`-c` flag to director along with the config file::
	
	cds && cd apps/iiwa
	directorPython -m director.imageviewapp -c iiwaManip.cfg --channel OPENNI_FRAME --rgbd --pointcloud
	
	
	
