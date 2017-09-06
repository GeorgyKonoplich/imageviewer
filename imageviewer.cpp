#include <QtWidgets>
#ifndef QT_NO_PRINTER
#include <QPrintDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <string>
#include <QFile>
#endif

#include "imageviewer.h"

enum { absoluteFileNameRole = Qt::UserRole + 1 };

static inline QString fileNameOfItem(const QTableWidgetItem *item)
{
    return item->data(absoluteFileNameRole).toString();
}

static inline void openFile(const QString &fileName)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

ImageViewer::ImageViewer()
{
    setAttribute(Qt::WA_AcceptTouchEvents);
    imageLabel = new ClickableLabel;
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    imageLabel->setScaledContents(true);

    lineEdit = new QLineEdit();
    lineEdit->setText(QString("car"));

    QPushButton *browseButton = new QPushButton(tr("&Browse..."), this);
    connect(browseButton, &QAbstractButton::clicked, this, &ImageViewer::browse);
    findButton = new QPushButton(tr("&Find"), this);
    connect(findButton, &QAbstractButton::clicked, this, &ImageViewer::find);

    fileComboBox = createComboBox(tr("*"));
    connect(fileComboBox->lineEdit(), &QLineEdit::returnPressed,
            this, &ImageViewer::animateFindClick);

    directoryComboBox = createComboBox(QDir::toNativeSeparators(QDir::currentPath()));
    connect(directoryComboBox->lineEdit(), &QLineEdit::returnPressed,
            this, &ImageViewer::animateFindClick);

    filesFoundLabel = new QLabel;

    createFilesTable();

    QGridLayout *mainLayout = new QGridLayout(this);
    QLabel *named = new QLabel(tr("Named:"));
    named->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    named->setMinimumWidth(100);
    mainLayout->addWidget(named, 1, 0);
    mainLayout->addWidget(fileComboBox, 1, 1, 1, 1);
    lineEdit->setMaximumWidth(100);
    mainLayout->addWidget(lineEdit, 1, 3);
    mainLayout->addWidget(new QLabel(tr("In directory:")), 0, 0);
    mainLayout->addWidget(directoryComboBox, 0, 1, 1, 2);
    mainLayout->addWidget(browseButton, 0, 3);

    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);

    mainLayout->addWidget(filesTable, 2, 0);
    mainLayout->addWidget(scrollArea, 2, 1, 1, 3);
    mainLayout->addWidget(filesFoundLabel, 3, 0, 1, 2);
    mainLayout->addWidget(findButton, 1, 2);

    window = new QWidget();
    window->setLayout(mainLayout);
    setCentralWidget(window);

    createActions();
    createMenus();

    resize(QGuiApplication::primaryScreen()->availableSize() * 4 / 5);
    connect(imageLabel, SIGNAL(clicked()), this, SLOT(onclicked()));
}


void ImageViewer::browse()
{
    QString directory =
        QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Find Files"), QDir::currentPath()));

    if (!directory.isEmpty()) {
        if (directoryComboBox->findText(directory) == -1)
            directoryComboBox->addItem(directory);
        directoryComboBox->setCurrentIndex(directoryComboBox->findText(directory));
    }
}

static void updateComboBox(QComboBox *comboBox)
{
    if (comboBox->findText(comboBox->currentText()) == -1)
        comboBox->addItem(comboBox->currentText());
}

static void findRecursion(const QString &path, const QString &pattern, QStringList *result)
{
    QDir currentDir(path);
    const QString prefix = path + QLatin1Char('/');
    foreach (const QString &match, currentDir.entryList(QStringList(pattern), QDir::Files | QDir::NoSymLinks))
        result->append(prefix + match);
    foreach (const QString &dir, currentDir.entryList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot))
        findRecursion(prefix + dir, pattern, result);
}

void ImageViewer::find()
{
    filesTable->setRowCount(0);

    QString fileName = fileComboBox->currentText();

    path = QDir::cleanPath(directoryComboBox->currentText());

    updateComboBox(fileComboBox);

    updateComboBox(directoryComboBox);


    currentDir = QDir(path);
    QStringList files;
    findRecursion(path, fileName.isEmpty() ? QStringLiteral("*") : fileName, &files);

    showFiles(files);
}

void ImageViewer::animateFindClick()
{
    findButton->animateClick();
}

