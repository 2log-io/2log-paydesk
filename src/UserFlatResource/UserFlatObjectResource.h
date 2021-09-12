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


#ifndef USERFlatOBJECTRESOURCE_H
#define USERFlatOBJECTRESOURCE_H

#include <QObject>
#include "Server/Resources/ObjectResource/ObjectResource.h"
#include "Database/UserAccess.h"

class UserFlatObjectResource : public ObjectResource
{
    Q_OBJECT

public:
    UserFlatObjectResource(fablabUserPtr user, QObject* parent = nullptr);
    QVariantMap getObjectData() const;
    ModificationResult setProperty(QString name, const QVariant &value, QString token);

private:
    QVariantMap _data;
    fablabUserPtr _userPtr;
};

#endif // USERFlatOBJECTRESOURCE_H
