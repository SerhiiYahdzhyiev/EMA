fn main() {
    println!("Initializing EMA...");
    EMA::init().expect("Failed to init EMA!");

    let plugins = EMA::PluginList::new();
    println!("Found {} plugins:", plugins.size());

    for plugin in plugins.iter() {
        println!("\t{}", plugin.name());
    }

    let devices = EMA::DeviceList::new();
    println!("Found {} devices:", devices.size());

    for device in devices.iter() {
        println!("\t{}", device.name());
    }

    let filter = EMA::Filter::new("NVML");

    let region = EMA::Region::new("region", &filter);
    region.begin();
    std::thread::sleep(std::time::Duration::from_secs(5));
    region.end();

    filter.finalize().expect("Failed to finalize filter!");

    println!("Finalizing EMA...");
    EMA::finalize().expect("Failed to finalize EMA!");
}
