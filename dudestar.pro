#-------------------------------------------------
#
# Project created by QtCreator 2019-04-26T18:18:55
#
#-------------------------------------------------

QT       += core gui serialport network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dudestar
TEMPLATE = app
VERSION_BUILD='$(shell cd $$PWD;git rev-parse --short HEAD)'
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += VERSION_NUMBER=$$VERSION_BUILD

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        CRCenc.cpp \
        DMRData.cpp \
        Golay24128.cpp \
        SHA256.cpp \
        YSFConvolution.cpp \
        YSFFICH.cpp \
        ambe.c \
        cbptc19696.cpp \
        cgolay2087.cpp \
        chamming.cpp \
        crc.cpp \
        crs129.cpp \
        dmrencoder.cpp \
        dudestar.cpp \
        fec.cpp \
        imbe_vocoder/aux_sub.cc \
        imbe_vocoder/basicop2.cc \
        imbe_vocoder/ch_decode.cc \
        imbe_vocoder/ch_encode.cc \
        imbe_vocoder/dc_rmv.cc \
        imbe_vocoder/decode.cc \
        imbe_vocoder/dsp_sub.cc \
        imbe_vocoder/encode.cc \
        imbe_vocoder/imbe_vocoder.cc \
        imbe_vocoder/math_sub.cc \
        imbe_vocoder/pe_lpf.cc \
        imbe_vocoder/pitch_est.cc \
        imbe_vocoder/pitch_ref.cc \
        imbe_vocoder/qnt_sub.cc \
        imbe_vocoder/rand_gen.cc \
        imbe_vocoder/sa_decode.cc \
        imbe_vocoder/sa_encode.cc \
        imbe_vocoder/sa_enh.cc \
        imbe_vocoder/tbls.cc \
        imbe_vocoder/uv_synt.cc \
        imbe_vocoder/v_synt.cc \
        imbe_vocoder/v_uv_det.cc \
        main.cpp \
        mbedec.cpp \
        mbeenc.cc \
        mbefec.cpp \
        p25encoder.cpp \
        pn.cpp \
        viterbi.cpp \
        viterbi5.cpp \
        ysfdec.cpp \
        ysfenc.cpp

HEADERS += \
        CRCenc.h \
        DMRData.h \
        DMRDefines.h \
        Golay24128.h \
        SHA256.h \
        YSFConvolution.h \
        YSFFICH.h \
        ambe.h \
        ambe3600x2250_const.h \
        ambe3600x2400_const.h \
        cbptc19696.h \
        cgolay2087.h \
        chamming.h \
        crc.h \
        crs129.h \
        dmrencoder.h \
        dudestar.h \
        fec.h \
        imbe_vocoder/aux_sub.h \
        imbe_vocoder/basic_op.h \
        imbe_vocoder/ch_decode.h \
        imbe_vocoder/ch_encode.h \
        imbe_vocoder/dc_rmv.h \
        imbe_vocoder/decode.h \
        imbe_vocoder/dsp_sub.h \
        imbe_vocoder/encode.h \
        imbe_vocoder/globals.h \
        imbe_vocoder/imbe.h \
        imbe_vocoder/imbe_vocoder.h \
        imbe_vocoder/math_sub.h \
        imbe_vocoder/pe_lpf.h \
        imbe_vocoder/pitch_est.h \
        imbe_vocoder/pitch_ref.h \
        imbe_vocoder/qnt_sub.h \
        imbe_vocoder/rand_gen.h \
        imbe_vocoder/sa_decode.h \
        imbe_vocoder/sa_encode.h \
        imbe_vocoder/sa_enh.h \
        imbe_vocoder/tbls.h \
        imbe_vocoder/typedef.h \
        imbe_vocoder/typedefs.h \
        imbe_vocoder/uv_synt.h \
        imbe_vocoder/v_synt.h \
        imbe_vocoder/v_uv_det.h \
        mbedec.h \
        mbeenc.h \
        mbefec.h \
        mbelib_parms.h \
        p25encoder.h \
        pn.h \
        viterbi.h \
        viterbi5.h \
        vocoder_tables.h \
        ysfdec.h \
        ysfenc.h

FORMS += \
    dudestar.ui

win32:QMAKE_LFLAGS += -static

QMAKE_LFLAGS_WINDOWS += --enable-stdcall-fixup

LIBS += -LC:\Qt\5.14.0\mingw73_32_static\lib -lmbe -lflite_cmu_us_slt -lflite_cmu_us_kal16 -lflite_cmu_us_awb -lflite_cmu_us_rms -lflite_usenglish -lflite_cmulex -lflite -lasound

RC_ICONS = images/dstar.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    dudestar.qrc

DISTFILES +=
