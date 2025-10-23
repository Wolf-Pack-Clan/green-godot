#include "modules/efx/resource_format_effect.h"
#include "core/os/file_access.h"

RES ResourceFormatEFX::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_no_subresource_cache) {
	if (r_error) {
		*r_error = ERR_CANT_OPEN;
	}

	Error err;
	FileAccess *f = FileAccess::open(p_path, FileAccess::READ, &err);
	if (!f) {
		return RES();
	}

	FileAccessRef fref(f);
	if (r_error) {
		*r_error = ERR_FILE_CORRUPT;
	}

	ERR_FAIL_COND_V_MSG(err != OK, RES(), "Unable to open EFX effect file '" + p_path + "'.");
	return RES();
}

void ResourceFormatEFX::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("efx");
}

bool ResourceFormatEFX::handles_type(const String &p_type) const {
	return ClassDB::is_parent_class(p_type, "Effect");
}

String ResourceFormatEFX::get_resource_type(const String &p_path) const {
	if (p_path.get_extension().to_lower() == "efx") {
		return "EffectFormat";
	}
	return "";
}
