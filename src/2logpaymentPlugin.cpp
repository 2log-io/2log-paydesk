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


#include "2logpaymentPlugin.h"
#include "Server/Resources/ResourceManager/ResourceManager.h"
#include "Server/Services/ServiceManager.h"
#include <QDebug>
#include "src/UserFlatResource/UserFlatObjectResourceFactory.h"
#include "src/ProductResource/ProductResourceFactory.h"
#include "src/PaymentService/PaymentService.h"


_2logpaymentPlugin::_2logpaymentPlugin(QObject *parent) : IPlugin(parent)
{
}

bool _2logpaymentPlugin::init(QVariantMap parameters)
{
    Q_UNUSED(parameters)
    ResourceManager::instance()->addResourceFactory(new UserFlatObjectResourceFactory(this));
    ResourceManager::instance()->addResourceFactory(new ProductResourceFactory(this));
    ServiceManager::instance()->registerService(new PaymentService(this));
    return true;
}

bool _2logpaymentPlugin::shutdown()
{
    return false;
}

QString _2logpaymentPlugin::getPluginName()
{
    return "2log-paydesk";
}
