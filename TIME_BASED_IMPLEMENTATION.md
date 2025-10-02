# Time-Based Vine Growth - Implementation Summary

## Overview
Successfully implemented a dynamic time-based growth system where vines:
1. Start as tiny sprouts at midnight (00:00)
2. Grow progressively throughout the day
3. Reach full coverage by 17:00 (5 PM)
4. Transform to orange sunset colors after 17:00

## Technical Implementation

### New Functions Added

#### `getVineGrowthProgress()`
- Reads system time using `std::chrono`
- Calculates growth as percentage: `currentHour / 17.0`
- Returns 0.0 at midnight, 1.0 at 5 PM and after

#### `getVineColorForTime()`
- Determines vine color based on current hour
- Before 17:00: Green tint (enhanced green channel)
- After 17:00: Orange tint (enhanced red, reduced blue)

### Modified Functions

#### `drawVines()`
- Now checks growth progress every frame
- Regenerates vine paths when progress changes by >1%
- Implements clockwise growth pattern:
  - **0-25%**: Top edge (left to right)
  - **25-50%**: Right edge (top to bottom)
  - **50-75%**: Bottom edge (right to left)
  - **75-100%**: Left edge (bottom to top)
- Each edge scales proportionally within its time window

### New Member Variables

- `m_fLastGrowthProgress` - Tracks last calculated growth to detect changes
- Prevents unnecessary regeneration, updates ~every 10 minutes (1% change)

## Growth Algorithm

### Stage Calculation
```cpp
float growthProgress = getVineGrowthProgress(); // 0.0 to 1.0

// Determine active edges
bool drawTop = growthProgress > 0.0f;      // Start immediately
bool drawRight = growthProgress > 0.25f;   // After 4:15
bool drawBottom = growthProgress > 0.5f;   // After 8:30
bool drawLeft = growthProgress > 0.75f;    // After 12:45
```

### Per-Edge Scaling
Each edge scales within its 25% window:
```cpp
// Example for top edge (0-25% window)
float topProgress = std::min((growthProgress * 4.0f), 1.0f);
```

This creates smooth, continuous growth across all edges.

## Color Transitions

### Green Phase (00:00 - 16:59)
```cpp
vineColor.r = r * 0.5 + 0.2  // Reduce red
vineColor.g = g * 1.2        // Boost green
vineColor.b = b * 0.5        // Reduce blue
```

### Orange Phase (17:00+)
```cpp
vineColor.r = r * 1.5 + 0.3  // Boost red significantly
vineColor.g = g * 0.8 + 0.2  // Moderate green (for orange)
vineColor.b = b * 0.3        // Minimize blue
```

## Performance Considerations

### Efficient Updates
- Vine paths only regenerate when growth changes by >1%
- At 1% increments across 17 hours = ~10 minute intervals
- No unnecessary recalculation between updates

### Time Checking
- Uses `std::chrono::system_clock` for accurate time
- Lightweight calculation (simple division)
- No external dependencies or system calls except time reading

## File Changes

### Modified Files
1. **borderDeco.hpp**
   - Added function declarations for growth and color
   - Added `m_fLastGrowthProgress` member variable

2. **borderDeco.cpp**
   - Implemented `getVineGrowthProgress()`
   - Implemented `getVineColorForTime()`
   - Modified `drawVines()` for staged growth
   - Updated `drawPass()` to use time-based color
   - Added `#include <ctime>` and `#include <chrono>`

### Documentation Updates
1. **README.md** - Added growth timeline overview
2. **VINES_GUIDE.md** - Expanded with growth details and testing tips
3. **GROWTH_TIMELINE.md** - New visual timeline document
4. **test-growth.fish** - New interactive test script

## Testing

### Manual Testing
Users can observe natural growth by:
- Checking windows at different times of day
- Watching the ~10 minute update intervals

### Automated Testing
Included `test-growth.fish` script:
- Temporarily changes system time
- Shows vine states at key hours
- Automatically restores time when done
- Requires sudo privileges

## User Experience

### What Users See

**Morning (6 AM)**
- Top edge mostly covered with green vines
- Right edge just starting

**Midday (Noon)**
- Top edge complete
- Right and bottom edges active
- Still green colored

**Afternoon (3 PM)**
- Nearly complete frame
- Left edge filling in
- Green vines

**Evening (6 PM)**
- Full vine coverage
- **Orange/amber color** for sunset ambiance
- Creates warm evening atmosphere

## Configuration

No new configuration options needed! The time-based behavior is automatic:
- `enable_vines = 1` - Works as before
- `vine_thickness = 2` - Works as before
- Growth and color happen automatically based on system time

## Future Enhancements (Ideas)

Possible future additions:
- Configuration for growth speed (faster/slower than 17 hours)
- Custom sunset time instead of hardcoded 17:00
- Seasonal variations (different growth patterns by season)
- Weather integration (slower growth on "rainy" days)
- Multiple color schemes (not just green→orange)

## Compilation

Successfully compiles with existing build system:
```bash
make clean && make all
```

No new dependencies required beyond what was already used.
