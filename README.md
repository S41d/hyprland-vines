# hyprland-vines

A Hyprland plugin that adds decorative vines around your windows with **time-based growth**. Vines emerge from the top-left at midnight, progressively grow throughout the day, achieve full coverage by 5 PM, and transform to warm orange tones in the evening.

## Features

- **Time-Based Growth**: Vines start small at midnight and expand throughout the day
- **Growth Timeline**: Reach full coverage by 17:00 (5 PM)
- **Sunset Colors**: Vines turn orange after 5 PM for evening ambiance
- **Animated Motion**: Subtle animations make vines feel alive
- **Customizable**: Control vine thickness and toggle them on/off
- **Multiple Borders**: Still supports the original 1-9 borders feature
- **Natural Rounding**: Vines and borders follow window corner rounding

## Growth Cycle

- **00:00 - 04:15**: Top edge grows from left to right
- **04:15 - 08:30**: Right edge grows from top to bottom  
- **08:30 - 12:45**: Bottom edge grows from right to left
- **12:45 - 17:00**: Left edge grows from bottom to top
- **17:00+**: Full coverage with orange sunset colors

## Example Config:
```
plugin {
    borders-plus-plus {
        add_borders = 1 # 0 - 9

        # you can add up to 9 borders
        col.border_1 = rgb(ffffff)
        col.border_2 = rgb(2222ff)

        # -1 means "default" as in the one defined in general:border_size
        border_size_1 = 10
        border_size_2 = -1

        # makes outer edges match rounding of the parent. Turn on / off to better understand. Default = on.
        natural_rounding = yes

        # Enable decorative vines around windows (1 = on, 0 = off)
        enable_vines = 1

        # Thickness of the vine stems (in pixels)
        vine_thickness = 2
    }
}
```

## Vine Options

- `enable_vines`: Toggle vine decorations (0 or 1, default: 1)
- `vine_thickness`: Control the thickness of vine stems in pixels (default: 2)

Vines automatically:
- Inherit and adapt the color from your first border (`col.border_1`)
- Tint green during daytime (00:00 - 17:00)
- Transform to orange after 17:00 for evening ambiance
- Grow progressively based on your system time
- Regenerate every ~10 minutes to reflect time progression