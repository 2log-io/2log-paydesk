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


#include "UserFlatObjectResourceFactory.h"
#include "UserFlatObjectResource.h"

UserFlatObjectResourceFactory::UserFlatObjectResourceFactory(QObject *parent) : IResourceFactory(parent)
{

}

UserFlatObjectResourceFactory::~UserFlatObjectResourceFactory()
{

}

QString UserFlatObjectResourceFactory::getResourceID(QString descriptor, QString token) const
{
    Q_UNUSED(token)
    return descriptor;
}

QString UserFlatObjectResourceFactory::getResourceType() const
{
    return "object";
}

QString UserFlatObjectResourceFactory::getDescriptorPrefix() const
{
    return "labcontrol_payment/user";
}

resourcePtr UserFlatObjectResourceFactory::createResource(QString token, QString descriptor, QObject *parent)
{
    Q_UNUSED(parent);
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull() || !( user->isAuthorizedTo(LAB_ADMIN) || user->isAuthorizedTo(IS_ADMIN) || user->isAuthorizedTo(LAB_SEE_USERS)))
        return resourcePtr();

    fablabUserPtr userPtr;
    QVariantMap parameters = parseParameters(descriptor);
    QString userID = parameters["userID"].toString();
    if(userID.isEmpty())
    {
        QString cardID = parameters["cardID"].toString();
        userPtr = UserAccess::instance()->getUserWithCard(cardID);
    }
    else
    {
        userPtr = UserAccess::instance()->getUser(userID);
    }


    if(userPtr.isNull())
        return resourcePtr();

    return resourcePtr(new UserFlatObjectResource(userPtr));
}
