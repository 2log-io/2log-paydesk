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


#include "PaymentAccess.h"
#include "Database/MongoDB.h"

PaymentAccess::PaymentAccess(QObject *parent) : QObject(parent)
{
}

QVariantMap PaymentAccess::getUserRelatedPaymentData(QString userID, QString field)
{
    auto projection = document{};
    projection << field.toStdString() << 1;
    document filter = document{};
    filter << "userID"<< userID.toStdString();
    QVariantList users = MongoDB::instance()->selectProjection("payment_userdata", projection.view(), filter.view());
    if(users.count() <= 0)
        return QVariantMap();

    QVariantMap result = users[0].toMap();
    return result.value(field).toMap();
}

bool PaymentAccess::changeUserRelatedPaymentData(QString userID, QString field, QVariantMap data)
{
    QVariantMap doc;
    doc["userID"] = userID;
    doc[field] = data;
    auto changes = document{};
    changes  << "$set" <<  MongoDB::queryFromVariant(doc);
    auto filter = document{};
    filter << "userID"<< userID.toStdString();
    return MongoDB::instance()->updateDocument("payment_userdata", filter.view(), changes.view(), true );
}

QList<PaymentAccess::Product> PaymentAccess::getProducts()
{
    QVariantList productResult = MongoDB::instance()->select("payment_products");
    QList<Product> products;
    QListIterator<QVariant>it(productResult);
    while(it.hasNext())
    {
        Product product = Product(it.next().toMap());
        products << product;
    }

    return products;
}

QString PaymentAccess::insertProduct(Product& product)
{
    return MongoDB::instance()->insertDocument("payment_products", product.toDatabase());
}


QString PaymentAccess::insertBill(Bill& bill)
{
    if(!bill.cartID.isEmpty())
    {
        QVariantMap query;
        query["cartID"] = bill.cartID;
        // check if there is alreay a bill with the given cartID
        if(MongoDB::instance()->select("payment_bills", MongoDB::queryFromVariant(query).view()).length() > 0)
            return "";
    }
    return MongoDB::instance()->insertDocument("payment_bills", bill.toDatabase());
}

QVariantList PaymentAccess::getProductHistory(QDateTime from, QDateTime to)
{

    QVariantMap match;

    if(from.isValid() && to.isValid())
    {
        QVariantMap time;
        if(from.isValid())
            time["$gte"] = from;
        if(to.isValid())
            time["$lte"] = to;

        match["timestamp"] = time;

        qDebug()<<match << "MATCH ";
    }

    //=========

    QString unwind = "$items";

    // ===========

    QVariantMap project;
    QVariantList conditionArgs;
    conditionArgs << "$items.flat" << 1 << 0;
    QVariantMap condMap;
    condMap["$cond"] = conditionArgs;
    project["items.isFlat"] = condMap;

    //==========

    QVariantMap group;
    group["_id"] = "$items.productID";
    QVariantList multiplyTotalSaleParams;
    multiplyTotalSaleParams << "$items.paidPrice" << "$items.count";
    QVariantMap multiplyTotalSale;
    multiplyTotalSale["$multiply"] = multiplyTotalSaleParams;
    QVariantMap totalSaleAmountSum;
    totalSaleAmountSum["$sum"] = multiplyTotalSale;
    group["totalNettoSaleAmount"] = totalSaleAmountSum;


    QVariantList multiplyBruttoSaleParams;
    multiplyBruttoSaleParams << "$items.normalPrice" << "$items.count";
    QVariantMap multiplyBruttoSale;
    multiplyBruttoSale["$multiply"] = multiplyBruttoSaleParams;
    QVariantMap bruttoSaleAmountSum;

    bruttoSaleAmountSum["$sum"] = multiplyBruttoSale;
    group["totalBruttoSaleAmount"] = bruttoSaleAmountSum;

    QVariantMap countSum;
    countSum["$sum"] = "$items.count";
    group["count"] = countSum;

    QVariantMap name;
    name["$first"] = "$items.productName";
    group["name"] = name;

    QVariantList multiplyFlatCountArgs;
    multiplyFlatCountArgs << "$items.isFlat" << "$items.count";
    QVariantMap multiplyFlatCount;
    multiplyFlatCount["$multiply"] = multiplyFlatCountArgs;
    QVariantMap sumFlats;
    sumFlats["$sum"] = multiplyFlatCount;
    group["flatCount"] = sumFlats;

    return MongoDB::instance()->selectUnwindPipeline("payment_bills", match,  unwind, project , group, QVariantMap(), QVariantMap());

}

bool PaymentAccess::updateProduct(PaymentAccess::Product &product)
{
    return MongoDB::instance()->updateDocument("payment_products", product.uuid, product.toDatabase());
}

bool PaymentAccess::deleteProduct(QString uuid)
{
    return MongoDB::instance()->removeDocument("payment_products", uuid);
}



