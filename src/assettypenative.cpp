#include "assettypenative.h"



AssetTypeNative::AssetTypeNative() {

}

QString AssetTypeNative::getType() {
    return QString("native");
}

bool AssetTypeNative::equals(Asset* object) {
    return dynamic_cast<AssetTypeNative*>(object);
}

int AssetTypeNative::hashCode() {
    return 0;
}

stellar::Asset AssetTypeNative::toXdr() {
    stellar::Asset xdr;
    xdr.type = stellar::AssetType::ASSET_TYPE_NATIVE;
    return xdr;
}
