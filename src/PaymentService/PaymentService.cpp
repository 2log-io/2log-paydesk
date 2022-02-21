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


#include "PaymentService.h"
#include "Database/LogAccess.h"
#include "Server/Authentication/AuthentificationService.h"
#include "Server/Resources/ResourceManager/ResourceManager.h"
#include "Server/Resources/ObjectResource/ObjectResource.h"
#include "Database/UserAccess.h"
#include "Database/LogAccess.h"
#include "PaymentUserAccess/PaymentAccess.h"
#include <QUuid>
#include "MailClient/MailManager.h"

PaymentService::PaymentService(QObject *parent) : IService(parent)
{
}

QString PaymentService::getServiceName()
{
      return "payment";
}

QStringList PaymentService::getServiceCalls()
{
    QStringList calls;
    calls << "bill" << "preparebill" << "getsales";
    return calls;
}

bool PaymentService::call(QString call, QString token, QString cbID, QVariant argument)
{
    iIdentityPtr executiveUser = AuthenticationService::instance()->validateToken(token);
    if(executiveUser.isNull())
        return false;

    QVariantMap argMap = argument.toMap();

    if(call == "getsales")
    {
        QDateTime from = argMap["from"].toDateTime();
        QDateTime to = argMap["to"].toDateTime();

        QVariantList data = PaymentAccess::getProductHistory(from, to);
        qDebug()<<data;
        Q_EMIT response(cbID, data);
        return true;
    }

    if(call == "preparebill")
    {
        QVariantMap answer;

        if((!argMap.contains("cardID") && !argMap.contains("userID")) || !argMap.contains("bill") || !argMap.contains("total"))
        {
            answer["errcode"] = -3;
            answer["errstring"] = "Invalid parameters";
            Q_EMIT response(cbID, answer);
            return true;
        }

        if(!checkCobot(token))
        {
            qInfo()<<"No Cobot Service found ):";
            answer = prepareBill(argMap, {});
            answer["cobotSuccess"] = false;
            answer["cobotConnected"] = false;
        }
        else
        {
            qInfo()<<"Cobot Service found!";

            QString cardID = argMap["cardID"].toString();
            QString userID = argMap["userID"].toString();
            fablabUserPtr user;
            if(userID.isEmpty())
                user = UserAccess::instance()->getUserWithCard(cardID);
            else
                user = UserAccess::instance()->getUser(userID);
            QVariantMap answer;

            if(user.isNull())
            {
                answer["errcode"] = -2;
                answer["errstring"] = "Unknown user";
                Q_EMIT response(cbID, answer);
                return true;
            }

            Request req;
            QString cobotUid = QUuid::createUuid().toString();

            req.ownCbID = cbID;
            req.cobotCbID = cobotUid;
            req.data = argMap;

            QVariantMap arg;
            arg["userID"] = user->getUuid();

            _cobotCbMap.insert(cobotUid, req);
            if(_cobotService->call("getMemberPlanExtras", token,cobotUid, arg))
            {
                return true;
            }
            else
            {
                _cobotCbMap.remove(cobotUid);
                answer["errcode"] = -1;
                answer["errstring"] = "Cobot service call not available";
                Q_EMIT response(cbID, answer);
                return true;
            }
        }

        Q_EMIT response(cbID, answer);
        return true;
    }

    if(call == "bill")
    {

        if( !(executiveUser->isAuthorizedTo(IS_ADMIN) ||
             executiveUser->isAuthorizedTo(LAB_ADMIN) ||
             executiveUser->isAuthorizedTo(LAB_SEND_BILLS) ))
                return false;

        QVariantMap answer;

        if((!argMap.contains("cardID") && !argMap.contains("userID")) || !argMap.contains("bills"))
        {
            answer["errcode"] = -3;
            answer["errstring"] = "Invalid parameters";
            Q_EMIT response(cbID, answer);
            return true;
        }

        QString cardID = argMap["cardID"].toString();
        QString userID = argMap["userID"].toString();
        QString cartID = argMap.value("cartID", "").toString();

        fablabUserPtr user;
        if(userID.isEmpty())
            user = UserAccess::instance()->getUserWithCard(cardID);
        else
            user = UserAccess::instance()->getUser(userID);

        if(user.isNull())
        {
            answer["errcode"] = -2;
            answer["errstring"] = "Unknown user";
            Q_EMIT response(cbID, answer);
            return true;
        }

        answer["userID"] = user->identityID();
        answer["eMail"] = user->getEMail();

        bool ok = true;
        int total = argMap["discountTotal"].toInt(&ok);
        if(!ok)
        {
            answer["errcode"] = -3;
            answer["errstring"] = "Invalid parameters";
            Q_EMIT response(cbID, answer);
            return true;
        }

        answer["errcode"] = 0;
        answer["errstring"] = "";


        QListIterator<QVariant> billIt(argMap["bills"].toList());
        QList<Log> logs;
        PaymentAccess::Bill totalBill;
        totalBill.cartID = cartID;

        while (billIt.hasNext())
        {
            QVariantMap bill = billIt.next().toMap();

            QString accountingCode = bill["accountingCode"].toString();
            int billTotal = bill["totalNetto"].toInt();

            // create a temporary bill which contains only items that
            // belongs to the given accounting code
            PaymentAccess::Bill partOfBill;
            partOfBill.executiveID = executiveUser->identityID();
            partOfBill.userID = argMap["userID"].toString();
            partOfBill.normalPrice = argMap["total"].toInt();
            partOfBill.paidPrice = argMap["discountTotal"].toInt();
            partOfBill.cardID = argMap["cardID"].toString();

            // prepare the log for the internal 2log accounting / cobot payment
            Log log;
            log.event = Log::BILL;
            log.price = billTotal;
            log.userID = user->identityID();
            log.resourceID = accountingCode;
            log.userName = user->getName()+" "+user->getSurname();
            log.description = accountingCode;
            log.timestamp = QDateTime::currentDateTime();
            log.executive = executiveUser->identityID();
            log.externalType = "payment";

            QMap<QString, PaymentAccess::Bill::BillItem> map;
            QListIterator<QVariant> it(bill["items"].toList());

            // iterate over all items and count / collect the products
            while(it.hasNext())
            {
                QVariantMap itemMap = it.next().toMap();
                QString uuid = itemMap["uuid"].toString();
                if(map.contains(uuid) && !uuid.isEmpty())
                {
                    auto item =  map.value(uuid);
                    map[uuid].count ++;
                }
                else
                {
                    PaymentAccess::Bill::BillItem item(uuid);
                    item.flat = itemMap["flat"].toBool();
                    item.normalPrice = itemMap["price"].toInt();
                    item.paidPrice = itemMap["newprice"].toInt();
                    item.accountingCode = accountingCode;
                    item.productName = itemMap["name"].toString();
                    map[uuid] = item;
                }
            }

            QMapIterator<QString, PaymentAccess::Bill::BillItem> internalMapIt(map);
            while(internalMapIt.hasNext())
            {
                auto internalBillItem = internalMapIt.next().value();
                partOfBill.addItem(internalBillItem);
                totalBill.addItem(internalBillItem);
            };

            log.internalAttachment = partOfBill.toDatabase();
            logs << log;
        }

        totalBill.executiveID = executiveUser->identityID();
        totalBill.userID = argMap["userID"].toString();
        totalBill.normalPrice = argMap["total"].toInt();
        totalBill.paidPrice = argMap["discountTotal"].toInt();
        totalBill.cardID = argMap["cardID"].toString();
        QString uid = PaymentAccess::insertBill(totalBill);

        if(uid.isEmpty())
        {
            answer["errcode"] = -4;
            answer["errstring"] = "The shopping cart has already been paid";
            Q_EMIT response(cbID, answer);
            return true;
        }

        user->setBalance(user->getBalance() - total);
        answer["balance"] = user->getBalance();
        answer["username"] = user->getName();
        answer["total"] = total;

        MailManager().sendMailFromTemplate(user->getEMail(),"member-payment", answer);
        QListIterator<Log> logIt(logs);
        while(logIt.hasNext())
        {
            auto log = logIt.next();
            log.externalReference = uid;
            LogAccess::instance()->insertLog(log);
        }
        Q_EMIT response(cbID, answer);
        return true;
    }
    return false;
}

