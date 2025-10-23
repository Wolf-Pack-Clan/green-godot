#include "modules/efx/register_types.h"
#include "modules/efx/resource_format_effect.h"

static Ref<ResourceFormatEFX> resource_loader_efx;

void register_efx_types() {
    resource_loader_efx.instance();
    ResourceLoader::add_resource_format_loader(resource_loader_efx);
}

void unregister_efx_types() {
    ResourceLoader::remove_resource_format_loader(resource_loader_efx);
    resource_loader_efx.unref();
}
