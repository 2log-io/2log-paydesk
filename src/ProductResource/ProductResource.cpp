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


#include "ProductResource.h"
#include "Storage/ListResourceTemporaryStorage.h"


ProductResource::ProductResource(QObject *parent) : ListResource(nullptr, parent)
{
    setAllowUserAccess(false);
    setDynamicContent(false);
    _products = PaymentAccess::getProducts();
    _categories = getCategories();
    _flatrateCategories = getFlatrateCategories();
    _accountingCodes = getAccountingCodes();
}

QVariantList ProductResource::getListData() const
{
    QVariantList data;
    QListIterator<PaymentAccess::Product> it(_products);
    while(it.hasNext())
    {
        data << createTemplate(it.next().toVariant());
    }

    return data;
}

int ProductResource::getCount() const
{
    return _products.count();
}

QVariant ProductResource::getItem(int idx, QString uuid) const
{
    Q_UNUSED(uuid)
    QVariant item = createTemplate(_products.at(idx).toVariant());
    return item;
}

IResource::ModificationResult ProductResource::appendItem(QVariant data, QString token)
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    ModificationResult result;

    if(user.isNull())
    {
        result.error = PERMISSION_DENIED;
        return result;
    }

    PaymentAccess::Product product(data.toMap());

    if(!product.isValid())
    {
        result.error = INVALID_PARAMETERS;
        return result;
    }

    product.uuid = PaymentAccess::insertProduct(product);
    if(!product.uuid.isEmpty())
    {
        result.data = createTemplate(product.toVariant());
        _products << product;
        Q_EMIT itemAppended(result.data, user);
        bool changed = false;

        if(!_categories.contains(product.category))
        {
            _categories << product.category;
            changed = true;
        }

        if(!_flatrateCategories.contains(product.flatrateCategory))
        {
            _flatrateCategories << product.flatrateCategory;
            changed = true;
        }

        if(!_accountingCodes.contains(product.accountingCode))
        {
            _accountingCodes << product.accountingCode;
            changed = true;
        }

        if(changed)
            Q_EMIT metadataChanged();
    }
    else
    {
        result.error = STORAGE_ERROR;
    }

    return result;
}

IResource::ModificationResult ProductResource::removeItem(QString uuid, QString token, int index)
{
     iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
     ModificationResult result;
     if(user.isNull())
     {
         result.error = PERMISSION_DENIED;
         return result;
     }

    int idx = getOrCorrectIndex(uuid, index);

    if(idx < 0)
        result.error = INVALID_PARAMETERS;


    if(!PaymentAccess::deleteProduct(uuid))
    {
        result.error = STORAGE_ERROR;
    }

    _products.removeAt(idx);
    Q_EMIT itemRemoved(idx, uuid, user);
    _categories = getCategories();
    _flatrateCategories = getFlatrateCategories();
    Q_EMIT metadataChanged();
    return result;
}

IResource::ModificationResult ProductResource::setProperty(QString property, QVariant data, int index, QString uuid, QString token)
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    ModificationResult result;
    if(user.isNull())
    {
        result.error = PERMISSION_DENIED;
        return result;
    }

    int idx = getOrCorrectIndex(uuid, index);
    PaymentAccess::Product product = _products.at(idx);

    if(property == "price")
    {
        product.price = data.toInt();
    }
    else if(property == "name")
    {
        product.name = data.toString();
    }
    else if(property == "flatrateCategory")
    {
        product.flatrateCategory = data.toString();
    }
    else if(property == "imageURL")
    {
        product.imageURL = data.toString();
    }
    else if(property == "category")
    {
        product.category = data.toString();
    }
    else if(property == "selfService")
    {
        product.selfService = data.toBool();
    }
    else if(property == "accountingCode")
    {
        product.accountingCode = data.toString();
    }
    else
    {
        result.error = INVALID_PARAMETERS;
        return result;
    }

    if(!PaymentAccess::updateProduct(product))
    {
        result.error = STORAGE_ERROR;
        return result;
    }

    _products.replace(index, product);

    if( property == "category")
    {
        _categories = getCategories();
        Q_EMIT metadataChanged();
    }

    else if(property == "flatrateCategory")
    {
        _flatrateCategories = getFlatrateCategories();
        Q_EMIT metadataChanged();
    }

    else if(property == "accountingCode")
    {
        _accountingCodes = getAccountingCodes();
        Q_EMIT metadataChanged();
    }

    Q_EMIT propertySet(property, data, index, uuid, user, 0);
    result.data = createTemplate(product.toVariant());
    return result;
}

QVariantMap ProductResource::getMetadata() const
{
    QVariantList cat;
    QSetIterator<QString> catIt(_categories);
    while(catIt.hasNext())
        cat << catIt.next();

    QVariantList flatrateCat;
    QSetIterator<QString> flatrateIt(_flatrateCategories);
    while(flatrateIt.hasNext())
        flatrateCat << flatrateIt.next();

    QVariantList accountingCodes;
    QSetIterator<QString> accountingIt(_accountingCodes);
    while(accountingIt.hasNext())
        accountingCodes << accountingIt.next();

    QVariantMap metadata;
    metadata["categories"] =  cat;
    metadata["flatrateCategories"] =  flatrateCat;
    metadata["accountingCodes"] =  accountingCodes;
    return metadata;
}

QVariantMap ProductResource::createTemplate(QVariantMap data) const
{
    QVariantMap item;
    item["data"] = data;
    item["uuid"] = data["uuid"];
    return item;
}

int ProductResource::getOrCorrectIndex(QString id, int idx)
{
    if(_products[idx].uuid == id || id.isEmpty())
        return idx;

    int index = 0;
    QListIterator<PaymentAccess::Product> it(_products);
    while(it.hasNext())
    {
        if(it.next().uuid == id)
            return index;
        index++;
    }
    return -1;
}

QSet<QString> ProductResource::getCategories() const
{
    QSet<QString> categories;
    QListIterator<PaymentAccess::Product> it(_products);
    while(it.hasNext())
    {
        categories << it.next().category;
    }

    return categories;
}


QSet<QString> ProductResource::getFlatrateCategories() const
{
    QSet<QString> categories;
    QListIterator<PaymentAccess::Product> it(_products);
    while(it.hasNext())
    {
        categories << it.next().flatrateCategory;
    }

    return categories;
}

QSet<QString> ProductResource::getAccountingCodes() const
{
    QSet<QString> codes;
    QListIterator<PaymentAccess::Product> it(_products);
    while(it.hasNext())
    {
        codes << it.next().accountingCode;
    }

    return codes;
}