QVariantMap PaymentService::prepareBill(QVariantMap argMap, QStringList extras)
{
    QString cardID = argMap["cardID"].toString();
    QString userID = argMap["userID"].toString();
    fablabUserPtr user;
    if(userID.isEmpty())
        user = UserAccess::instance()->getUserWithCard(cardID);
    else
        user = UserAccess::instance()->getUser(userID);

    QVariantMap answer;
    if(user.isNull())
    {
        answer["errcode"] = -2;
        answer["errstring"] = "Unknown user";
        return answer;
    }

    QVariantList bill = argMap["bill"].toList();

    // matches "10% Rabatt" "5% Kaffee-Rabatt" "12% Kaffee-Discount" and so on
    QRegExp rx("(\\d\\d?\\d?)%\\s?((?:\\w| )*)(?: |-|–)(?:Rabatt|Discount)");
    QStringList discount = extras.filter(rx);
    int totalDiscount = 0;
    QMap<QString, int> discountMap;
    QListIterator<QString> discIt(extras);
    while(discIt.hasNext())
    {
        if (rx.indexIn(discIt.next()) != -1 && rx.captureCount() >= 1)
        {
           if(rx.captureCount() >= 2)
           {
               discountMap.insert(rx.capturedTexts()[2].toLower().trimmed() ,rx.capturedTexts()[1].toInt());
           }
           totalDiscount = rx.capturedTexts()[1].toInt();
        }
    }

    qDebug()<<discountMap;

    QListIterator<QVariant> it(bill);

    QMap<QString, BillItem> billData;

    int discountTotal = 0;
    int total = 0;

    while(it.hasNext())
    {
        QVariantMap item  = it.next().toMap();
        QString accountingCode = item["accountingCode"].toString();

        bool isNew = !billData.contains(accountingCode);
        BillItem& bill = billData[accountingCode];
        if(isNew)
        {
            bill.accountingCode = accountingCode;
            bill.discountPercent = discountMap.value(accountingCode.toLower(), 0);
        }

        QString flatrateCategory = item["flatrateCategory"].toString();

        bool flat = extras.contains(flatrateCategory+"-flat", Qt::CaseInsensitive) || extras.contains(flatrateCategory+"-flatrate", Qt::CaseInsensitive);
        int price = item["price"].toInt();
        int newPrice = 0;

        if(!flat)
        {
            newPrice = int(price * ((100 - bill.discountPercent) / 100.0));
            bill.total +=  price;
        }

        bill.discountTotal += newPrice;
        discountTotal += newPrice;
        total += price;

        item["flat"] = flat;
        item["newprice"] = newPrice;
        bill.billItems << item;
    }

    answer["total"] = total;
    answer["discountTotal"] = discountTotal;

    QVariantList bills;
    QMapIterator<QString, BillItem> billIt(billData);
    while(billIt.hasNext())
    {
        BillItem item = billIt.next().value();
        bills << item.toVariant();
    }

    answer["bills"] = bills;

    answer["userID"] = user->identityID();
    answer["eMail"] = user->getEMail();
    answer["surname"] = user->getSurname();
    answer["name"] = user->getName();
    answer["errcode"] = 0;
    answer["errstring"] = "";
    answer["cardID"] = argMap["cardID"];
    answer["cartID"] = argMap["cartID"];
    return answer;
}

