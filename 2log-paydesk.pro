#   2log.io
#   Copyright (C) 2021 - 2log.io | mail@2log.io,  mail@friedemann-metzger.de
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Affero General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Affero General Public License for more details.
#
#   You should have received a copy of the GNU Affero General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.


TARGET = 2log-payment

QT          += network
DEFINES     += TWOLOGPAYMENT_LIBRARY

INCLUDEPATH += /usr/local/include/bsoncxx/v_noabi
DEPENDPATH += /usr/local/include/bsoncxx/v_noabi

INCLUDEPATH += /usr/local/include/mongocxx/v_noabi
DEPENDPATH += /usr/local/include/mongocxx/v_noabi
LIBS += -L/usr/local/lib -lmongocxx -lbsoncxx


include(../buildconfig.pri)

unix {
#    target.path = /usr/lib
#    INSTALLS += target
}
CONFIG += c++14

DISTFILES += \
    src/2logpaymentPlugin.json

HEADERS += \
    src/2logpaymentPlugin.h \
    src/2logpayment_global.h \
    src/PaymentService/PaymentService.h \
    src/PaymentUserAccess/PaymentAccess.h \
    src/ProductResource/ProductResource.h \
    src/ProductResource/ProductResourceFactory.h \
    src/UserFlatResource/UserFlatObjectResource.h \
    src/UserFlatResource/UserFlatObjectResourceFactory.h

SOURCES += \
    src/2logpaymentPlugin.cpp \
    src/PaymentService/PaymentService.cpp \
    src/PaymentUserAccess/PaymentAccess.cpp \
    src/ProductResource/ProductResource.cpp \
    src/ProductResource/ProductResourceFactory.cpp \
    src/UserFlatResource/UserFlatObjectResource.cpp \
    src/UserFlatResource/UserFlatObjectResourceFactory.cpp
