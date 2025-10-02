# Quick Start Guide - Hyprland Vines

## What You Get

🌱 **Living window decorations** that grow throughout the day!

- **Midnight → 5 PM**: Vines progressively grow from top-left, clockwise around windows
- **After 5 PM**: Full coverage with warm orange sunset colors
- **Always**: Gentle swaying animation for organic feel

## Installation (Quick)

```bash
cd /home/s4id/Projects/hyprland-vines
make all
hyprpm add .
hyprpm enable borders-plus-plus
```

## Configuration (Quick)

Add to `~/.config/hypr/hyprland.conf`:

```conf
plugin {
    borders-plus-plus {
        add_borders = 1
        col.border_1 = rgb(4488ff)
        border_size_1 = 10
        
        enable_vines = 1
        vine_thickness = 2
    }
}
```

Then reload:
```bash
hyprctl reload
```

## See It In Action (Quick Test)

```bash
# Run the interactive test (shows growth at different times)
./test-growth.fish
```

## What Happens During the Day

| Time | What You See |
|------|-------------|
| 00:00 (Midnight) | Tiny sprout in top-left corner |
| 06:00 (6 AM) | Top edge mostly covered |
| 12:00 (Noon) | Top, right, and bottom edges growing |
| 15:00 (3 PM) | Nearly complete frame |
| 17:00 (5 PM) | **Full coverage, still green** |
| 18:00 (6 PM) | **Full coverage, NOW ORANGE!** 🍂 |
| 23:59 (11:59 PM) | Orange vines maintained |

## Key Settings

```conf
enable_vines = 1    # 1 = on, 0 = off (just borders)
vine_thickness = 2  # 1-5 recommended (pixel width)
```

## That's It!

The vines automatically:
- ✅ Grow based on your system time
- ✅ Change color at 5 PM
- ✅ Adapt to your border colors
- ✅ Regenerate when windows resize
- ✅ Animate smoothly

Just install, configure, and watch them grow throughout your day!

## More Info

- **Full Guide**: See `VINES_GUIDE.md`
- **Growth Details**: See `GROWTH_TIMELINE.md`
- **Technical Details**: See `TIME_BASED_IMPLEMENTATION.md`