void ImageViewer::showFiles(const QStringList &files)
{
    for (int i = 0; i < files.size(); ++i) {
        const QString &fileName = files.at(i);
        const QString toolTip = QDir::toNativeSeparators(fileName);
        const QString relativePath = QDir::toNativeSeparators(currentDir.relativeFilePath(fileName));
        const qint64 size = QFileInfo(fileName).size();
        QTableWidgetItem *fileNameItem = new QTableWidgetItem(relativePath);
        fileNameItem->setData(absoluteFileNameRole, QVariant(fileName));
        fileNameItem->setToolTip(toolTip);
        fileNameItem->setFlags(fileNameItem->flags() ^ Qt::ItemIsEditable);
        QTableWidgetItem *sizeItem = new QTableWidgetItem(tr("%1 KB")
                                             .arg(int((size + 1023) / 1024)));
        sizeItem->setData(absoluteFileNameRole, QVariant(fileName));
        sizeItem->setToolTip(toolTip);
        sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        sizeItem->setFlags(sizeItem->flags() ^ Qt::ItemIsEditable);

        int row = filesTable->rowCount();
        filesTable->insertRow(row);
        filesTable->setItem(row, 0, fileNameItem);
        filesTable->setItem(row, 1, sizeItem);
    }
    filesFoundLabel->setText(tr("%n file(s) found (Double click on a file to open it)", 0, files.size()));
    filesFoundLabel->setWordWrap(true);
}

QComboBox *ImageViewer::createComboBox(const QString &text)
{
    QComboBox *comboBox = new QComboBox;
    comboBox->setEditable(true);
    comboBox->addItem(text);
    comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return comboBox;
}

void ImageViewer::createFilesTable()
{
    filesTable = new QTableWidget(0, 2);
    filesTable->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    filesTable->setMinimumWidth(100);

    filesTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList labels;
    labels << tr("Filename") << tr("Size");
    filesTable->setHorizontalHeaderLabels(labels);
    filesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    filesTable->verticalHeader()->hide();
    filesTable->setShowGrid(false);
    filesTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(filesTable, &QTableWidget::customContextMenuRequested,
            this, &ImageViewer::contextMenu);
    connect(filesTable, &QTableWidget::cellActivated,
            this, &ImageViewer::openFileOfItem);
    connect(filesTable, &QTableWidget::cellClicked,
            this, &ImageViewer::loadFileOfItem);
}


void ImageViewer::openFileOfItem(int row, int /* column */)
{
    const QTableWidgetItem *item = filesTable->item(row, 0);
    openFile(fileNameOfItem(item));
}

void ImageViewer::saveExit(){
    writeObjects(image_name);
}

void ImageViewer::loadFileOfItem(int row, int /* column */)
{
    const QTableWidgetItem *item = filesTable->item(row, 0);
    size_t sz = fileNameOfItem(item).split('/').size();
    qDebug() << fileNameOfItem(item).split('/')[sz-1];
    qDebug() << path;
    if (image_name != "") writeObjects(image_name);
    bool res = loadFile(fileNameOfItem(item));
    image_name = fileNameOfItem(item).split('/')[sz-1];
    drawObjects(image_name);
}

void ImageViewer::drawObjects(QString &fileName){
    QString name = "";
    for (int i = 0; i < fileName.split('.').size()-1; i ++){
        name = name + fileName.split('.')[i] + ".";
    }
    name = path + "/labels/" + name + "txt";
    qDebug() << "drawingObjects: " << name;
    QFile inputFile(name);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            QStringList arr = line.split(' ');
            xx = arr[1].toInt();
            yy = arr[2].toInt();
            xx1 = arr[3].toInt();
            yy1 = arr[4].toInt();
            xx2 = arr[5].toInt();
            yy2 = arr[6].toInt();
            xx3 = arr[7].toInt();
            yy3 = arr[8].toInt();
            vector<pair<int, int>> coord;
            coord.push_back(make_pair(xx, yy));
            coord.push_back(make_pair(xx1, yy1));
            coord.push_back(make_pair(xx2, yy2));
            coord.push_back(make_pair(xx3, yy3));
            rects.push_back(coord);
            objects.push_back(arr[0]);
        }
        drawingRects(rects);
        inputFile.close();
    }

}

void ImageViewer::illumination(){
    QImage tmp(imageLabel->pixmap()->toImage());
    QPainter painter(&tmp);
    for (int i = rects.size()-1; i < rects.size(); i++){
        QPen paintpen(Qt::blue);
        paintpen.setWidth(2);
        painter.setPen(paintpen);
        QLineF line((float)rects[i][0].first, (float)rects[i][0].second, (float)rects[i][1].first, (float)rects[i][1].second);
        painter.drawLine(line);
        QPen paintpen1(Qt::green);
        paintpen1.setWidth(2);
        painter.setPen(paintpen1);
        for (int j = 2; j < rects[i].size(); j++){
            QLineF line((float)rects[i][j-1].first, (float)rects[i][j-1].second, (float)rects[i][j].first, (float)rects[i][j].second);
            painter.drawLine(line);
        }
        QLineF line1((float)rects[i][0].first, (float)rects[i][0].second, (float)rects[i][3].first, (float)rects[i][3].second);
        painter.drawLine(line1);
    }
    imageLabel->setPixmap(QPixmap::fromImage(tmp));
}

