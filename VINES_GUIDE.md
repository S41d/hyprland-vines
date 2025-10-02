# Hyprland Vines - User Guide

## Overview

This plugin transforms your Hyprland window borders into beautiful, organic vines that **grow throughout the day**. Starting from the top-left corner at midnight, vines gradually expand around your windows, reaching full coverage by evening, then turning a warm orange color after 5 PM.

## Features

### 🌱 Time-Based Growth
- **Midnight (00:00)**: Vines start as a small sprout in the top-left corner
- **Morning-Afternoon**: Vines progressively grow clockwise around windows
- **5 PM (17:00)**: Full vine coverage achieved
- **Evening (after 17:00)**: Vines turn orange for sunset ambiance
- Growth is smooth and automatic based on your system time

### 🌿 Organic Design
- Curved vine paths that follow your window borders
- Natural-looking sine wave patterns
- Random variations for authentic appearance

### 🎬 Subtle Animation
- Vines gently sway and move
- Smooth, non-distracting motion
- Creates a "living" aesthetic

### 🎨 Customizable
- Control vine thickness
- Toggle vines on/off without reloading
- Vines adapt to your border colors

### 🍃 Decorative Elements
- Leaves appear at intervals along vines
- Semi-transparent for layered effect
- Color changes from green to orange throughout the day

## Configuration

Add this to your `~/.config/hypr/hyprland.conf`:

```conf
plugin {
    borders-plus-plus {
        # Standard border options
        add_borders = 1
        col.border_1 = rgb(4488ff)  # Vines will be greenish version of this
        border_size_1 = 10
        natural_rounding = yes
        
        # Vine-specific options
        enable_vines = 1        # 1 = enabled, 0 = disabled
        vine_thickness = 2      # Thickness in pixels (1-5 recommended)
    }
}
```

## Configuration Options

### `enable_vines` (default: 1)
- **1**: Vines are rendered around windows
- **0**: Only standard borders are shown

### `vine_thickness` (default: 2)
- Controls the width of vine stems in pixels
- Recommended range: 1-5
- Larger values create bolder, more visible vines
- Smaller values create delicate, subtle decoration

## How It Works

### Growth Timeline
The vines follow a natural 24-hour cycle:

**00:00 - 04:15 (0-25%)** - Top edge growth
- Vines emerge from top-left corner
- Gradually extend across the top of the window

**04:15 - 08:30 (25-50%)** - Right edge growth  
- Top edge complete
- Vines cascade down the right side

**08:30 - 12:45 (50-75%)** - Bottom edge growth
- Top and right complete
- Vines spread across the bottom

**12:45 - 17:00 (75-100%)** - Left edge growth
- Bottom edge fills in
- Vines climb up the left side to complete the frame

**17:00+** - Sunset transformation
- Full coverage maintained
- Vines transition from green to warm orange

### Technical Process
1. **Time Check**: Plugin reads system time every frame
2. **Growth Calculation**: Converts time to growth percentage (0-100%)
3. **Path Generation**: Creates vine paths for visible segments only
4. **Color Adaptation**: Green before 5 PM, orange after
5. **Animation**: Subtle sway animation continues regardless of growth
6. **Automatic Updates**: Vines regenerate every ~10 minutes to reflect time changes

## Visual Tips

### Watch Them Grow
The best way to experience the vines is to observe them at different times:
- **Morning (6 AM)**: See the top edge mostly complete
- **Noon**: Watch vines reaching down the right side
- **Afternoon (3 PM)**: Nearly full coverage
- **Evening (6 PM)**: Full orange glow

### For Testing Growth
If you want to see the full cycle quickly, you can use the included test script:
```bash
./test-growth.fish
```

This will walk you through different times of day, showing how vines appear at each stage. The script temporarily changes system time (requires sudo) and restores it when done.

Or manually test with different times (requires root):
```bash
# View current state at different times
sudo date -s "06:00:00"  # Morning state
sudo date -s "12:00:00"  # Midday state  
sudo date -s "18:00:00"  # Evening orange

# Don't forget to reset!
sudo timedatectl set-ntp true
sudo systemctl restart systemd-timesyncd
```

### For a Subtle Look
```conf
enable_vines = 1
vine_thickness = 1
col.border_1 = rgb(888888)  # Light gray base
```

### For a Bold Look
```conf
enable_vines = 1
vine_thickness = 4
col.border_1 = rgb(00ff00)  # Green base (will tint automatically)
```

### For Traditional Borders Only
```conf
enable_vines = 0
```

## Technical Details

- **Performance**: Vines are efficiently rendered using OpenGL
- **Per-Window**: Each window has its own unique vine pattern
- **Regeneration**: Vine paths regenerate when windows are resized
- **Compatibility**: Works with all Hyprland window rounding settings

## Troubleshooting

### Vines not appearing?
1. Check that `enable_vines = 1` in your config
2. Ensure you have at least one border: `add_borders = 1`
3. Reload your Hyprland config: `hyprctl reload`

### Vines too thin/thick?
Adjust the `vine_thickness` value between 1-5

### Want different colors?
The vine color is derived from `col.border_1`. Change your border color to change the vine tint.

## Credits

Based on the borders-plus-plus plugin by Vaxry
Vine decoration system adds natural, organic aesthetics to your workspace
