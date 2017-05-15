'''
Usage:

  directorPython scripts/renderTrainingImages.py --bot-config $SPARTAN_SOURCE_DIR/apps/iiwa/iiwaManip.cfg --logFolder logs/moving-camera

Optionally you can pass --logFolder <logFolder> on the command line
where <logFolder> is the path to the lcm log folder relative to the
data folder.  For example: --logFolder logs/moving-camera
'''


from director import drcargs
from director import mainwindowapp
import corl.utils
from corl.rendertrainingimages import RenderTrainingImages


if __name__ == '__main__':
    parser = drcargs.getGlobalArgParser().getParser()
    parser.add_argument('--logFolder', type=str, dest='logFolder',
                        help='location of top level folder for this log, relative to CorlDev/data')
    args = parser.parse_args()

    app = mainwindowapp.construct()
    app.view.setParent(None)
    app.view.show()

    print "logFolder = ", args.logFolder

    pathDict = corl.utils.getFilenames(args.logFolder)
    rti = RenderTrainingImages(app.view, app.viewOptions, pathDict)
    rti.renderAndSaveLabeledImages()
