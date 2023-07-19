# LIEM - ListsItemsEffectsManipulator

This is a SKSE plugin for Skyrim Special Edition that allows you to manipulate lists and containers, items, effects and so on at startup runtime with configuration files.  
No more patch hell for things like WACCF or CACO!

# Features

The configuration files are written in ini format and are loaded from the root `Data` folder of the game with a suffix `_LIEM(.ini)`.

## General syntax

In general the syntax is the one described below, similar to the one used in SPID or KID, and specialised on the key type.

```ini
#--Type---   -What to do|------------Where to do it------------|-----When to do it-----|
<RuleType> = <Modifiers>|<StringFilters>|<FormFilters>|<Traits>|<Chance>|<LoadCondition>
```

### Key: NONE

A `NONE` key (or blank) ignores the value that should be provided, telling it to skip.

### Operation: &lt;empty>/+/-

- `<empty>` Empty (or blank) means absolute modifier, thus overwriting the target with the value provided.
- `+` Plus sign means relative adding modifier, thus adding the value to the target.
- `-` Minus sign means relative substraction modifier, thus substracting the value from the target.

Examples:
```ini
# nerfs all cuirass armors tagged as Iron by adding 5 more weight and removing 2 points of rating
# E.g. "Iron Armor" becomes 35 of weight and 23 of rating (see table https://elderscrolls.fandom.com/wiki/Iron_Armor_(Skyrim)#Attributes_by_piece)
Armor = NONE,+5,-2|ArmorMaterialIron+ArmorCuirass
# buffs all helmets tagged as Iron by making their weight be 4
# E.g. "Iron Helmet" has it's weigth set to 4 (instead of previous 5)
Armor = NONE,4,|ArmorMaterialIron+ArmorHelmet
```

### Type: integer

Form: `<digits>`
A [natural number](https://www.wikiwand.com/en/Natural_number) written with digits.

### Type: real number

Form: `<digits>[.<digits>]`
A [real number](https://www.wikiwand.com/en/Real_number) written with digits and optionally a deciaml part.

## RuleType

Supported types are:
- `Armor` for armors and clothing (including shields and jewelry) [[ARMO]](#ARMO)
- `Weapon` for weapons [[WEAP]](WEAP)
- `Ammo` for ammonitions (bolts and arrows) [[AMMO]](AMMO)

<a name="ARMO"></a>
### Armor

`Armor = value,weight,rating|...`

| Operation      | Name   | Type        | Description              |
|----------------|--------|-------------|--------------------------|
| &lt;empty>/+/- | value  | integer     | Base value of the armor  |
| &lt;empty>/+/- | weight | real number | Base weight of the armor |
| &lt;empty>/+/- | rating | real number | Base rating of the armor |

<a name="WEAP"></a>
### Weapon

`Weapon = value,weight,damage,critDamage,speed,reach,stagger|...`

| Operation      | Name       | Type        | Description                        |
|----------------|------------|-------------|------------------------------------|
| &lt;empty>/+/- | value      | integer     | Base value of the weapon           |
| &lt;empty>/+/- | weight     | real number | Base weight of the weapon          |
| &lt;empty>/+/- | damage     | integer     | Base damage of the weapon          |
| &lt;empty>/+/- | critDamage | integer     | Base critical damage of the weapon |
| &lt;empty>/+/- | speed      | real number | Base speed of the weapon           |
| &lt;empty>/+/- | reach      | real number | Base reach of the weapon           |
| &lt;empty>/+/- | stagger    | real number | Base stagger of the weapon         |

<a name="AMMO"></a>
### Ammo

`Ammo = value,weight,damage|...`

| Operation      | Name    | Type        | Description             |
|----------------|---------|-------------|-------------------------|
| &lt;empty>/+/- | value   | integer     | Base value of the ammo  |
| &lt;empty>/+/- | weight  | real number | Base weight of the ammo |
| &lt;empty>/+/- | damage  | real number | Base damage of the ammo |


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

## MO2 deployment

There is a CMake rule to automatically deploy the plugin to MO2's own plugin folder if you have set the `MO2SkyrimSEPath` environment variable to the path of your MO2 installation.
