# Darkly GTK theme

A GTK theme to go with the [Darkly Qt style](https://github.com/Bali10050/Darkly) by Bali10050

![preview](preview.png?raw=true)

* Supports GTK 3.20+, GTK 4 and libadwaita
* Automatically adapts to the Plasma color scheme
* Customization options from the Darkly Qt style config are applied during installation (work in progress, see `sass/_darkly_default_settings.scss` for the current status)

## Requirements
`sassc` to generate the theme's CSS files

## Installation

Run the installation script

```
./install.sh 
```

Detailed usage of the installation script:

```
Usage: ./install.sh [OPTIONS]...

OPTIONS:
-d      Specify destination directory (Default: $HOME/.local/share/themes
                                                or $XDG_DATA_HOME/themes if $XDG_DATA_HOME is set)
-l      Libadwaita support. Copies theme to ~/.config/gtk-4.0/
                                            or $XDG_CONFIG_DIR/gtk-4.0/ if $XDG_CONFIG_DIR is set
-u      Uninstall the theme
-h      Show help
```

### Flatpak apps

Flatpak apps need permissions to read the user's theme directory.

```
sudo flatpak override --filesystem=xdg-data/themes
```

### Libadwaita Flatpak apps

If you installed the libadwaita theme, you have to enable grant permissions as well

 ```
sudo flatpak override --filesystem=xdg-config/gtk-4.0
 ```

## Disclaimer

Third party themes may break certain apps. If this theme breaks an app, report it here and not to the app developers.
There may be some cases where these bugs can't be fixed from within the theme, in which case we're out of luck unfortunately.


## Credits

Based on the stylesheets from [GTK](https://gitlab.gnome.org/GNOME/gtk/) and [libadwaita](https://gitlab.gnome.org/GNOME/libadwaita)
