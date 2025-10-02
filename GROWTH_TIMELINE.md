# Vine Growth Timeline

## 24-Hour Vine Growth Visualization

This document illustrates how vines grow around your windows throughout the day.

```
Time: 00:00 (Midnight)          Progress: 0%
┌─────────────────────┐
│                     │         Just a tiny sprout in top-left
│                     │
│                     │
│                     │
│                     │
└─────────────────────┘


Time: 03:00 (3 AM)              Progress: ~18%
┌──────────────────────┐
│~~~~~~~~~~~~~~~~~~~~  │        Vines crawling across top
│                      │
│                      │
│                      │
│                      │
└──────────────────────┘


Time: 06:00 (6 AM)              Progress: ~35%
┌──────────────────────┐
│~~~~~~~~~~~~~~~~~~~~~~│        Top complete, starting right
│                     ~│
│                     ~│
│                     ~│
│                      │
└──────────────────────┘


Time: 09:00 (9 AM)              Progress: ~53%
┌──────────────────────┐
│~~~~~~~~~~~~~~~~~~~~~~│        Top and right growing
│                     ~│
│                     ~│
│                     ~│
│                     ~│
└──────────────────────┘


Time: 12:00 (Noon)              Progress: ~71%
┌──────────────────────┐
│~~~~~~~~~~~~~~~~~~~~~~│        Top, right, and bottom edges
│                     ~│
│                     ~│
│                     ~│
│                     ~│
└~~~~~~~~~~~~~~~~~~~~~┘


Time: 15:00 (3 PM)              Progress: ~88%
┌──────────────────────┐
│~~~~~~~~~~~~~~~~~~~~~~│        Nearly complete!
│~                    ~│
│~                    ~│
│~                    ~│
│~                    ~│
└~~~~~~~~~~~~~~~~~~~~~~┘


Time: 17:00 (5 PM)              Progress: 100%
┌──────────────────────┐
│~~~~~~~~~~~~~~~~~~~~~~│        FULL COVERAGE
│~                    ~│        Color: GREEN
│~                    ~│
│~                    ~│
│~                    ~│
└~~~~~~~~~~~~~~~~~~~~~~┘


Time: 18:00 (6 PM)              Progress: 100%
┌──────────────────────┐
│○○○○○○○○○○○○○○○○○○○○○○│        FULL COVERAGE
│○                    ○│        Color: ORANGE 🍂
│○                    ○│        Sunset transformation!
│○                    ○│
│○                    ○│
└○○○○○○○○○○○○○○○○○○○○○┘
```

## Growth Pattern Details

### Stage 1: Top Edge (0:00 - 4:15)
- Growth Direction: Left → Right
- Starting Point: Top-left corner
- Progress: 0% → 25%

### Stage 2: Right Edge (4:15 - 8:30)
- Growth Direction: Top → Bottom
- Starting Point: Top-right corner
- Progress: 25% → 50%

### Stage 3: Bottom Edge (8:30 - 12:45)
- Growth Direction: Right → Left
- Starting Point: Bottom-right corner
- Progress: 50% → 75%

### Stage 4: Left Edge (12:45 - 17:00)
- Growth Direction: Bottom → Top
- Starting Point: Bottom-left corner
- Progress: 75% → 100%

### Stage 5: Evening Transformation (17:00+)
- Full coverage maintained
- Color transitions from green to orange
- Sunset ambiance for evening computing

## Color Scheme

**Daytime (00:00 - 16:59)**
- Base: Your configured border color
- Tint: Green overlay
- RGB: Enhanced green channel, reduced red/blue

**Evening (17:00 - 23:59)**
- Base: Your configured border color
- Tint: Warm orange
- RGB: Enhanced red, moderate green, minimal blue

## Technical Notes

- Growth is calculated as: `currentHour / 17.0`
- Updates occur approximately every 10 minutes (1% growth)
- Vines regenerate when growth progress changes by >1%
- Window resizing triggers immediate regeneration
- Animation continues at all growth stages
