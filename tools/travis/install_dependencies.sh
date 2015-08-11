#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e
# Print commands and their arguments as they are executed.
#set -x

# enable testing locally or on forks without multi-os enabled
if [[ "${TRAVIS_OS_NAME:-false}" == false ]]; then
    if [[ $(uname -s) == "Darwin" ]]; then
        TRAVIS_OS_NAME="osx"
    elif [[ $(uname -s) == "Linux" ]]; then
        TRAVIS_OS_NAME="linux"
    fi
fi


if [[ ${TRAVIS_OS_NAME} == "linux" ]]; then
    TEST_CC=gcc
    lsb_release -a

    # Natron requires boost >= 1.49 to compile in C++11 mode
    # see http://stackoverflow.com/questions/11302758/error-while-copy-constructing-boostshared-ptr-using-c11
    # we use the irie/boost ppa for that purpose
    sudo add-apt-repository -y ppa:irie/boost
    # the PPA xorg-edgers contains cairo 1.12 (required for rotoscoping)
    sudo add-apt-repository -y ppa:xorg-edgers/ppa
    # ubuntu-toolchain-r/test contains recent versions of gcc
    if [ "$CC" = "$TEST_CC" ]; then sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test; sudo apt-get update; sudo apt-get install gcc-4.8 g++-4.8; sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 90; sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 90; fi

    if [ "$CC" = "$TEST_CC" ]; then sudo pip install cpp-coveralls --use-mirrors; fi
    # Python 3.4
    #sudo add-apt-repository --yes ppa:fkrull/deadsnakes # python3.x
    # we get libyaml-cpp-dev from kubuntu backports (for OpenColorIO)
    if [ "$CC" = "$TEST_CC" ]; then sudo add-apt-repository -y ppa:kubuntu-ppa/backports; fi
    # we also need a recent ffmpeg for the newest version of the plugin
    #if [ "$CC" = "$TEST_CC" ]; then sudo add-apt-repository -y ppa:jon-severinsson/ffmpeg; fi #not available
    if [ "$CC" = "$TEST_CC" ]; then sudo add-apt-repository -y ppa:archivematica/externals; fi
    sudo apt-get update
    sudo apt-get update -qq


    sudo apt-get install libqt4-dev libglew-dev libboost-serialization-dev libexpat1-dev gdb libcairo2-dev python3-dev python3-pyside libpyside-dev libshiboken-dev

    echo "*** Python version:"
    python3 --version
    python3 -c "from PySide import QtGui, QtCore, QtOpenGL"
    echo "*** PySide:"
    env PKG_CONFIG_PATH=`python3-config --prefix`/lib/pkgconfig pkg-config --libs pyside
    echo "*** Shiboken:"
    pkg-config --libs shiboken
    cat /usr/lib/x86_64-linux-gnu/pkgconfig/shiboken.pc

    
    # OpenFX
    if [ "$CC" = "$TEST_CC" ]; then make -C libs/OpenFX/Examples; fi
    if [ "$CC" = "$TEST_CC" ]; then make -C libs/OpenFX/Support/Plugins; fi
    if [ "$CC" = "$TEST_CC" ]; then make -C libs/OpenFX/Support/PropTester; fi
    if [ "$CC" = "$TEST_CC" ]; then rm -rf Tests/Plugins; mkdir -p Tests/Plugins/Examples Tests/Plugins/Support Tests/Plugins/IO; fi
    if [ "$CC" = "$TEST_CC" ]; then mv libs/OpenFX/Examples/*/*-64-debug/*.ofx.bundle Tests/Plugins/Examples; fi
    if [ "$CC" = "$TEST_CC" ]; then mv libs/OpenFX/Support/Plugins/*/*-64-debug/*.ofx.bundle libs/OpenFX/Support/PropTester/*-64-debug/*.ofx.bundle Tests/Plugins/Support;  fi
    # OpenFX-IO
    if [ "$CC" = "$TEST_CC" ]; then sudo apt-get install cmake libtinyxml-dev liblcms2-dev libyaml-cpp-dev libboost-dev libavcodec-dev libavformat-dev libswscale-dev libavutil-dev libswresample-dev; wget https://github.com/imageworks/OpenColorIO/archive/v1.0.9.tar.gz -O /tmp/ocio-1.0.9.tar.gz; tar zxf /tmp/ocio-1.0.9.tar.gz; cd OpenColorIO-1.0.9; mkdir _build; cd _build; cmake .. -DCMAKE_INSTALL_PREFIX=/opt/ocio -DCMAKE_BUILD_TYPE=Release -DOCIO_BUILD_JNIGLUE=OFF -DOCIO_BUILD_NUKE=OFF -DOCIO_BUILD_SHARED=ON -DOCIO_BUILD_STATIC=OFF -DOCIO_STATIC_JNIGLUE=OFF -DOCIO_BUILD_TRUELIGHT=OFF -DUSE_EXTERNAL_LCMS=ON -DUSE_EXTERNAL_TINYXML=ON -DUSE_EXTERNAL_YAML=ON -DOCIO_BUILD_APPS=OFF -DOCIO_USE_BOOST_PTR=ON -DOCIO_BUILD_TESTS=OFF -DOCIO_BUILD_PYGLUE=OFF; make $J && sudo make install; cd ../..; fi
    if [ "$CC" = "$TEST_CC" ]; then sudo apt-get install libopenexr-dev libilmbase-dev; fi
    if [ "$CC" = "$TEST_CC" ]; then sudo apt-get install libopenjpeg-dev libtiff-dev libjpeg-dev libpng-dev libboost-filesystem-dev libboost-regex-dev libboost-thread-dev libboost-system-dev libwebp-dev libfreetype6-dev libssl-dev; wget https://github.com/OpenImageIO/oiio/archive/Release-1.5.13.tar.gz -O /tmp/OpenImageIO-1.5.13.tar.gz; tar zxf /tmp/OpenImageIO-1.5.13.tar.gz; cd oiio-Release-1.5.13; make $J USE_QT=0 USE_TBB=0 USE_PYTHON=0 USE_FIELD3D=0 USE_OPENJPEG=1 USE_OCIO=1 USE_FFMPEG=0 USE_OPENCV=0 OIIO_BUILD_TESTS=0 OIIO_BUILD_TOOLS=0 OCIO_HOME=/opt/ocio INSTALLDIR=/opt/oiio dist_dir=. cmake; sudo make $J dist_dir=.; cd ..; fi
    if [ "$CC" = "$TEST_CC" ]; then sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev libavutil-dev; fi
    if [ "$CC" = "$TEST_CC" ]; then wget https://github.com/wdas/SeExpr/archive/rel-1.0.1.tar.gz -O /tmp/SeExpr-1.0.1.tar.gz; tar zxf /tmp/SeExpr-1.0.1.tar.gz; cd SeExpr-rel-1.0.1; mkdir _build; cd _build; cmake .. -DCMAKE_INSTALL_PREFIX=/opt/seexpr; make $J && sudo make install; cd ../..; fi
    # config.pri
    # Ubuntu 12.04 precise doesn't have a pkg-config file for expat (expat.pc)
    echo 'boost: LIBS += -lboost_serialization' > config.pri
    echo 'expat: LIBS += -lexpat' >> config.pri
    echo 'expat: PKGCONFIG -= expat' >> config.pri
    # pyside and shiboken for python3 cannot be configured with pkg-config on Ubuntu 12.04LTS Precise
    echo 'pyside: PKGCONFIG -= pyside' >> config.pri
    echo 'pyside: INCLUDEPATH += $$system(pkg-config --variable=includedir pyside)' >> config.pri
    echo 'pyside: INCLUDEPATH += $$system(pkg-config --variable=includedir pyside)/QtCore' >> config.pri
    echo 'pyside: INCLUDEPATH += $$system(pkg-config --variable=includedir pyside)/QtGui' >> config.pri
    echo 'pyside: INCLUDEPATH += $$system(pkg-config --variable=includedir QtGui)' >> config.pri
    echo 'pyside: LIBS += -lpyside.cpython-32mu' >> config.pri
    # pyside doesn't have PySide::getWrapperForQObject on Ubuntu 12.04LTS Precise 
    echo 'pyside: DEFINES += PYSIDE_OLD' >> config.pri
    echo 'shiboken: PKGCONFIG -= shiboken' >> config.pri
    echo 'shiboken: INCLUDEPATH += $$system(pkg-config --variable=includedir shiboken)' >> config.pri
    echo 'shiboken: LIBS += -lshiboken.cpython-32mu' >> config.pri

    # build OpenFX-IO
    if [ "$CC" = "$TEST_CC" ]; then (cd $TRAVIS_BUILD_DIR; git clone https://github.com/MrKepzie/openfx-io.git; (cd openfx-io; git submodule update --init --recursive)) ; fi
    if [ "$CC" = "$TEST_CC" ]; then env PKG_CONFIG_PATH=/opt/ocio/lib/pkgconfig make -C openfx-io SEEXPR_HOME=/opt/seexpr OIIO_HOME=/opt/oiio; fi
    if [ "$CC" = "$TEST_CC" ]; then mv openfx-io/*/*-64-debug/*.ofx.bundle Tests/Plugins/IO;  fi

elif [[ ${TRAVIS_OS_NAME} == "osx" ]]; then
    TEST_CC=clang
    sw_vers -productVersion

    # See Travis OSX setup:
    # http://docs.travis-ci.com/user/osx-ci-environment

    # XQuartz already installed on Travis
    # install_xquartz(){
    #     echo "XQuartz start install"
    #     XQUARTZ_VERSION=2.7.6
    #     
    #     echo "XQuartz download"
    #     wget --quiet http://xquartz.macosforge.org/downloads/SL/XQuartz-${XQUARTZ_VERSION}.dmg
    #     echo "XQuartz mount dmg"
    #     hdiutil mount XQuartz-${XQUARTZ_VERSION}.dmg
    #     echo "XQuartz installer"  # sudo
    #     installer -store -pkg /Volumes/XQuartz-${XQUARTZ_VERSION}/XQuartz.pkg -target /
    #     echo "XQuartz unmount"
    #     hdiutil unmount /Volumes/XQuartz-${XQUARTZ_VERSION}
    #     echo "XQuartz end"
    # }

    # sudo install_xquartz &
    # XQ_INSTALL_PID=$!

    brew update
    brew tap homebrew/python
    brew tap homebrew/science

    echo "Install Natron dependencies"
    echo " - pip install numpy"
    pip install numpy
    # brew install numpy  # Compilation errors with gfortran
    echo " - install brew packages"
    # TuttleOFX's dependencies:
    #brew install scons swig ilmbase openexr jasper little-cms2 glew freetype fontconfig ffmpeg imagemagick libcaca aces_container ctl jpeg-turbo libraw seexpr openjpeg opencolorio openimageio
    # Natron's dependencies only
    brew install qt expat cairo glew
    # pyside/shiboken take a long time to compile, see https://github.com/travis-ci/travis-ci/issues/1961
    brew install pyside --with-python3 --without-python &
    while true; do
        #ps -p$! 2>& 1>/dev/null
        #if [ $? = 0 ]; then
        if ps -p$! 2>& 1>/dev/null; then
          echo "still going"; sleep 10
        else
            break
        fi
    done
    if [ "$CC" = "$TEST_CC" ]; then
	# dependencies for building all OpenFX plugins
	brew install ilmbase openexr freetype fontconfig ffmpeg opencolorio openimageio
    fi

    echo "Python version:"
    python3 --version
    python3 -c "from PySide import QtGui, QtCore, QtOpenGL"
    echo "PySide libs:"
    env PKG_CONFIG_PATH=`python3-config --prefix`/lib/pkgconfig pkg-config --libs pyside


    # OpenFX
    if [ "$CC" = "$TEST_CC" ]; then make -C libs/OpenFX/Examples; fi
    if [ "$CC" = "$TEST_CC" ]; then make -C libs/OpenFX/Support/Plugins; fi
    if [ "$CC" = "$TEST_CC" ]; then make -C libs/OpenFX/Support/PropTester; fi
    if [ "$CC" = "$TEST_CC" ]; then rm -rf Tests/Plugins; mkdir -p Tests/Plugins/Examples Tests/Plugins/Support Tests/Plugins/IO; fi
    if [ "$CC" = "$TEST_CC" ]; then mv libs/OpenFX/Examples/*/*-64-debug/*.ofx.bundle Tests/Plugins/Examples; fi
    if [ "$CC" = "$TEST_CC" ]; then mv libs/OpenFX/Support/Plugins/*/*-64-debug/*.ofx.bundle libs/OpenFX/Support/PropTester/*-64-debug/*.ofx.bundle Tests/Plugins/Support;  fi
    # OpenFX-IO
    if [ "$CC" = "$TEST_CC" ]; then (cd $TRAVIS_BUILD_DIR; git clone https://github.com/MrKepzie/openfx-io.git; (cd openfx-io; git submodule update --init --recursive)) ; fi
    if [ "$CC" = "$TEST_CC" ]; then make -C openfx-io OIIO_HOME=/usr/local; fi
    if [ "$CC" = "$TEST_CC" ]; then mv openfx-io/*/*-64-debug/*.ofx.bundle Tests/Plugins/IO;  fi

    # wait $XQ_INSTALL_PID || true

    echo "End dependencies installation."
    # config.pri
    echo 'boost: INCLUDEPATH += /usr/local/include' > config.pri
    echo 'boost: LIBS += -L/usr/local/lib -lboost_serialization-mt -lboost_thread-mt -lboost_system-mt' >> config.pri
    echo 'expat: PKGCONFIG -= expat' >> config.pri
    echo 'expat: INCLUDEPATH += /usr/local/opt/expat/include' >> config.pri
    echo 'expat: LIBS += -L/usr/local/opt/expat/lib -lexpat' >> config.pri
fi
#debug travis
pwd
ls
cat config.pri
echo "GCC/G++ versions:"
gcc --version
g++ --version
