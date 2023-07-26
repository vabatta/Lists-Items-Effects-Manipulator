# LIEM - Lists Items Effects Manipulator [WIP]

This is a SKSE plugin for Skyrim Special Edition that allows you to manipulate lists and containers, items, effects and so on at startup runtime with configuration files.  
No more patch hell for things like WACCF or CACO!

# Features

The configuration files are written in ini format and are loaded from the root `Data` folder of the game with a suffix `_LIEM(.ini)`.

## General note

This allows you to mass change many values that might not be supposed to be changed (for example Armors `ARMO` used as "skins" fx for thralls, meaning that adding rating value buffs them inadvertently).  
This is thought mostly as a companion to mods which adds and sets up keywords to **all forms** correctly, eventually even more than the base game so that you can really create a carefully patch-less world (e.g. KID and related are a good example).

## Planning

Still very much a WIP. Plan to cover all possible forms and records within them where make sense (containers or leveled lists for items, ingredients or consumables for effects.

# General syntax

> NOTE: Everything is **case-insensitive**.

In general the syntax is the one described below, similar to the one used in SPID or KID, and specialised on the key type.

```ini
#--Type----|-What to do-|------------Where to do it------------|-----When to do it-----|
<RuleType> = <Modifiers>|<StringFilters>|<FormFilters>|<Traits>|<Chance>|<LoadCondition>
```

Think of a rule as a set of "type", "what to do", "where to do it" and "when to do it".  

### Modifier - "what to do"
The "modifier" part refers to the "what to do" part, which is _almost always_ specialised for the rule type (Armor-specific, Weapon-specific, ...).  

<a name="appliers"></a>
### Appliers - "where to do it" & "when to do it"
The "appliers" part refers to "where" and "when" to do it parts. It encludes many different ways of filtering the final target records.  
Because this is common and/or true for most rules, it can also be enclosed in an expansable/reusable rule `Alias` at the top of your file and use a special syntax in `StringFilters` to expand it. (see [Alias](#alias))

## Definitions

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
# E.g. "Iron Helmet" has it's weight set to 4 (instead of previous 5)
Armor = NONE,4,|ArmorMaterialIron+ArmorHelmet
```

### Type: integer

Form: `<digits>`  
A [natural number](https://www.wikiwand.com/en/Natural_number) written with digits.

### Type: real number

Form: `<digits>[.<digits>]`  
A [real number](https://www.wikiwand.com/en/Real_number) written with digits and optionally a deciaml part.  
> Note: Trailing `0` is required. `.5` is not valid but `0.5` is.

## RuleType

Supported special rules:
- `Alias` for aliasing the right part of rules (["Appliers"](#appliers)) [Alias](#alias)

Supported types related to game form records:
- `Armor` for armors and clothing (including shields and jewelry) [[ARMO]](#ARMO)
- `Weapon` for weapons [[WEAP]](#WEAP)
- `Ammo` for ammonitions (bolts and arrows) [[AMMO]](#AMMO)

<a name="alias"></a>
### Alias

`Alias = name|...`

It's a simple `name` to "Appliers" expansion rule. In `StringFilters` use `@<name>` to target an alias and have it expanded.

> Note: It should not matter but aliases gets expanded and added _at the end_ before storing the appliers.

Example:
```ini
Alias = IronHelmets|ArmorMaterialIron+ArmorHelmet
# buff all values of iron helmets to 100 but MySpecialHelmet (edid)
Armor = 100|@IronHelmets,-MySpecialHelmet
# is equivalent to
Armor = 100|-MySpecialHelmet,ArmorMaterialIron+ArmorHelmet
```

<a name="ARMO"></a>
### Armor

`Armor = value,weight,rating|...`

| Operation      | Name   | Type        | Description         |
|----------------|--------|-------------|---------------------|
| &lt;empty>/+/- | value  | integer     | Value of the armor  |
| &lt;empty>/+/- | weight | real number | Weight of the armor |
| &lt;empty>/+/- | rating | real number | Rating of the armor |

#### String Filters

It supports the following string filters:
  - Keywords it has (e.g. `ArmorMaterialIron`)
  - Armor editorID (e.g. `ArmorIronShield`)
  - Armor name (e.g. `Iron Shield`)

#### Form Filters

It supports the following form filters:
  - Leveled list it is in (e.g. `LItemClothesAll`)
  - Outfit it belongs to (e.g. `BeggarWithHatOutfit`)
  - Form list it is in (e.g. `WICommentCollegeRobesList`)

<a name="WEAP"></a>
### Weapon

`Weapon = value,weight,damage,critDamage,speed,reach,stagger|...`

| Operation      | Name       | Type        | Description                   |
|----------------|------------|-------------|-------------------------------|
| &lt;empty>/+/- | value      | integer     | Value of the weapon           |
| &lt;empty>/+/- | weight     | real number | Weight of the weapon          |
| &lt;empty>/+/- | damage     | integer     | Damage of the weapon          |
| &lt;empty>/+/- | critDamage | integer     | Critical damage of the weapon |
| &lt;empty>/+/- | speed      | real number | Speed of the weapon           |
| &lt;empty>/+/- | reach      | real number | Reach of the weapon           |
| &lt;empty>/+/- | stagger    | real number | Stagger of the weapon         |

#### String Filters

It supports the following string filters:
  - Keywords it has (e.g. `WeapMaterialIron`)
  - Weapon editorID (e.g. `IronWarhammer`)
  - Weapon name (e.g. `Iron Warhammer`)

#### Form Filters

It supports the following form filters:
  - Leveled list it is in (e.g. `LItemWeaponBattleAxe`)
  - Form list it is in (e.g. `MineOreToolsList`)


<a name="AMMO"></a>
### Ammo

`Ammo = value,weight,damage|...`

| Operation      | Name    | Type        | Description        |
|----------------|---------|-------------|--------------------|
| &lt;empty>/+/- | value   | integer     | Value of the ammo  |
| &lt;empty>/+/- | weight  | real number | Weight of the ammo |
| &lt;empty>/+/- | damage  | real number | Damage of the ammo |

#### String Filters

It supports the following string filters:
  - Keywords it has (e.g. `WeapMaterialOrcish`)
  - Ammo editorID (e.g. `OrcishArrow`)
  - Ammo name (e.g. `Orcish Arrow`)

#### Form Filters

It supports the following form filters:
  - Leveled list it is in (e.g. `LItemAmmo`)
  - Form list it is in (e.g. `Bolts`)

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
