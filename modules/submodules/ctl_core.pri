# specify folder for the executable
CONFIG(debug, debug|release) {
    DESTDIR = bin
} else {
    DESTDIR = bin
}

# declare module
CONFIG += CTL_CORE_MODULE

# enable logging function names and line numbers even for release builds
DEFINES += QT_MESSAGELOGCONTEXT
# disable min/max macros in Windows headers
DEFINES += NOMINMAX

INCLUDEPATH += $$PWD/../src

# Qt-dependent headers and sources
HEADERS += \
    $$PWD/../src/acquisition/abstractpreparestep.h \
    $$PWD/../src/acquisition/acquisitionsetup.h \
    $$PWD/../src/acquisition/ctsystem.h \
    $$PWD/../src/acquisition/ctsystembuilder.h \
    $$PWD/../src/acquisition/geometrydecoder.h \
    $$PWD/../src/acquisition/geometryencoder.h \
    $$PWD/../src/acquisition/preparationprotocols.h \
    $$PWD/../src/acquisition/preparesteps.h \
    $$PWD/../src/acquisition/simplectsystem.h \
    $$PWD/../src/acquisition/systemblueprints.h \
    $$PWD/../src/acquisition/trajectories.h \
    $$PWD/../src/acquisition/viewgeometry.h \
    $$PWD/../src/components/abstractbeammodifier.h \
    $$PWD/../src/components/abstractdetector.h \
    $$PWD/../src/components/abstractgantry.h \
    $$PWD/../src/components/abstractsource.h \
    $$PWD/../src/components/allcomponents.h \
    $$PWD/../src/components/allgenerictypes.h \
    $$PWD/../src/components/carmgantry.h \
    $$PWD/../src/components/cylindricaldetector.h \
    $$PWD/../src/components/flatpaneldetector.h \
    $$PWD/../src/components/genericbeammodifier.h \
    $$PWD/../src/components/genericdetector.h \
    $$PWD/../src/components/genericgantry.h \
    $$PWD/../src/components/genericsource.h \
    $$PWD/../src/components/systemcomponent.h \
    $$PWD/../src/components/tubulargantry.h \
    $$PWD/../src/components/xraylaser.h \
    $$PWD/../src/components/xraytube.h \
    $$PWD/../src/img/abstractdynamicvoxelvolume.h \
    $$PWD/../src/img/chunk2d.h \
    $$PWD/../src/img/compositevolume.h \
    $$PWD/../src/img/modulelayout.h \
    $$PWD/../src/img/projectiondata.h \
    $$PWD/../src/img/singleviewdata.h \
    $$PWD/../src/img/spectralvolumedata.h \
    $$PWD/../src/img/voxelvolume.h \
    $$PWD/../src/img/trivialdynamicvolume.h \
    $$PWD/../src/io/abstractbasetypeio.h \
    $$PWD/../src/io/abstractserializer.h \
    $$PWD/../src/io/basetypeio.h \
    $$PWD/../src/io/binaryserializer.h \
    $$PWD/../src/io/ctldatabase.h \
    $$PWD/../src/io/jsonserializer.h \
    $$PWD/../src/io/messagehandler.h \
    $$PWD/../src/io/metainfokeys.h \
    $$PWD/../src/io/serializationhelper.h \
    $$PWD/../src/io/serializationinterface.h \
    $$PWD/../src/mat/homography.h \
    $$PWD/../src/mat/mat.h \
    $$PWD/../src/mat/matrix_algorithm.h \
    $$PWD/../src/mat/matrix_types.h \
    $$PWD/../src/mat/matrix_utils.h \
    $$PWD/../src/mat/pi.h \
    $$PWD/../src/mat/pmatcomparator.h \
    $$PWD/../src/mat/projectionmatrix.h \
    $$PWD/../src/models/abstractdatamodel.h \
    $$PWD/../src/models/detectorsaturationmodels.h \
    $$PWD/../src/models/intervaldataseries.h \
    $$PWD/../src/models/pointseriesbase.h \
    $$PWD/../src/models/stepfunctionmodels.h \
    $$PWD/../src/models/tabulateddatamodel.h \
    $$PWD/../src/models/xrayspectrummodels.h \
    $$PWD/../src/models/xydataseries.h \
    $$PWD/../src/processing/abstractvolumedecomposer.h \
    $$PWD/../src/processing/errormetrics.h \
    $$PWD/../src/processing/diff.h \
    $$PWD/../src/processing/filter.h \
    $$PWD/../src/processing/imageprocessing.h \
    $$PWD/../src/processing/modelbasedvolumedecomposer.h \
    $$PWD/../src/projectors/abstractprojector.h \
    $$PWD/../src/projectors/abstractprojectorconfig.h \
    $$PWD/../src/projectors/arealfocalspotextension.h \
    $$PWD/../src/projectors/detectorsaturationextension.h \
    $$PWD/../src/projectors/dynamicprojector.h \
    $$PWD/../src/projectors/poissonnoiseextension.h \
    $$PWD/../src/projectors/projectorextension.h \
    $$PWD/../src/projectors/spectralprojectorextension.h \
    $$PWD/../src/models/datamodeloperations.h

