# iwinfo_snmp

Very basic C program that uses libiwinfo to return specific information over SNMP about hard-coded wireless interfaces in OpenWRT. Requires snmpd on your router/AP. See the source file for what is currently supported.

1. Follow the instructions [here](https://openwrt.org/docs/guide-developer/toolchain/single.package) (and related pages) to set up a build system for your target platform, then build iwinfo with `make package/iwinfo/compile`. You do not need to build the entire firmware.
2. In your openwrt folder, go to `build_dir/target-<targetname>/libiwinfo-<version>`. Add `iwinfo_snmp.c` from this repository, and patch the Makefile with Makefile.patch also found in this repo
3. Re-run `make package/iwinfo/compile`
4. Copy your shiny new iwinfo_snmp binary to your target device
5. At the bottom of /etc/config/snmp, add these lines:

    ```conf
    config pass
        option name iwinfo_snmp
        option miboid .1.3.9950
        option persist '1'
        option prog '/path/to/iwinfo_snmp'
    ```
6. Restart snmpd
7. From another device, run `snmpwalk -On -v 2c -c public 192.168.1.2 .1.3.9950` to make sure you are getting data.
