# specify folder for the executable
CONFIG(debug, debug|release) {
    DESTDIR = bin
} else {
    DESTDIR = bin
}

# declare module
CONFIG += CTL_CORE_MODULE

# definition for mathematical constants
DEFINES += _USE_MATH_DEFINES

INCLUDEPATH += $$PWD/src

HEADERS += \
    $$PWD/src/components/abstractbeammodifier.h \
    $$PWD/src/components/abstractdetector.h \
    $$PWD/src/components/abstractgantry.h \
    $$PWD/src/components/abstractsource.h \
    $$PWD/src/components/allcomponents.h \
    $$PWD/src/components/allgenerictypes.h \
    $$PWD/src/components/carmgantry.h \
    $$PWD/src/components/cylindricaldetector.h \
    $$PWD/src/components/flatpaneldetector.h \
    $$PWD/src/components/genericbeammodifier.h \
    $$PWD/src/components/genericdetector.h \
    $$PWD/src/components/genericgantry.h \
    $$PWD/src/components/genericsource.h \
    $$PWD/src/components/jsonparser.h \
    $$PWD/src/components/systemcomponent.h \
    $$PWD/src/components/tubulargantry.h \
    $$PWD/src/components/xraylaser.h \
    $$PWD/src/components/xraytube.h \
    $$PWD/src/img/abstractdynamicvoxelvolume.h \
    $$PWD/src/img/chunk2d.h \
    $$PWD/src/img/modulelayout.h \
    $$PWD/src/img/projectiondata.h \
    $$PWD/src/img/singleviewdata.h \
    $$PWD/src/img/voxelvolume.h \
    $$PWD/src/img/trivialdynamicvolume.h \
    $$PWD/src/io/abstractbasetypeio.h \
    $$PWD/src/io/basetypeio.h \
    $$PWD/src/io/metainfokeys.h \
    $$PWD/src/mat/mat.h \
    $$PWD/src/mat/matrix.h \
    $$PWD/src/mat/matrix_algorithm.h \
    $$PWD/src/mat/matrix_types.h \
    $$PWD/src/mat/matrix_utils.h \
    $$PWD/src/mat/projectionmatrix.h \
    $$PWD/src/models/xrayspectrummodels.h \
    $$PWD/src/models/abstractdatamodel.h \
    $$PWD/src/projectors/abstractprojector.h \
    $$PWD/src/projectors/abstractprojectorconfig.h \
    $$PWD/src/projectors/arealfocalspotextension.h \
    $$PWD/src/projectors/dynamicprojector.h \
    $$PWD/src/projectors/projectorextension.h \
    $$PWD/src/projectors/poissonnoiseextension.h \
    $$PWD/src/acquisition/abstractpreparestep.h \
    $$PWD/src/acquisition/acquisitionsetup.h \
    $$PWD/src/acquisition/ctsystem.h \
    $$PWD/src/acquisition/ctsystembuilder.h \
    $$PWD/src/acquisition/fullgeometry.h \
    $$PWD/src/acquisition/geometrydecoder.h \
    $$PWD/src/acquisition/geometryencoder.h \
    $$PWD/src/acquisition/preparationprotocols.h \
    $$PWD/src/acquisition/preparesteps.h \
    $$PWD/src/acquisition/simplectsystem.h \
    $$PWD/src/acquisition/systemblueprints.h \
    $$PWD/src/acquisition/trajectories.h \
    $$PWD/src/models/tabulateddatamodel.h \
    $$PWD/src/models/xydataseries.h \
    $$PWD/src/models/pointseriesbase.h \
    $$PWD/src/models/intervaldataseries.h \
    $$PWD/src/mat/pmatcomparator.h \
    $$PWD/src/models/detectorsaturationmodels.h \
    $$PWD/src/projectors/detectorsaturationextension.h \
    $$PWD/src/io/jsonserializer.h

SOURCES += \
    $$PWD/src/components/carmgantry.cpp \
    $$PWD/src/components/cylindricaldetector.cpp \
    $$PWD/src/components/flatpaneldetector.cpp \
    $$PWD/src/components/genericdetector.cpp \
    $$PWD/src/components/genericgantry.cpp \
    $$PWD/src/components/genericsource.cpp \
    $$PWD/src/components/systemcomponent.cpp \
    $$PWD/src/components/tubulargantry.cpp \
    $$PWD/src/components/xraylaser.cpp \
    $$PWD/src/components/xraytube.cpp \
    $$PWD/src/img/chunk2d.tpp \
    $$PWD/src/img/projectiondata.cpp \
    $$PWD/src/img/singleviewdata.cpp \
    $$PWD/src/img/voxelvolume.tpp \
    $$PWD/src/io/basetypeio.tpp \
    $$PWD/src/mat/matrix.tpp \
    $$PWD/src/mat/matrix_utils.tpp \
    $$PWD/src/mat/matrix_algorithm.cpp \
    $$PWD/src/mat/projectionmatrix.cpp \
    $$PWD/src/models/xrayspectrummodels.cpp \
    $$PWD/src/projectors/arealfocalspotextension.cpp \
    $$PWD/src/projectors/dynamicprojector.cpp \
    $$PWD/src/projectors/poissonnoiseextension.cpp \
    $$PWD/src/acquisition/acquisitionsetup.cpp \
    $$PWD/src/acquisition/ctsystem.cpp \
    $$PWD/src/acquisition/ctsystembuilder.cpp \
    $$PWD/src/acquisition/geometrydecoder.cpp \
    $$PWD/src/acquisition/geometryencoder.cpp \
    $$PWD/src/acquisition/preparationprotocols.cpp \
    $$PWD/src/acquisition/preparesteps.cpp \
    $$PWD/src/acquisition/simplectsystem.cpp \
    $$PWD/src/acquisition/trajectories.cpp \
    $$PWD/src/models/tabulateddatamodel.cpp \
    $$PWD/src/models/xydataseries.cpp \
    $$PWD/src/models/intervaldataseries.cpp \
    $$PWD/src/mat/pmatcomparator.cpp \
    $$PWD/src/models/detectorsaturationmodels.cpp \
    $$PWD/src/projectors/detectorsaturationextension.cpp \
    $$PWD/src/io/jsonserializer.cpp
