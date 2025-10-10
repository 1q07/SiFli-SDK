# RT-PKG Command Usage Guide 

## Environment Configuration
After configuring the environment using `.\export.ps1` in the SDK path, switch to the project directory where packages need to be configured, and use the following command to enter the configuration interface: 
```shell
scons --board=sf32lb52-lcd_n16r8 --menuconfig
```
After entering the configuration interface, select `RT-Thread online packages` to configure the relevant software packages.
**Note**: When using it for the first time, you need to execute `sdk.py rt-pkg-upgrade` to download and update the software package configuration. 
![](../../assets/rt_pkg_option_packages.png)


## Command List 

1. View help information 
```shell
sdk.py --help
```
This command can be used to query the usage instructions of rt-pkg related commands.
**Note**: After using this command, if there are no rt-pkg related commands under the "Commands" section, please update the local SDK. ---
![](../../assets/rt_pkg_sdk.py--help.png)


2. List the configured software packages 
```shell
sdk.py rt-pkg-list
```
After adding a new software package in `menuconfig`, you can use this command to output the added software packages. For example, in `menuconfig`, select `RT-Thread online packages` and press Enter to enter. This includes all configurable software packages. Here, we go to the `multimedia packages` section, press the space bar to select Openmv, save and exit. 
![](../../assets/rt_pkg_sdk.py_rt-pkg-list_1.png)
By using this command, you can view the software packages that have been added as well as their respective versions. 
![](../../assets/rt_pkg_sdk.py_rt-pkg-list_2.png)
---


3. Update software package configuration 
```shell
sdk.py rt-pkg-update
```
After adding a new software package in `menuconfig`, use this command to download the corresponding software package. The operation of adding the software package is the same as that in `sdk.py rt-pkg-list`. 
![](../../assets/sdk.py_rt-pkg-update_add.png)
If the selection of a software package is deselected in menuconfig, use this command to delete the previously downloaded software package. For example, in `menuconfig`, if the selection of the Openmv software package is removed, save and exit. Using this command, input 'Y' to delete the corresponding software package. By using `sdk.py rt-pkg-list`, you can see that the added Openmv software package has been deleted. 
![](../../assets/rt_pkg_sdk.py_rt-pkg-update_delete.png)
---


4. Print environment variables 
```shell
sdk.py rt-pkg-printenv
```
This command can print the environment variables for inspection. The output is shown as follows in the figure. 
![](../../assets/sdk.py_rt-pkg-printenv.png)

---


5. Upgrade the software package 
```shell
sdk.py rt-pkg-upgrade
```
Using this command allows you to upgrade the already configured software packages. When using rt-pkg for the first time, you need to use this command to configure the software packages. As shown in the figure, this command was used to upgrade the `Env packages`.
![](../../assets/rt-pkg-upgrade.png)
---


6. Upgrade Python Modules 
```shell
sdk.py rt-pkg-upgrade-modules
```
Use this command to upgrade the Python module object (such as requests). 

---


7. Create a new software package 
```shell
sdk.py rt-pkg-wizard
```
Using this command, you can create a new software package using the wizard. 

## Usage Flow Suggestions
1. First, use `sdk.py --help` to view the available commands.
2. Use `sdk.py rt-pkg-list` to view the current configuration.
3. Configure the required software packages through menuconfig.
4. Use `sdk.py rt-pkg-update` to update the configuration. Format it properly.