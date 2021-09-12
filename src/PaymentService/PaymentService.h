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


#ifndef PAYMENTSERVICE_H
#define PAYMENTSERVICE_H

#include <QObject>
#include "Server/Services/IService.h"
#include "Server/Services/ServiceManager.h"
#include <QDateTime>

class PaymentService : public IService
{
    Q_OBJECT
public:

    struct Request
    {
        QString ownCbID;
        QString cobotCbID;
        QVariantMap data;
        QDateTime ts = QDateTime::currentDateTime();
    };


    struct BillItem
    {
      int total;
      int discountTotal;
      int discountPercent;
      QString accountingCode;
      QVariantList billItems;
      QVariantMap toVariant()
      {
          QVariantMap bill;
          bill["totalBrutto"] = total;
          bill["totalNetto"] = discountTotal;
          bill["discountPercent"] = discountPercent;
          bill["accountingCode"] = accountingCode ;
          bill["items"] = billItems;
          return bill;
      }
    };
    explicit PaymentService(QObject *parent = nullptr);

    QString                 getServiceName();
    QStringList             getServiceCalls();
    bool                    call(QString call, QString token, QString cbID, QVariant argument = QVariant());
    QVariantMap             prepareBill(QVariantMap argMap, QStringList extras);

private:
    bool                    checkCobot(QString token);
    IService*               _cobotService = nullptr;
    QMap<QString, Request>  _cobotCbMap;

private slots:
    void responseFromCobot(QString uid, QVariant answer);
};

#endif // PAYMENTSERVICE_H
