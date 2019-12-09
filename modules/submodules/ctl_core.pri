!CTL_CORE_MODULE {

# include Qt-free sources
include(ctl_core_qtfree.pri)

# specify folder for the executable
CONFIG(debug, debug|release) {
    DESTDIR = bin
} else {
    DESTDIR = bin
}

# define source directory
CTL_SOURCE_DIR = $$PWD/../src

# declare module
CONFIG += CTL_CORE_MODULE

# enable logging function names and line numbers even for release builds
DEFINES += QT_MESSAGELOGCONTEXT

HEADERS += \
    $$CTL_SOURCE_DIR/acquisition/abstractpreparestep.h \
    $$CTL_SOURCE_DIR/acquisition/acquisitionsetup.h \
    $$CTL_SOURCE_DIR/acquisition/ctsystem.h \
    $$CTL_SOURCE_DIR/acquisition/ctsystembuilder.h \
    $$CTL_SOURCE_DIR/acquisition/geometrydecoder.h \
    $$CTL_SOURCE_DIR/acquisition/geometryencoder.h \
    $$CTL_SOURCE_DIR/acquisition/preparationprotocols.h \
    $$CTL_SOURCE_DIR/acquisition/preparesteps.h \
    $$CTL_SOURCE_DIR/acquisition/simplectsystem.h \
    $$CTL_SOURCE_DIR/acquisition/systemblueprints.h \
    $$CTL_SOURCE_DIR/acquisition/trajectories.h \
    $$CTL_SOURCE_DIR/acquisition/viewgeometry.h \
    $$CTL_SOURCE_DIR/components/abstractbeammodifier.h \
    $$CTL_SOURCE_DIR/components/abstractdetector.h \
    $$CTL_SOURCE_DIR/components/abstractgantry.h \
    $$CTL_SOURCE_DIR/components/abstractsource.h \
    $$CTL_SOURCE_DIR/components/allcomponents.h \
    $$CTL_SOURCE_DIR/components/allgenerictypes.h \
    $$CTL_SOURCE_DIR/components/carmgantry.h \
    $$CTL_SOURCE_DIR/components/cylindricaldetector.h \
    $$CTL_SOURCE_DIR/components/flatpaneldetector.h \
    $$CTL_SOURCE_DIR/components/genericbeammodifier.h \
    $$CTL_SOURCE_DIR/components/genericdetector.h \
    $$CTL_SOURCE_DIR/components/genericgantry.h \
    $$CTL_SOURCE_DIR/components/genericsource.h \
    $$CTL_SOURCE_DIR/components/systemcomponent.h \
    $$CTL_SOURCE_DIR/components/tubulargantry.h \
    $$CTL_SOURCE_DIR/components/xraylaser.h \
    $$CTL_SOURCE_DIR/components/xraytube.h \
    $$CTL_SOURCE_DIR/img/abstractdynamicvoxelvolume.h \
    $$CTL_SOURCE_DIR/img/chunk2d.h \
    $$CTL_SOURCE_DIR/img/compositevolume.h \
    $$CTL_SOURCE_DIR/img/modulelayout.h \
    $$CTL_SOURCE_DIR/img/projectiondata.h \
    $$CTL_SOURCE_DIR/img/singleviewdata.h \
    $$CTL_SOURCE_DIR/img/voxelvolume.h \
    $$CTL_SOURCE_DIR/img/trivialdynamicvolume.h \
    $$CTL_SOURCE_DIR/io/abstractbasetypeio.h \
    $$CTL_SOURCE_DIR/io/abstractserializer.h \
    $$CTL_SOURCE_DIR/io/basetypeio.h \
    $$CTL_SOURCE_DIR/io/binaryserializer.h \
    $$CTL_SOURCE_DIR/io/ctldatabase.h \
    $$CTL_SOURCE_DIR/io/jsonserializer.h \
    $$CTL_SOURCE_DIR/io/messagehandler.h \
    $$CTL_SOURCE_DIR/io/metainfokeys.h \
    $$CTL_SOURCE_DIR/io/serializationhelper.h \
    $$CTL_SOURCE_DIR/io/serializationinterface.h \
    $$CTL_SOURCE_DIR/mat/homography.h \
    $$CTL_SOURCE_DIR/mat/mat.h \
    $$CTL_SOURCE_DIR/mat/matrix_algorithm.h \
    $$CTL_SOURCE_DIR/mat/matrix_types.h \
    $$CTL_SOURCE_DIR/mat/matrix_utils.h \
    $$CTL_SOURCE_DIR/mat/pi.h \
    $$CTL_SOURCE_DIR/mat/pmatcomparator.h \
    $$CTL_SOURCE_DIR/mat/projectionmatrix.h \
    $$CTL_SOURCE_DIR/models/abstractdatamodel.h \
    $$CTL_SOURCE_DIR/models/detectorsaturationmodels.h \
    $$CTL_SOURCE_DIR/models/intervaldataseries.h \
    $$CTL_SOURCE_DIR/models/pointseriesbase.h \
    $$CTL_SOURCE_DIR/models/tabulateddatamodel.h \
    $$CTL_SOURCE_DIR/models/xrayspectrummodels.h \
    $$CTL_SOURCE_DIR/models/xydataseries.h \
    $$CTL_SOURCE_DIR/processing/errormetrics.h \
    $$CTL_SOURCE_DIR/processing/diff.h \
    $$CTL_SOURCE_DIR/processing/filter.h \
    $$CTL_SOURCE_DIR/processing/imageprocessing.h \
    $$CTL_SOURCE_DIR/projectors/abstractprojector.h \
    $$CTL_SOURCE_DIR/projectors/abstractprojectorconfig.h \
    $$CTL_SOURCE_DIR/projectors/arealfocalspotextension.h \
    $$CTL_SOURCE_DIR/projectors/detectorsaturationextension.h \
    $$CTL_SOURCE_DIR/projectors/dynamicprojector.h \
    $$CTL_SOURCE_DIR/projectors/projectorextension.h \
    $$CTL_SOURCE_DIR/projectors/poissonnoiseextension.h \
    $$CTL_SOURCE_DIR/projectors/spectralprojectorextension.h

SOURCES += \
    $$CTL_SOURCE_DIR/acquisition/acquisitionsetup.cpp \
    $$CTL_SOURCE_DIR/acquisition/ctsystem.cpp \
    $$CTL_SOURCE_DIR/acquisition/ctsystembuilder.cpp \
    $$CTL_SOURCE_DIR/acquisition/geometrydecoder.cpp \
    $$CTL_SOURCE_DIR/acquisition/geometryencoder.cpp \
    $$CTL_SOURCE_DIR/acquisition/preparationprotocols.cpp \
    $$CTL_SOURCE_DIR/acquisition/preparesteps.cpp \
    $$CTL_SOURCE_DIR/acquisition/simplectsystem.cpp \
    $$CTL_SOURCE_DIR/acquisition/trajectories.cpp \
    $$CTL_SOURCE_DIR/acquisition/viewgeometry.cpp \
    $$CTL_SOURCE_DIR/components/carmgantry.cpp \
    $$CTL_SOURCE_DIR/components/cylindricaldetector.cpp \
    $$CTL_SOURCE_DIR/components/flatpaneldetector.cpp \
    $$CTL_SOURCE_DIR/components/genericbeammodifier.cpp \
    $$CTL_SOURCE_DIR/components/genericdetector.cpp \
    $$CTL_SOURCE_DIR/components/genericgantry.cpp \
    $$CTL_SOURCE_DIR/components/genericsource.cpp \
    $$CTL_SOURCE_DIR/components/systemcomponent.cpp \
    $$CTL_SOURCE_DIR/components/tubulargantry.cpp \
    $$CTL_SOURCE_DIR/components/xraylaser.cpp \
    $$CTL_SOURCE_DIR/components/xraytube.cpp \
    $$CTL_SOURCE_DIR/img/chunk2d.tpp \
    $$CTL_SOURCE_DIR/img/compositevolume.cpp \
    $$CTL_SOURCE_DIR/img/projectiondata.cpp \
    $$CTL_SOURCE_DIR/img/singleviewdata.cpp \
    $$CTL_SOURCE_DIR/img/voxelvolume.tpp \
    $$CTL_SOURCE_DIR/io/basetypeio.tpp \
    $$CTL_SOURCE_DIR/io/jsonserializer.cpp \
    $$CTL_SOURCE_DIR/io/serializationhelper.cpp \
    $$CTL_SOURCE_DIR/io/binaryserializer.cpp \
    $$CTL_SOURCE_DIR/io/ctldatabase.cpp \
    $$CTL_SOURCE_DIR/io/messagehandler.cpp \
    $$CTL_SOURCE_DIR/mat/homography.cpp \
    $$CTL_SOURCE_DIR/mat/matrix_utils.tpp \
    $$CTL_SOURCE_DIR/mat/matrix_algorithm.cpp \
    $$CTL_SOURCE_DIR/mat/pmatcomparator.cpp \
    $$CTL_SOURCE_DIR/mat/projectionmatrix.cpp \
    $$CTL_SOURCE_DIR/models/detectorsaturationmodels.cpp \
    $$CTL_SOURCE_DIR/models/intervaldataseries.cpp \
    $$CTL_SOURCE_DIR/models/tabulateddatamodel.cpp \
    $$CTL_SOURCE_DIR/models/xrayspectrummodels.cpp \
    $$CTL_SOURCE_DIR/models/xydataseries.cpp \
    $$CTL_SOURCE_DIR/processing/errormetrics.cpp \
    $$CTL_SOURCE_DIR/processing/filter.cpp \
    $$CTL_SOURCE_DIR/processing/imageprocessing.cpp \
    $$CTL_SOURCE_DIR/projectors/arealfocalspotextension.cpp \
    $$CTL_SOURCE_DIR/projectors/detectorsaturationextension.cpp \
    $$CTL_SOURCE_DIR/projectors/dynamicprojector.cpp \
    $$CTL_SOURCE_DIR/projectors/poissonnoiseextension.cpp \
    $$CTL_SOURCE_DIR/projectors/spectralprojectorextension.cpp

# create a file that contains the absolute path to database
DATABASE_ROOT = $$PWD/../../database
QMAKE_SUBSTITUTES += indirect
indirect.input = $$PWD/../.database.path.in
indirect.output = $$DESTDIR/database.path

} # CTL_CORE_MODULE
