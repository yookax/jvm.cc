﻿module;
#include "vmdef.h"

export module properties;

import std.core;
import sysinfo;
import class_loader;

using namespace std;

// jdk/internal/util/SystemProps.java

enum {
    _display_country_NDX = 0, // "user.country"
    _display_language_NDX, // "user.language"
    _display_script_NDX, // "user.script"
    _display_variant_NDX, // "user.variant"
    _file_encoding_NDX,
    _file_separator_NDX,
    _format_country_NDX,
    _format_language_NDX,
    _format_script_NDX,
    _format_variant_NDX,
    _ftp_nonProxyHosts_NDX,
    _ftp_proxyHost_NDX,
    _ftp_proxyPort_NDX,
    _http_nonProxyHosts_NDX,
    _http_proxyHost_NDX,
    _http_proxyPort_NDX,
    _https_proxyHost_NDX,
    _https_proxyPort_NDX,
    _java_io_tmpdir_NDX,
    _line_separator_NDX,
    _os_arch_NDX,
    _os_name_NDX,
    _os_version_NDX,
    _path_separator_NDX,
    _socksNonProxyHosts_NDX,
    _socksProxyHost_NDX,
    _socksProxyPort_NDX,
    _stderr_encoding_NDX,
    _stdout_encoding_NDX,
    _sun_arch_abi_NDX,
    _sun_arch_data_model_NDX,
    _sun_cpu_endian_NDX,
    _sun_cpu_isalist_NDX,
    _sun_io_unicode_encoding_NDX,
    _sun_jnu_encoding_NDX,
    _sun_os_patch_level_NDX,
    _user_dir_NDX,
    _user_home_NDX,
    _user_name_NDX,
    FIXED_LENGTH
};

static const char *platform_properties[FIXED_LENGTH] = { nullptr };

export pair<const char **, int> get_platform_properties() {
    return make_pair(platform_properties, FIXED_LENGTH);
}

static vector<pair<const char *, const char *>> vm_properties;

export vector<pair<const char *, const char *>>& get_vm_properties() {
    return vm_properties;
}

export void init_properties() {
    vm_properties.emplace_back("java.home", g_java_home.c_str());
    vm_properties.emplace_back("java.class.path", get_classpath());

    // ------------------------------------------------------------------------

    platform_properties[_os_name_NDX] = os_name();
    platform_properties[_os_arch_NDX] = os_arch();
    platform_properties[_os_version_NDX] = strdup(os_version().c_str());
    platform_properties[_user_name_NDX] = strdup(get_user_name().c_str());
    platform_properties[_user_home_NDX] = strdup(get_user_home_dir().c_str());
    platform_properties[_user_dir_NDX] = get_current_working_directory();
    platform_properties[_line_separator_NDX] = line_separator();
    platform_properties[_file_encoding_NDX] = "UTF-8";
    platform_properties[_sun_jnu_encoding_NDX] = "UTF-8";
    platform_properties[_file_separator_NDX] = file_separator();
    platform_properties[_path_separator_NDX] = path_separator();
    platform_properties[_java_io_tmpdir_NDX] = ""; // todo
}

// ---------------------------------------------------------------------------------------

export TEST_CASE(test_properties)
    // todo
}

