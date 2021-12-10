/*   2log.io
 *   Copyright (C) 2021 - 2log.io | mail@2log.io,  mail@friedemann-metzger.de
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Affero General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Affero General Public License for more details.
 *
 *   You should have received a copy of the GNU Affero General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef PAYMENTUSERACCESS_H
#define PAYMENTUSERACCESS_H


#include <QObject>
#include <QVariant>
#include <QDateTime>

class PaymentAccess : public QObject
{
    Q_OBJECT

public:

    struct Bill
    {
        struct BillItem
        {
            BillItem(QString productID) :
                productID(productID){}

            BillItem(){}

            QString productName;
            QString productID;
            QString accountingCode;
            int normalPrice;
            int paidPrice;
            int count = 1;
            int discount;
            bool flat = false;

            QVariantMap toVariant() const
            {
                QVariantMap map;
                map["productID"] = productID;
                map["count"] = count;
                map["normalPrice"] = normalPrice;
                map["paidPrice"] = paidPrice;
                map["flat"] = flat;
                map["productName"] = productName;
                map["discount"] = discount;
                map["accountingCode"] = accountingCode;
                return map;
            }
        };

        void addItem(BillItem& item)
        {
            items << item;
        }

        QString cartID;
        QString uuid;
        QList<BillItem> items;
        QString userID;
        int normalPrice;
        int paidPrice;
        QString executiveID;
        QString cardID;
        QDateTime timestamp = QDateTime::currentDateTime();

        QVariantMap toDatabase() const
        {
            QVariantMap map;
            map["userID"] = userID;
            map["executiveID"] = executiveID;
            map["timestamp"] = timestamp;
            map["normalPrice"] = normalPrice;
            map["paidPrice"] = paidPrice;
            map["cardID"] = cardID;
            map["cartID"] = cartID;

            QVariantList itemList;
            QListIterator<BillItem> it(items);
            while (it.hasNext())
            {
                BillItem item = it.next();
                itemList << item.toVariant();
            }

            map["items"] = itemList;
            return map;
        }
    };

    struct Product
    {
        QString uuid;
        QString name;
        bool selfService = false;
        QString category;
        QString imageURL;
        QString flatrateCategory;
        QString accountingCode;
        int price = -1;

        Product(QVariantMap data)
        {
            if(data.contains("_id"))
                uuid                 = data["_id"].toMap()["$oid"].toString();
            else
                uuid                 = data["uuid"].toString();

            name                = data["name"].toString();
            category            = data["category"].toString();
            flatrateCategory    = data["flatrateCategory"].toString();
            imageURL            = data["imageURL"].toString();
            accountingCode      = data["accountingCode"].toString();
            price               = data["price"].toInt();
            selfService         = data["selfService"].toBool();
        }

        bool isValid()
        {
            return !( name.isEmpty() || category.isEmpty() || price < 0 );
        }

        QVariantMap toVariant() const
        {
            QVariantMap map;
            map["uuid"]              = uuid;
            map["name"]             = name;
            map["category"]         = category;
            map["flatrateCategory"] = flatrateCategory;
            map["imageURL"]         = imageURL;
            map["price"]            = price;
            map["selfService"]      = selfService;
            map["accountingCode"]   = accountingCode;
            return map;
        }

        QVariantMap toDatabase()
        {
            QVariantMap map;
            map["name"]             = name;
            map["category"]         = category;
            map["flatrateCategory"] = flatrateCategory;
            map["imageURL"]         = imageURL;
            map["price"]            = price;
            map["selfService"]      = selfService;
            map["accountingCode"]   = accountingCode;
            return map;
        }
    };

    explicit PaymentAccess(QObject *parent = nullptr);
    static QVariantMap getUserRelatedPaymentData(QString userID, QString field);
    static bool changeUserRelatedPaymentData(QString userID, QString field, QVariantMap data);
    static QList<Product> getProducts();
    static QString insertProduct(Product &product);
    static bool updateProduct(Product& product);
    static bool deleteProduct(QString product);
    static QString insertBill(Bill &bill);
    static QVariantList getProductHistory(QDateTime from = QDateTime(), QDateTime to = QDateTime());
};

#endif // PAYMENTUSERACCESS_H
