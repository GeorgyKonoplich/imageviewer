#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QDir>
#include <QMainWindow>
#ifndef QT_NO_PRINTER
#include <QPrinter>
#include <QTouchEvent>
#include "clickablelabel.h"
#include <map>
#include <vector>
using namespace std;

#endif

QT_BEGIN_NAMESPACE
class QComboBox;
class QTableWidget;
class QTableWidgetItem;
class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
class QPushButton;
class QLineEdit;
class QPainter;
QT_END_NAMESPACE

class ImageViewer : public QMainWindow
{
    Q_OBJECT

public:
    ImageViewer();
    bool loadFile(const QString &);

protected:
    //void mousePressEvent(QMouseEvent * event);
private slots:
    void open();
    void print();
    void close();
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow();
    void about();
    void onclicked();
    void deleteRect();
    void rotateRect();
    void browse();
    void find();
    void animateFindClick();
    void openFileOfItem(int row, int column);
    void loadFileOfItem(int row, int column);
    void contextMenu(const QPoint &pos);
    void saveExit();

private:
    QStringList findFiles(const QStringList &files, const QString &text);
    void showFiles(const QStringList &files);
    QComboBox *createComboBox(const QString &text = QString());
    void createFilesTable();
    void writeObjects(QString &fileName);
    void drawObjects(QString &fileName);
    void drawingRects(vector< vector< pair<int, int> > > &rects);

    QComboBox *fileComboBox;
    QComboBox *textComboBox;
    QComboBox *directoryComboBox;
    QLabel *filesFoundLabel;
    QPushButton *findButton;
    QTableWidget *filesTable;

    QDir currentDir;
    void createActions();
    void createMenus();
    void updateActions();
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void illumination();
    void delight();

    int global_counter = 0;
    int kol_photo = 55;
    ClickableLabel *imageLabel;
    QScrollArea *scrollArea;
    QPixmap original;
    int xx;
    int yy;
    int xx1;
    int yy1;
    int xx2;
    int yy2;
    int xx3;
    int yy3;
    QImage prev;
    vector< vector< pair<int, int> > > rects;
    vector<QImage> images;
    vector<QString> objects;
    QString path;
    QString image_name;

    double scaleFactor;
    QFile *file = new QFile("hah");

#ifndef QT_NO_PRINTER
    QPrinter printer;
#endif
    QPainter *paint;
    QLineEdit *lineEdit;
    QLineEdit *lineEdit1;
    QLineEdit *lineEdit2;
    QLineEdit *lineEdit3;
    QLineEdit *lineEdit4;
    QLineEdit *lineEdit5;
    QPushButton *button;
    QPushButton *button1;
    QAction *openAct;
    QAction *deleteAct;
    QAction *rotateAct;
    QAction *printAct;
    QAction *exitAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;
    QAction *aboutAct;
    QAction *aboutQtAct;

    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;

    QWidget *window;
};

#endif
