#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use std::ffi::CStr;
use std::ffi::CString;
use std::ptr;

use ema_sys::*;

pub fn init() -> Result<(), i32> {
    let res = unsafe { EMA_init(None) };
    if res == 0 {
        Ok(())
    } else {
        Err(res)
    }
}

pub fn finalize() -> Result<(), i32> {
    let res = unsafe { EMA_finalize() };
    if res == 0 {
        Ok(())
    } else {
        Err(res)
    }
}

pub struct Device {
    inner: *mut ema_sys::Device,
}

impl Device {
    pub fn name(&self) -> &str {
        unsafe {
            let c_str = CStr::from_ptr(
                ema_sys::EMA_get_device_name(self.inner)
            );
            c_str.to_str().unwrap_or("<invalid utf8>")
        }
    }
}

pub struct Plugin {
    inner: *mut ema_sys::Plugin,
}

impl Plugin {
    pub fn name(&self) -> &str {
        unsafe {
            let c_str = CStr::from_ptr(
                ema_sys::EMA_get_plugin_name(self.inner)
            );
            c_str.to_str().unwrap_or("<invalid utf8>")
        }
    }
}

pub struct DeviceList {
    devices: Vec<Device>,
}

impl DeviceList {
    pub fn new() -> Self {
        let raw = unsafe { ema_sys::EMA_get_devices() };
        let slice = unsafe {
            std::slice::from_raw_parts(
                raw.array, raw.size as usize
            )
        };

        let devices = slice.iter()
            .map(|&ptr| Device { inner: ptr })
            .collect();

        DeviceList { devices }
    }

    pub fn size(&self) -> usize {
        self.devices.len()
    }

    pub fn iter(&self) -> impl Iterator<Item = &Device> {
        self.devices.iter()
    }
}

pub struct PluginList {
    plugins: Vec<Plugin>,
}

impl PluginList {
    pub fn new() -> Self {
        let raw = unsafe { ema_sys::EMA_get_plugins() };
        let slice = unsafe {
            std::slice::from_raw_parts(
                raw.array, raw.size as usize
            )
        };

        let plugins = slice.iter()
            .map(|&ptr| Plugin { inner: ptr })
            .collect();

        PluginList { plugins }
    }

    pub fn size(&self) -> usize {
        self.plugins.len()
    }

    pub fn iter(&self) -> impl Iterator<Item = &Plugin> {
        self.plugins.iter()
    }
}

pub fn get_time_in_us() -> u64 {
    unsafe { EMA_get_time_in_us() }
}

pub struct Filter {
    filter: *mut ema_sys::Filter,
}

impl Filter {
    pub fn new(name: &str) -> Self {
        let cname = CString::new(name).unwrap();
        let filter = unsafe {
            EMA_filter_exclude_plugin(cname.as_ptr()) 
        };

        Filter { filter }
    }

    pub fn finalize(&self) -> Result<(), i32> {
        let res = unsafe { EMA_filter_finalize(self.filter) };
        if res == 0 {
            Ok(())
        } else {
            Err(res)
        }
    }
}

pub struct Region {
    region: *mut ema_sys::Region,
}

impl Region {
    pub fn new(name: &str, filter: &Filter) -> Self {
        let cname = CString::new(name).unwrap();
        let file = CString::new(file!()).unwrap();
        let func = CString::new("rust").unwrap();

        let mut region: *mut ema_sys::Region = ptr::null_mut();
        unsafe {
            EMA_region_define(
                &mut region,
                cname.as_ptr(),
                filter.filter,
                file.as_ptr(),
                line!(),
                func.as_ptr(),
            );
        }

        Self { region }
    }

    pub fn begin(&self) {
        unsafe {
            EMA_region_begin(self.region);
        }
    }

    pub fn end(&self) {
        unsafe {
            EMA_region_end(self.region);
        }
    }
}