void ImageViewer::delight(){
    QImage tmp(imageLabel->pixmap()->toImage());
    QPainter painter(&tmp);

    for (int i = rects.size()-1; i < rects.size(); i++){
        QPen paintpen(Qt::blue);
        paintpen.setWidth(2);
        painter.setPen(paintpen);
        QLineF line((float)rects[i][0].first, (float)rects[i][0].second, (float)rects[i][1].first, (float)rects[i][1].second);
        painter.drawLine(line);
        QPen paintpen1(Qt::red);
        paintpen1.setWidth(2);
        painter.setPen(paintpen1);
        for (int j = 2; j < rects[i].size(); j++){
            QLineF line((float)rects[i][j-1].first, (float)rects[i][j-1].second, (float)rects[i][j].first, (float)rects[i][j].second);
            painter.drawLine(line);
        }
        QLineF line1((float)rects[i][0].first, (float)rects[i][0].second, (float)rects[i][3].first, (float)rects[i][3].second);
        painter.drawLine(line1);
    }
    imageLabel->setPixmap(QPixmap::fromImage(tmp));
}


void ImageViewer::drawingRects(vector<vector<pair<int, int> > > &rects){
    QImage tmp(imageLabel->pixmap()->toImage());
    QPainter painter(&tmp);
    for (int i = 0; i < rects.size(); i++){
        images.push_back(tmp);
        QPen paintpen(Qt::blue);
        paintpen.setWidth(2);
        painter.setPen(paintpen);
        QLineF line((float)rects[i][0].first, (float)rects[i][0].second, (float)rects[i][1].first, (float)rects[i][1].second);
        painter.drawLine(line);
        QPen paintpen1(Qt::red);
        paintpen1.setWidth(2);
        painter.setPen(paintpen1);
        for (int j = 2; j < rects[i].size(); j++){
            QLineF line((float)rects[i][j-1].first, (float)rects[i][j-1].second, (float)rects[i][j].first, (float)rects[i][j].second);
            painter.drawLine(line);
        }
        QLineF line1((float)rects[i][0].first, (float)rects[i][0].second, (float)rects[i][3].first, (float)rects[i][3].second);
        painter.drawLine(line1);
    }
    imageLabel->setPixmap(QPixmap::fromImage(tmp));
    illumination();
}

void ImageViewer::writeObjects(QString &fileName)
{
    QString name = "";
    for (int i = 0; i < fileName.split('.').size()-1; i ++){
        name = name + fileName.split('.')[i] + ".";
    }
    if (!QDir(path + "/labels").exists()){
        QDir().mkdir(path + "/labels");
    }
    if (file->isOpen()) file->close();
    if (QFile(path + "/labels/" + name + "txt").exists()){
        QFile(path + "/labels/" + name + "txt").remove();
    }
    file = new QFile(path + "/labels/" + name + "txt");
    file->open(QIODevice::ReadWrite);
    for (int i = 0; i < rects.size(); i++){
        QString text = objects[i];
        for (int j = 0; j < rects[i].size(); j++){
            text = text + " " + QString::number(rects[i][j].first) + " " + QString::number(rects[i][j].second);
        }
        text = text + "\n";
        QByteArray ba = text.toLatin1();
        const char *msg = ba.data();
        qDebug() << msg;
        int x = file->write(msg, text.length());
        qDebug() << x;
    }
    rects.clear();
    objects.clear();
    images.clear();
    if (file->isOpen()) file->close();
}

