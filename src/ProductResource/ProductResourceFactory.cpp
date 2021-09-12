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


#include "ProductResourceFactory.h"
#include "ProductResource/ProductResource.h"


ProductResourceFactory::ProductResourceFactory(QObject *parent) : IResourceFactory(parent)
{
}

QString ProductResourceFactory::getResourceID(QString descriptor, QString token) const
{
    Q_UNUSED(token)
    return descriptor;
}

QString ProductResourceFactory::getResourceType() const
{
    return "synclist";
}

QString ProductResourceFactory::getDescriptorPrefix() const
{
    return "labcontrol_payment/products";
}

resourcePtr ProductResourceFactory::createResource(QString token, QString descriptor, QObject *parent)
{
    Q_UNUSED(descriptor)
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull())
        return resourcePtr();

    return resourcePtr(new ProductResource());
}