SOURCES += \
    $$PWD/../src/acquisition/acquisitionsetup.cpp \
    $$PWD/../src/acquisition/ctsystem.cpp \
    $$PWD/../src/acquisition/ctsystembuilder.cpp \
    $$PWD/../src/acquisition/geometrydecoder.cpp \
    $$PWD/../src/acquisition/geometryencoder.cpp \
    $$PWD/../src/acquisition/preparationprotocols.cpp \
    $$PWD/../src/acquisition/preparesteps.cpp \
    $$PWD/../src/acquisition/simplectsystem.cpp \
    $$PWD/../src/acquisition/trajectories.cpp \
    $$PWD/../src/acquisition/viewgeometry.cpp \
    $$PWD/../src/components/carmgantry.cpp \
    $$PWD/../src/components/cylindricaldetector.cpp \
    $$PWD/../src/components/flatpaneldetector.cpp \
    $$PWD/../src/components/genericbeammodifier.cpp \
    $$PWD/../src/components/genericdetector.cpp \
    $$PWD/../src/components/genericgantry.cpp \
    $$PWD/../src/components/genericsource.cpp \
    $$PWD/../src/components/systemcomponent.cpp \
    $$PWD/../src/components/tubulargantry.cpp \
    $$PWD/../src/components/xraylaser.cpp \
    $$PWD/../src/components/xraytube.cpp \
    $$PWD/../src/img/chunk2d.tpp \
    $$PWD/../src/img/compositevolume.cpp \
    $$PWD/../src/img/projectiondata.cpp \
    $$PWD/../src/img/singleviewdata.cpp \
    $$PWD/../src/img/spectralvolumedata.cpp \
    $$PWD/../src/img/voxelvolume.tpp \
    $$PWD/../src/io/basetypeio.tpp \
    $$PWD/../src/io/jsonserializer.cpp \
    $$PWD/../src/io/serializationhelper.cpp \
    $$PWD/../src/io/binaryserializer.cpp \
    $$PWD/../src/io/ctldatabase.cpp \
    $$PWD/../src/io/messagehandler.cpp \
    $$PWD/../src/mat/homography.cpp \
    $$PWD/../src/mat/matrix_utils.tpp \
    $$PWD/../src/mat/matrix_algorithm.cpp \
    $$PWD/../src/mat/pmatcomparator.cpp \
    $$PWD/../src/mat/projectionmatrix.cpp \
    $$PWD/../src/models/detectorsaturationmodels.cpp \
    $$PWD/../src/models/intervaldataseries.cpp \
    $$PWD/../src/models/stepfunctionmodels.cpp \
    $$PWD/../src/models/tabulateddatamodel.cpp \
    $$PWD/../src/models/xrayspectrummodels.cpp \
    $$PWD/../src/models/xydataseries.cpp \
    $$PWD/../src/processing/errormetrics.cpp \
    $$PWD/../src/processing/filter.cpp \
    $$PWD/../src/processing/imageprocessing.cpp \
    $$PWD/../src/processing/modelbasedvolumedecomposer.cpp \
    $$PWD/../src/projectors/arealfocalspotextension.cpp \
    $$PWD/../src/projectors/detectorsaturationextension.cpp \
    $$PWD/../src/projectors/dynamicprojector.cpp \
    $$PWD/../src/projectors/poissonnoiseextension.cpp \
    $$PWD/../src/projectors/spectralprojectorextension.cpp \
    $$PWD/../src/models/abstractdatamodel.cpp \
    $$PWD/../src/models/datamodeloperations.cpp

# Qt-free headers and sources
HEADERS += \
    $$PWD/../src/mat/deg.h \
    $$PWD/../src/mat/matrix.h \
    $$PWD/../src/models/copyableuniqueptr.h \
    $$PWD/../src/processing/coordinates.h

SOURCES += \
    $$PWD/../src/mat/matrix.tpp

# create a file that contains the absolute path to database
DATABASE_ROOT = $$PWD/../../database
QMAKE_SUBSTITUTES += indirect
indirect.input = $$PWD/../.database.path.in
indirect.output = $$DESTDIR/database.path