void ImageViewer::contextMenu(const QPoint &pos)
{
    const QTableWidgetItem *item = filesTable->itemAt(pos);
    if (!item)
        return;
    QMenu menu;
#ifndef QT_NO_CLIPBOARD
    QAction *copyAction = menu.addAction("Copy Name");
#endif
    QAction *openAction = menu.addAction("Open");
    QAction *action = menu.exec(filesTable->mapToGlobal(pos));
    if (!action)
        return;
    const QString fileName = fileNameOfItem(item);
    if (action == openAction)
        openFile(fileName);
#ifndef QT_NO_CLIPBOARD
    else if (action == copyAction)
        QGuiApplication::clipboard()->setText(QDir::toNativeSeparators(fileName));
#endif
}
void ImageViewer::onclicked(){
    qDebug() << imageLabel->ev->x();
    qDebug() << imageLabel->ev->y();

    if (global_counter == 0){
        xx = imageLabel->ev->x() / scaleFactor;
        yy = imageLabel->ev->y() / scaleFactor;
        global_counter++;
    } else if (global_counter == 1){
        xx1 = imageLabel->ev->x() / scaleFactor;
        yy1 = imageLabel->ev->y() / scaleFactor;
        delight();
        QImage tmp(imageLabel->pixmap()->toImage());
        images.push_back(tmp);
        QPainter painter(&tmp);
        QPen paintpen(Qt::blue);
        paintpen.setWidth(2);
        painter.setPen(paintpen);
        QLineF line((float)xx, (float)yy, (float)xx1, (float)yy1);
        painter.drawLine(line);
        imageLabel->setPixmap(QPixmap::fromImage(tmp));
        global_counter++;
    }else{
        xx2 = imageLabel->ev->x() / scaleFactor;
        yy2 = imageLabel->ev->y() / scaleFactor;

        int a1 = yy - yy1;
        int b1 = xx1 - xx;

        int aa1 = -b1;
        int bb1 = a1;
        int cc1 = -aa1 * xx1 - bb1 * yy1;
        int aa2 = -bb1;
        int bb2 = aa1;
        int cc2 = -aa2 * xx2 - bb2 * yy2;
        yy2 = -(aa1 * cc2 - aa2 * cc1)/(aa1 * bb2 - aa2 * bb1);
        xx2 = -(cc1 * bb2 - cc2 * bb1)/(aa1 * bb2 - aa2 * bb1);

        aa1 = -b1;
        bb1 = a1;
        cc1 = -aa1 * xx - bb1 * yy;

        int a2 = yy2 - yy1;
        int b2 = xx1 - xx2;

        aa2 = -b2;
        bb2 = a2;
        cc2 = -aa2 * xx2 - bb2 * yy2;

        yy3 = -(aa1 * cc2 - aa2 * cc1)/(aa1 * bb2 - aa2 * bb1);
        xx3 = -(cc1 * bb2 - cc2 * bb1)/(aa1 * bb2 - aa2 * bb1);


        vector<pair<int, int>> coord;
        coord.push_back(make_pair(xx, yy));
        coord.push_back(make_pair(xx1, yy1));
        coord.push_back(make_pair(xx2, yy2));
        coord.push_back(make_pair(xx3, yy3));
        rects.push_back(coord);

        QImage tmp(imageLabel->pixmap()->toImage());

        QPainter painter(&tmp);
        QPen paintpen(Qt::red);
        paintpen.setWidth(2);
        painter.setPen(paintpen);

        QLineF line((float)xx1, (float)yy1, (float)xx2, (float)yy2);
        painter.drawLine(line);
        QLineF line1((float)xx, (float)yy, (float)xx3, (float)yy3);
        painter.drawLine(line1);
        QLineF line2((float)xx2, (float)yy2, (float)xx3, (float)yy3);
        painter.drawLine(line2);

        imageLabel->setPixmap(QPixmap::fromImage(tmp));
        global_counter = 0;
        QString text = lineEdit->text();
        objects.push_back(text);
        //prev = tmp;
        illumination();
    }
}

bool ImageViewer::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    QImage image = reader.read();
    if (image.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1.").arg(QDir::toNativeSeparators(fileName)));
        setWindowFilePath(QString());
        imageLabel->setPixmap(QPixmap());
        imageLabel->adjustSize();
        return false;
    }
    QPixmap pix = QPixmap::fromImage(image);
    imageLabel->setPixmap(pix);
    original = QPixmap::fromImage(image);

    scaleFactor = 1.0;
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    updateActions();

    if (!fitToWindowAct->isChecked())
        imageLabel->adjustSize();

    setWindowFilePath(fileName);
    return true;
}

