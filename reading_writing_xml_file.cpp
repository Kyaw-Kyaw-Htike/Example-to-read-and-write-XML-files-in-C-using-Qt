#include <QApplication>
#include "my_header1.h"
#include <QString>
#include <QDebug>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include "dlib_data_structures_simulation.h"

dlib::image_dataset_metadata::dataset load_dlib_xml_dataset(std::string fpath_xml) {

    QFile file(QString::fromStdString(fpath_xml));
    QDomDocument dom;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr,"Error","Could not open the XML file.");
        qDebug() << "Error opening file";
        exit(1);
    }
    dom.setContent(&file);
    file.close();

    QDomElement root_elem = dom.documentElement();
    QDomNodeList nameCommentImages_nodes = root_elem.childNodes();
    QDomNodeList images_nodes = nameCommentImages_nodes.at(2).toElement().childNodes();
    QString name_dataset = nameCommentImages_nodes.at(0).toElement().text();
    QString comment_dataset = nameCommentImages_nodes.at(1).toElement().text();
    int nimg = images_nodes.size();

    qDebug() << "Dataset name: " << name_dataset;
    qDebug() << "Dataset comment: " << comment_dataset;
    qDebug() << "Number of images = " << nimg;

    dlib::image_dataset_metadata::dataset D;
    D.name = name_dataset.toStdString();
    D.comment = comment_dataset.toStdString();
    D.images.resize(nimg);

    for (int i = 0; i < nimg; ++i) {
        QDomElement image_elem = images_nodes.at(i).toElement();
        QString img_path = image_elem.attribute("file");
        qDebug() << QString("------------- Image index %1 --------------").arg(i);
        qDebug() << img_path;

        QDomNodeList boxes_nodes = image_elem.childNodes();
        int nbox = boxes_nodes.size();
        qDebug() << "Number of boxes = " << nbox;

        D.images[i].filename = img_path.toStdString();
        D.images[i].boxes.resize(nbox);

        for (int j = 0; j < nbox; ++j) {
            QDomElement box_elem = boxes_nodes.at(j).toElement();
            int x = box_elem.attribute("top").toInt();
            int y = box_elem.attribute("left").toInt();
            int w = box_elem.attribute("width").toInt();
            int h = box_elem.attribute("height").toInt();
            bool ignore = box_elem.attribute("ignore", "0").toInt() == 1;
            qDebug() << QString("box (x,y,w,h) = (%1,%2,%3,%4); ignore = %5").arg(x).arg(y).arg(w).arg(h).arg(ignore);

            D.images[i].boxes[j] = dlib::image_dataset_metadata::box(dlib::rectangle(x, y, x + w - 1, y + h - 1));
            D.images[i].boxes[j].ignore = ignore;

            QDomElement boxLabel_elem = box_elem.firstChildElement();
            bool box_has_label = !boxLabel_elem.isNull();
            qDebug() << "box_has_label = " << box_has_label;
            if (box_has_label) {
                QString box_label = boxLabel_elem.text();
                qDebug() << "box label = " << box_label;
                D.images[i].boxes[j].label = box_label.toStdString();
            }
        }
    }

    return D;
}


void save_dlib_xml_dataset(const dlib::image_dataset_metadata::dataset& D, std::string fpath_xml) {
    QFile file(QString::fromStdString(fpath_xml));
    QDomDocument dom;
    QString name_dataset = QString::fromStdString(D.name);
    QString comment_dataset = QString::fromStdString(D.comment);
    size_t nimg = D.images.size();

    QDomElement dataset_elem = dom.createElement("dataset");
    QDomElement name_elem = dom.createElement("name");
    name_elem.appendChild(dom.createTextNode(name_dataset));
    dataset_elem.appendChild(name_elem);
    QDomElement comment_elem = dom.createElement("comment");
    comment_elem.appendChild(dom.createTextNode(comment_dataset));
    dataset_elem.appendChild(comment_elem);
    QDomElement images_elem = dom.createElement("images");

    qDebug() << "Dataset name: " << name_dataset;
    qDebug() << "Dataset comment: " << comment_dataset;
    qDebug() << "Number of images = " << nimg;

    for (int i = 0; i < nimg; ++i) {

        QString img_path = QString::fromStdString(D.images[i].filename);

        qDebug() << QString("------------- Image index %1 --------------").arg(i);
        qDebug() << img_path;

        QDomElement image_elem = dom.createElement("image");
        image_elem.setAttribute("file", img_path);

        int nbox = D.images[i].boxes.size();
        qDebug() << "Number of boxes = " << nbox;

        for (int j = 0; j < nbox; ++j) {

            int x = D.images[i].boxes[j].rect.left();
            int y = D.images[i].boxes[j].rect.top();
            int w = D.images[i].boxes[j].rect.width();;
            int h = D.images[i].boxes[j].rect.height();;
            bool ignore = D.images[i].boxes[j].ignore;
            qDebug() << QString("box (x,y,w,h) = (%1,%2,%3,%4); ignore = %5").arg(x).arg(y).arg(w).arg(h).arg(ignore);

            QDomElement box_elem = dom.createElement("box");
            box_elem.setAttribute("left", x);
            box_elem.setAttribute("top", y);
            box_elem.setAttribute("width", w);
            box_elem.setAttribute("height", h);
            if (ignore)
                box_elem.setAttribute("ignore", 1);

            bool box_has_label = !D.images[i].boxes[j].label.empty();
            qDebug() << "box_has_label = " << box_has_label;
            if (box_has_label) {
                QString box_label = QString::fromStdString(D.images[i].boxes[j].label);
                qDebug() << "box label = " << box_label;
                QDomElement label_elem = dom.createElement("label");
                label_elem.appendChild(dom.createTextNode(box_label));
                box_elem.appendChild(label_elem);
            }

            image_elem.appendChild(box_elem);
        }

        images_elem.appendChild(image_elem);

    }

    dataset_elem.appendChild(images_elem);
    dom.appendChild(dataset_elem);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text )) {
        QMessageBox::warning(nullptr,"Error","Could not open the XML file.");
        qDebug() << "Error opening file";
        exit(1);
    }
    QTextStream stream( &file );
    stream << dom.toString();
    file.close();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    dlib::image_dataset_metadata::dataset D = load_dlib_xml_dataset("1.xml");	
    save_dlib_xml_dataset(D, "2.xml");




    return a.exec();
}
