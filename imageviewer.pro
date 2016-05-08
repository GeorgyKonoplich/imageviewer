QT += widgets gui core
qtHaveModule(printsupport): QT += printsupport

HEADERS       = imageviewer.h \
                clickablelabel.h
SOURCES       = imageviewer.cpp \
                main.cpp \
                clickablelabel.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/widgets/imageviewer
INSTALLS += target


wince {
   DEPLOYMENT_PLUGIN += qjpeg qgif
}