void ImageViewer::open()
{
    QStringList mimeTypeFilters;
    foreach (const QByteArray &mimeTypeName, QImageReader::supportedMimeTypes())
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    QFileDialog dialog(this, tr("Open File"),
                       picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

void ImageViewer::close(){
    file->close();
}

void ImageViewer::print()
{
    Q_ASSERT(imageLabel->pixmap());
#if !defined(QT_NO_PRINTER) && !defined(QT_NO_PRINTDIALOG)
    QPrintDialog dialog(&printer, this);
    if (dialog.exec()) {
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = imageLabel->pixmap()->size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(imageLabel->pixmap()->rect());
        painter.drawPixmap(0, 0, *imageLabel->pixmap());
    }
#endif
}

void ImageViewer::zoomIn()
{
    scaleImage(1.25);
}

void ImageViewer::zoomOut()
{
    scaleImage(0.8);
}

void ImageViewer::normalSize()
{
    imageLabel->adjustSize();
    scaleFactor = 1.0;
}

void ImageViewer::deleteRect()
{
    qDebug()<< "deleteRect" <<  global_counter;
    if (!rects.empty()){
        imageLabel->setPixmap(QPixmap::fromImage(images.back()));
        if (global_counter == 0){
            rects.pop_back();
            objects.pop_back();
            images.pop_back();
        }else{
            global_counter = 0;
            images.pop_back();
        }
        illumination();
        if (!objects.empty()) lineEdit->setText(objects.back());

    }
}

void ImageViewer::rotateRect()
{
    if (!rects.empty() && global_counter == 0){
        qDebug()<< "rotateRect";
        QImage tmp = images.back();
        QPainter painter(&tmp);
        QPen paintpen(Qt::blue);
        paintpen.setWidth(2);
        painter.setPen(paintpen);
        vector<pair<int, int>> coord = rects.back();
        rects.pop_back();
        rotate(coord.begin(), coord.begin() + 1, coord.end());
        QLineF line((float)coord[0].first, (float)coord[0].second, (float)coord[1].first, (float)coord[1].second);
        painter.drawLine(line);
        QPen paintpen1(Qt::green);
        paintpen1.setWidth(2);
        painter.setPen(paintpen1);
        for (int j = 2; j < coord.size(); j++){
            QLineF line((float)coord[j-1].first, (float)coord[j-1].second, (float)coord[j].first, (float)coord[j].second);
            painter.drawLine(line);
        }
        QLineF line1((float)coord[0].first, (float)coord[0].second, (float)coord[3].first, (float)coord[3].second);
        painter.drawLine(line1);
        rects.push_back(coord);
        imageLabel->setPixmap(QPixmap::fromImage(tmp));
    }
}


void ImageViewer::fitToWindow()
{
    bool fitToWindow = fitToWindowAct->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow) {
        normalSize();
    }
    updateActions();
}

void ImageViewer::about()
{
    QMessageBox::about(this, tr("About Image Viewer"),
            tr("hi"));
}

void ImageViewer::createActions()
{

    deleteAct = new QAction(tr("&Delete..."), this);
    deleteAct->setShortcut(tr("Delete"));
    connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteRect()));
    window->addAction(deleteAct);

    rotateAct = new QAction(tr("&Rotate..."), this);
    rotateAct->setShortcut(tr("R"));
    connect(rotateAct, SIGNAL(triggered()), this, SLOT(rotateRect()));
    window->addAction(rotateAct);

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    printAct = new QAction(tr("&Print..."), this);
    printAct->setShortcut(tr("Ctrl+P"));
    printAct->setEnabled(false);
    connect(printAct, SIGNAL(triggered()), this, SLOT(print()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    zoomInAct = new QAction(tr("Zoom &In (25%)"), this);
    zoomInAct->setShortcut(tr("Ctrl+="));
    zoomInAct->setEnabled(false);
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(tr("Zoom &Out (25%)"), this);
    zoomOutAct->setShortcut(tr("Ctrl+-"));
    zoomOutAct->setEnabled(false);
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    normalSizeAct = new QAction(tr("&Normal Size"), this);
    normalSizeAct->setShortcut(tr("Ctrl+S"));
    normalSizeAct->setEnabled(false);
    connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(normalSize()));

    fitToWindowAct = new QAction(tr("&Fit to Window"), this);
    fitToWindowAct->setEnabled(false);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));
    connect(fitToWindowAct, SIGNAL(triggered()), this, SLOT(fitToWindow()));

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void ImageViewer::createMenus()
{
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    viewMenu = new QMenu(tr("&View"), this);
    window->addAction(zoomInAct);
    window->addAction(zoomOutAct);
    window->addAction(normalSizeAct);
    viewMenu->addSeparator();
    viewMenu->addAction(fitToWindowAct);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(viewMenu);
    menuBar()->addMenu(helpMenu);
}

void ImageViewer::updateActions()
{
    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}

void ImageViewer::scaleImage(double factor)
{
    Q_ASSERT(imageLabel->pixmap());
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(scaleFactor < 3.0);
    zoomOutAct->setEnabled(scaleFactor > 0.133);
}

void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}
