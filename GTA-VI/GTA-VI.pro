######################################################################

## Select to compilte for debug or release
CONFIG += debug
#CONFIG += release

## Uncomment line below to use CUDA
CONFIG += USE_CUDA

## If you use CUDA you should find the compute capability of your GPU from the
## following list:
## https://developer.nvidia.com/cuda-gpus
## The compute architecture is the same as the compute capability without the dot
CUDA_COMPUTE_ARCH = 52
## Path to cuda toolkit install
CUDA_DIR = /usr/local/cuda-8.0

QT+=widgets 
TEMPLATE = app
TARGET = GTA-VI
INCLUDEPATH += .
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LIBS += -lavutil -lavformat -lavcodec -lswscale

## Enable profiling with gprof
#QMAKE_CXXFLAGS_DEBUG *= -pg
#QMAKE_LFLAGS_DEBUG *= -pg

## Setup NVCC for compilation
CONFIG (USE_CUDA) {
    ## add macro for conditional compilation in code
    DEFINES += USE_CUDA
    CUDA_DEFINES += USE_CUDA

    ## cuda source
    CUDA_SOURCES += EquirectangularToFovVideo.cu

    ## Path to cuda toolkit install
    INCLUDEPATH += $$CUDA_DIR/include

    ## 3rd party headers
    SYSTEMINCLUDEPATH = /usr/include/x86_64-linux-gnu/qt5/QtGui
    SYSTEMINCLUDEPATH += /usr/include/x86_64-linux-gnu/qt5/QtCore/
    SYSTEMINCLUDEPATH += /usr/include/x86_64-linux-gnu/qt5
    QMAKE_LIBDIR += $$CUDA_DIR/lib64

    ## SYSTEM_TYPE - compiling for 32 or 64 bit architecture
    SYSTEM_TYPE = 64

    ## correctly formats CUDA_COMPUTE_ARCH to CUDA_ARCH with code gen flags
    ## resulting format example: -gencode arch=compute_20,code=sm_20
    for(_a, CUDA_COMPUTE_ARCH):{
        formatted_arch =$$join(_a,'','-gencode arch=compute_',',code=sm_$$_a')
        CUDA_ARCH += $$formatted_arch
    }

    ## correctly formats CUDA_DEFINES for nvcc
    for(_defines, CUDA_DEFINES):{
        formatted_defines += -D$$_defines
    }
    CUDA_DEFINES = $$formatted_defines

    ## NVCC flags
    NVCCFLAGS += --compiler-options -fno-strict-aliasing -use_fast_math --ptxas-options=-v

    ## Path to libraries
    LIBS += -lcudart -lcuda

    ## join the includes in a line
    CUDA_INC = $$join(INCLUDEPATH,' -I','-I',' ')
    ## -isystem does not give warnings for vendor supplied headers
    CUDA_INC += $$join(SYSTEMINCLUDEPATH,' -isystem ','-isystem ',' ')

    ## NVCC_OPTIONS - any further options for the compiler
    NVCC_OPTIONS += -Xcompiler -fPIC -std=c++11

    CONFIG(debug, debug|release) {
        ## Debug settings
        cuda_d.input = CUDA_SOURCES
        cuda_d.output = ${OBJECTS_DIR}${QMAKE_FILE_BASE}_cuda.o
        cuda_d.commands = $$CUDA_DIR/bin/nvcc -D_DEBUG $$CUDA_DEFINES $$NVCC_OPTIONS $$CUDA_INC $$CUDA_LIBS --machine $$SYSTEM_TYPE $$CUDA_ARCH -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
        cuda_d.dependency_type = TYPE_C
        QMAKE_EXTRA_COMPILERS += cuda_d
    }
    else {
        ## Release settings
        cuda.input = CUDA_SOURCES
        cuda.output = ${OBJECTS_DIR}${QMAKE_FILE_BASE}_cuda.o
        cuda.commands = $$CUDA_DIR/bin/nvcc $$CUDA_DEFINES $$NVCC_OPTIONS $$CUDA_INC $$CUDA_LIBS --machine $$SYSTEM_TYPE $$CUDA_ARCH -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
        cuda.dependency_type = TYPE_C
        QMAKE_EXTRA_COMPILERS += cuda
    }
}
else {
    SOURCES += EquirectangularToFovVideo.cpp
}


## Input
HEADERS += FlowWidget.h \
           MainWindow.h \
           PaintGaze.h \
           ArffWidgetBase.h \
           ArffWidgetCoord.h \
           ArffWidgetSpeed.h \
           VideoWidget.h \
           MediaPlayer.h \
           VideoExtractor.h \
           EquirectangularToFovBase.h \
           EquirectangularToFovVideo.h \
           EquirectangularToFovGaze.h \
           EquirectangularToFovSpeed.h \
           EquirectangularToHead.h \
           GazeSpeed.h \
           Types.h \
           Util.h \
           Unused.h \
           ../arffHelper/Arff.h \
           ../arffHelper/ArffBase.h \
           ../arffHelper/AttributeTypes.h \
           ../arffHelper/ArffUtil.h \
           ../arffHelper/ArffOps.h
SOURCES += FlowWidget.cpp \
           GTA-VI.cpp \
           MainWindow.cpp \
           PaintGaze.cpp \
           ArffWidgetBase.cpp \
           ArffWidgetCoord.cpp \
           ArffWidgetSpeed.cpp \
           VideoWidget.cpp \
           MediaPlayer.cpp \
           VideoExtractor.cpp \
           EquirectangularToFovBase.cpp \
           EquirectangularToFovGaze.cpp \
           EquirectangularToFovSpeed.cpp \
           EquirectangularToHead.cpp \
           GazeSpeed.cpp \
           Util.cpp \
           ../arffHelper/Arff.cpp \
           ../arffHelper/ArffBase.cpp \
           ../arffHelper/AttributeTypes.cpp \
           ../arffHelper/ArffUtil.cpp \
           ../arffHelper/ArffOps.cpp
