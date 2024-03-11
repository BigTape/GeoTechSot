#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QTableView>
#include <QStandardItemModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHeaderView>
#include <QBrush>

class MyWidget : public QWidget {
public:
    MyWidget() : model(this), tableView(this) {
        setWindowTitle("JSON Placeholder Data");

        model.setColumnCount(2);
        model.setHorizontalHeaderLabels(QStringList() << "Name" << "Latitude");

        tableView.setModel(&model);

        QHBoxLayout *layout = new QHBoxLayout;
        layout->addWidget(&tableView);
        setLayout(layout);

        tableView.horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        connectToServer();
    }

private:
    QTableView tableView;
    QStandardItemModel model;
    QNetworkAccessManager networkManager;

    void connectToServer() {
        QNetworkRequest request;
        request.setUrl(QUrl("http://jsonplaceholder.typicode.com:80/users"));

        QNetworkReply* reply = networkManager.get(request);

        connect(reply, &QNetworkReply::finished, this, &MyWidget::readData);
        connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::errorOccurred),
                [=](QNetworkReply::NetworkError error){
                    qDebug() << "Network error:" << reply->errorString();
                });
    }

    void readData() {
        QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
        if (!reply)
            return;

        QByteArray responseData = reply->readAll();

        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
        if (!jsonResponse.isNull()) {
            QJsonArray jsonArray = jsonResponse.array();
            foreach (const QJsonValue &value, jsonArray) {
                QJsonObject obj = value.toObject();
                if (obj.contains("name") && obj["address"].toObject().contains("geo")) {
                    QString name = obj["name"].toString();
                    QString latitude = obj["address"].toObject()["geo"].toObject()["lat"].toString();
                    double latValue = latitude.toDouble();
                    addDataToModel(name, latitude, latValue);
                }
            }
        }

        reply->deleteLater();
    }

    void addDataToModel(const QString &name, const QString &latitude, double latValue) {
        QList<QStandardItem*> rowItems;
        rowItems.append(new QStandardItem(name));

        QStandardItem *item = new QStandardItem();
        item->setData(QVariant::fromValue(latValue), Qt::UserRole);
        item->setData(latitude, Qt::DisplayRole);

        QColor color = (latValue < 0) ? Qt::red : Qt::green;
        item->setData(color, Qt::BackgroundRole);

        rowItems.append(item);

        model.appendRow(rowItems);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MyWidget widget;
    widget.show();
    return app.exec();
}