bool PaymentService::checkCobot(QString token)
{
    resourcePtr resource = ResourceManager::instance()->getOrCreateResource("object", "cobot/state", token);
    QSharedPointer<ObjectResource> objectResource = resource.objectCast<ObjectResource>();
    if(objectResource.isNull())
    {
        qDebug()<< "Object Resource for state is missing";
        return false;
    }
    QVariantMap data = objectResource->getObjectData();
    if(data["ConnectionState"].toMap()["data"].toString() != "CONNECTED")
    {
        qDebug()<< "Cobot is disconnected";
        return false;
    }

    if(_cobotService)
        return true;

    _cobotService = ServiceManager::instance()->service("cobotservice");
    if(_cobotService)
    {       
        connect(_cobotService, &IService::response, this, &PaymentService::responseFromCobot);
        return true;
    }

    return false;
}

void PaymentService::responseFromCobot(QString uid, QVariant answer)
{

    if(!_cobotCbMap.contains(uid))
    {
        qDebug()<<"Unknown callback-ID from payment plugin";
        qDebug()<<uid;
        return;
    }

    QVariantMap data = answer.toMap();
    Request req = _cobotCbMap.take(uid);

    if(data["success"].toBool())
    {
        qDebug()<<"A";
        QVariantList extrasAsVariant = data["extras"].toList();
        QStringList extras;
        QListIterator<QVariant> it(extrasAsVariant);
        while(it.hasNext())
        {
            QString extra = it.next().toString();
            if(!extra.isEmpty())
                extras << extra;
        }

        QVariantMap responseData  = prepareBill(req.data, extras);
        responseData["cobotSuccess"] = true;
        responseData["cobotConnected"] = true;
        Q_EMIT response(req.ownCbID, responseData);
    }
    else
    {
        qDebug()<<"B";
        QVariantMap responseData  =  prepareBill(req.data, QStringList());
        responseData["cobotSuccess"] = false;
        responseData["cobotConnected"] = true;
        Q_EMIT response(req.ownCbID, responseData);
    }
}
