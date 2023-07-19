# LIEM - ListsItemsEffectsManipulator

This is a SKSE plugin for Skyrim Special Edition that allows you to manipulate lists and containers, items, effects and so on at startup runtime with configuration files.
No more patch hell for things like WACCF or CACO!

# Features

The configuration files are written in ini format and are loaded from the root `Data` folder of the game with a suffix `_LIEM(.ini)`.

## General syntax

```ini
#On what-   -What to do|------------Where to do it------------|-----When to do it-----|
<KeyType> = <Modifiers>|<StringFilters>|<FormFilters>|<Traits>|<Chance>|<LoadCondition>
```

In general the syntax is the one described above, similar to the one used in SPID or KID, and specialised on the key type (see below).

## Rule


# Requirements

- [Visual Studio 2022](https://visualstudio.microsoft.com/) (_the free Community edition_)
- [`vcpkg`](https://github.com/microsoft/vcpkg)
  - 1. Clone the repository using git OR [download it as a .zip](https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip)
  - 2. Go into the `vcpkg` folder and double-click on `bootstrap-vcpkg.bat`
  - 3. Edit your system or user Environment Variables and add a new one:
    - Name: `VCPKG_ROOT`  
      Value: `C:\path\to\wherever\your\vcpkg\folder\is`
- [DKUtil](https://github.com/gottyduke/DKUtil)
  - 1. Clone the repository using git
  - 2. Point an ENV variable to its folder
    - Name: `DKUtilPath`
      Value: `C:\path\to\wherever\your\DKUtil\folder\is`

<img src="https://raw.githubusercontent.com/SkyrimDev/Images/main/images/screenshots/Setting%20Environment%20Variables/VCPKG_ROOT.png" height="150">

## MO2 deployment

There is a CMake rule to automatically deploy the plugin to MO2's own plugin folder if you have set the `MO2SkyrimSEPath` environment variable to the path of your MO2 installation.
