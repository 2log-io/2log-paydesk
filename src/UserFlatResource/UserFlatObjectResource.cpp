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


#include "UserFlatObjectResource.h"
#include <QDebug>
#include "PaymentUserAccess/PaymentAccess.h"

UserFlatObjectResource::UserFlatObjectResource(fablabUserPtr user, QObject* parent) : ObjectResource(nullptr, parent),
    _userPtr(user)
{
    setDynamicContent(false);
    if(_userPtr.isNull())
        return;

    QVariantMap userdata = PaymentAccess::getUserRelatedPaymentData(_userPtr->getUuid(), "flatrates");
    QVariantMap data;
    QMapIterator<QString, QVariant>it(userdata);
    while(it.hasNext())
    {
        it.next();
        QVariantMap prop;
        prop["data"] = it.value();
        data.insert(it.key(), prop );
    }

    _data = data;
}

QVariantMap UserFlatObjectResource::getObjectData() const
{
    return _data;
}

IResource::ModificationResult UserFlatObjectResource::setProperty(QString name, const QVariant &value, QString token)
{
    ModificationResult result;
    iIdentityPtr executiveUser = AuthenticationService::instance()->validateToken(token);

    if(executiveUser.isNull() || ! (executiveUser->isAuthorizedTo(LAB_SERVICE) || executiveUser->isAuthorizedTo(LAB_ADMIN) || executiveUser->isAuthorizedTo(LAB_MODIFY_USERS)))
    {
        result.error = ResourceError::PERMISSION_DENIED;
        return result;
    }

    QVariantMap data = _data;
    data[name] = value;
    if(PaymentAccess::changeUserRelatedPaymentData(_userPtr->getUuid(), "flatrates", data))
    {
        result.error = NO_ERROR;
        Q_EMIT propertyChanged(name, value, nullptr);
    }


    result.error = INVALID_PARAMETERS;
    return result;
}


