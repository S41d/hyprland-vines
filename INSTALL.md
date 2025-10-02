# Installation Guide

## Prerequisites

- Hyprland window manager installed
- hyprpm (Hyprland Plugin Manager)
- Development tools: g++, make, pkg-config

## Installation Steps

### 1. Clone or Copy the Plugin

```bash
cd ~/.config/hypr/plugins
# If you're already in the hyprland-vines directory, skip this
```

### 2. Compile the Plugin

```bash
cd hyprland-vines
make all
```

This should create `borders-plus-plus.so` in the directory.

### 3. Install with hyprpm

```bash
# From the plugin directory
hyprpm add .
hyprpm enable borders-plus-plus
```

Or manually copy the .so file:

```bash
cp borders-plus-plus.so ~/.local/share/hyprland/plugins/
```

### 4. Configure Hyprland

Add to your `~/.config/hypr/hyprland.conf`:

```conf
plugin {
    borders-plus-plus {
        add_borders = 1
        col.border_1 = rgb(4488ff)
        border_size_1 = 10
        
        # Enable vines!
        enable_vines = 1
        vine_thickness = 2
    }
}
```

### 5. Reload Hyprland

```bash
hyprctl reload
# or
hyprpm reload -n
```

## Verifying Installation

Open a terminal and run:
```bash
hyprctl plugin list
```

You should see `borders-plus-plus` in the list.

## Updating

If you make changes to the code:

```bash
make clean
make all
hyprpm reload -n
```

## Uninstallation

```bash
hyprpm remove borders-plus-plus
# or manually delete the .so file
rm ~/.local/share/hyprland/plugins/borders-plus-plus.so
```

Then remove the plugin configuration from your `hyprland.conf` and reload.

## Troubleshooting

### "Plugin failed to load"
- Check Hyprland version matches the plugin compilation
- Ensure all dependencies are installed
- Check `~/.local/share/hyprland/hyprland.log` for errors

### Compilation errors
```bash
# Install required dependencies (Arch Linux example)
sudo pacman -S base-devel pkg-config

# Check pkg-config can find hyprland
pkg-config --cflags hyprland
```

### Changes not appearing
```bash
# Force reload
hyprctl reload
# or restart Hyprland
```
