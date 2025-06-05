#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use std::os::raw::{c_char, c_int, c_uint, c_ulonglong};

#[repr(C)]
pub struct Device {
     _private: core::ffi::c_void,
}
#[repr(C)]
pub struct Plugin {
     _private: core::ffi::c_void,
}
#[repr(C)]
pub struct Region {
     _private: core::ffi::c_void,
}
#[repr(C)]
pub struct Filter {
     _private: core::ffi::c_void,
}

#[repr(C)]
pub struct DevicePtrArray {
    pub array: *mut *mut Device,
    pub size: usize,
}

#[repr(C)]
pub struct PluginPtrArray {
    pub array: *mut *mut Plugin,
    pub size: usize,
}

unsafe extern "C" {
    // Init / Finalize
    pub fn EMA_init(cb: Option<extern "C" fn() -> c_int>) -> c_int;
    pub fn EMA_finalize() -> c_int;

    // Plugin/device access
    pub fn EMA_get_plugins() -> PluginPtrArray;
    pub fn EMA_get_devices() -> DevicePtrArray;
    pub fn EMA_get_plugin_name(plugin: *const Plugin) -> *const c_char;
    pub fn EMA_get_device_name(device: *const Device) -> *const c_char;
    pub fn EMA_plugin_init(plugin: *mut Plugin) -> c_int;
    pub fn EMA_plugin_finalize(plugin: *mut Plugin) -> c_int;
    pub fn EMA_get_plugin_devices(plugin: *const Plugin) -> DevicePtrArray;
    pub fn EMA_get_energy_uj(device: *const Device) -> c_ulonglong;
    pub fn EMA_plugin_get_energy_uj(device: *const Device) -> c_ulonglong;

    // Regions
    pub fn EMA_region_create_and_init(
        region: *mut *mut Region,
        idf: *const c_char,
        filter: *mut Filter,
        file: *const c_char,
        line: c_uint,
        func: *const c_char,
    ) -> c_int;

    pub fn EMA_region_define(
        region: *mut *mut Region,
        idf: *const c_char,
        filter: *mut Filter,
        file: *const c_char,
        line: c_uint,
        func: *const c_char,
    ) -> c_int;

    pub fn EMA_region_begin(region: *mut Region) -> c_int;
    pub fn EMA_region_end(region: *mut Region) -> c_int;
    pub fn EMA_region_finalize(region: *mut Region) -> c_int;

    // Filters
    pub fn EMA_filter_exclude_plugin(name: *const c_char) -> *mut Filter;
    pub fn EMA_filter_finalize(filter: *mut Filter) -> c_int;

    // Output
    pub fn EMA_print_all(f: *mut libc::FILE) -> c_int;

    // Time
    pub fn EMA_get_time_in_us() -> c_ulonglong;
}
