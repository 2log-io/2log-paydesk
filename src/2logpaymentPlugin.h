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


#ifndef _2LOGPAYMENT_H
#define _2LOGPAYMENT_H

#include <QObject>
#include "IPlugin.h"
#include "2logpayment_global.h"

class TWOLOGPAYMENTSHARED_EXPORT _2logpaymentPlugin : public  IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPlugin_iid FILE "2logpaymentPlugin.json")
    Q_INTERFACES(IPlugin)

public:
    explicit _2logpaymentPlugin(QObject *parent = nullptr);
    virtual bool init(QVariantMap parameters);
    virtual bool shutdown();
    virtual QString getPluginName();
};

#endif // _2LOGPAYMENT_H
