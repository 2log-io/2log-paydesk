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


#ifndef PRODUCTRESOURCE_H
#define PRODUCTRESOURCE_H

#include <QObject>
#include "Server/Resources/ListResource/ListResource.h"

#include "PaymentUserAccess/PaymentAccess.h"
class ProductResource : public ListResource
{
    Q_OBJECT

public:
    explicit                   ProductResource(QObject *parent = nullptr);
    QVariantList               getListData() const override;
    virtual int                getCount() const override;
    QVariant                   getItem(int idx, QString uuid) const override;
    virtual ModificationResult appendItem(QVariant data, QString token) override;
    virtual ModificationResult removeItem(QString uuid, QString token, int index = -1) override;
    virtual ModificationResult setProperty(QString property, QVariant data, int index, QString uuid, QString token) override;
    virtual QVariantMap        getMetadata() const override;

private:
    QList<PaymentAccess::Product> _products;
    QVariantMap createTemplate(QVariantMap data) const;
    int getOrCorrectIndex(QString id, int idx);
    QSet<QString> _categories;
    QSet<QString> _flatrateCategories;
    QSet<QString> _accountingCodes;
    QSet<QString> getCategories() const;
    QSet<QString> getFlatrateCategories() const;
    QSet<QString> getAccountingCodes() const;
};

#endif // PRODUCTRESOURCE_H
