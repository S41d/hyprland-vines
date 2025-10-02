#include "borderDeco.hpp"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/Window.hpp>
#include <hyprland/src/render/Renderer.hpp>
#include <hyprutils/memory/Casts.hpp>
using namespace Hyprutils::Memory;
#include "BorderppPassElement.hpp"
#include "globals.hpp"
#include <cmath>
#include <random>
#include <algorithm>
#include <ctime>
#include <chrono>

// Constructor: Initializes the borders-plus-plus decoration for a window
// Stores initial window position and size for tracking changes
CBordersPlusPlus::CBordersPlusPlus(PHLWINDOW pWindow)
    : IHyprWindowDecoration(pWindow), m_pWindow(pWindow) {
  m_lastWindowPos = pWindow->m_realPosition->value();
  m_lastWindowSize = pWindow->m_realSize->value();
}

// Destructor: Cleans up the decoration and damages the entire area for redraw
CBordersPlusPlus::~CBordersPlusPlus() { damageEntire(); }

// Returns positioning information for the decoration
// Calculates the total border thickness and reserves space around the window
SDecorationPositioningInfo CBordersPlusPlus::getPositioningInfo() {
  static auto *const PBORDERS =
      (Hyprlang::INT *const *)HyprlandAPI::getConfigValue(
          PHANDLE, "plugin:borders-plus-plus:add_borders")
          ->getDataStaticPtr();

  static std::vector<Hyprlang::INT *const *> PSIZES;
  for (size_t i = 0; i < 9; ++i) {
    PSIZES.push_back((Hyprlang::INT *const *)HyprlandAPI::getConfigValue(
                         PHANDLE, "plugin:borders-plus-plus:border_size_" +
                                      std::to_string(i + 1))
                         ->getDataStaticPtr());
  }

  SDecorationPositioningInfo info;
  info.policy = DECORATION_POSITION_STICKY;
  info.reserved = true;
  info.priority = 9990;
  info.edges = DECORATION_EDGE_BOTTOM | DECORATION_EDGE_LEFT |
               DECORATION_EDGE_RIGHT | DECORATION_EDGE_TOP;

  if (m_fLastThickness == 0) {
    double size = 0;

    for (size_t i = 0; i < **PBORDERS; ++i) {
      size += **PSIZES[i];
    }

    info.desiredExtents = {{size, size}, {size, size}};
    m_fLastThickness = size;
  } else
    info.desiredExtents = {{m_fLastThickness, m_fLastThickness},
                           {m_fLastThickness, m_fLastThickness}};

  return info;
}

// Handles the positioning reply from the decoration positioner
// Stores the assigned geometry for the decoration
void CBordersPlusPlus::onPositioningReply(
    const SDecorationPositioningReply &reply) {
  m_bAssignedGeometry = reply.assignedGeometry;
}

// Returns decoration flags indicating this decoration is part of the main window
uint64_t CBordersPlusPlus::getDecorationFlags() {
  return DECORATION_PART_OF_MAIN_WINDOW;
}

// Returns the layer on which this decoration should be rendered
// DECORATION_LAYER_OVER means it renders above the window content
eDecorationLayer CBordersPlusPlus::getDecorationLayer() {
  return DECORATION_LAYER_OVER;
}

// Returns the display name of this decoration for identification purposes
std::string CBordersPlusPlus::getDisplayName() { return "Borders++"; }

// Main draw function called by the renderer
// Validates the window and adds a render pass element for actual drawing
void CBordersPlusPlus::draw(PHLMONITOR pMonitor, const float &a) {
  if (!validMapped(m_pWindow))
    return;

  const auto PWINDOW = m_pWindow.lock();

  if (!PWINDOW->m_windowData.decorate.valueOrDefault())
    return;

  CBorderPPPassElement::SBorderPPData data;
  data.deco = this;

  g_pHyprRenderer->m_renderPass.add(makeUnique<CBorderPPPassElement>(data));
}

// Calculates vine growth progress based on current time of day
// Returns 0.0 at midnight, 1.0 at 17:00 (5 PM)
float CBordersPlusPlus::getVineGrowthProgress() {
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  auto localTime = std::localtime(&time);
  
  // Get current hour and minute
  float currentHour = localTime->tm_hour + (localTime->tm_min / 60.0f);
  
  // Growth from 0:00 to 17:00 (17 hours)
  const float TARGET_HOUR = 17.0f;
  
  if (currentHour >= TARGET_HOUR) {
    return 1.0f; // Full growth after 5 PM
  }
  
  // Linear growth from 0 to 1 over 17 hours
  return currentHour / TARGET_HOUR;
}

// Returns vine color based on time of day
// Green during growth hours (0:00-17:00), orange after 17:00
CHyprColor CBordersPlusPlus::getVineColorForTime(const CHyprColor& baseColor) {
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  auto localTime = std::localtime(&time);
  
  float currentHour = localTime->tm_hour + (localTime->tm_min / 60.0f);
  const float SUNSET_HOUR = 17.0f;
  
  CHyprColor vineColor = baseColor;
  
  if (currentHour >= SUNSET_HOUR) {
    // Orange sunset color after 17:00
    vineColor.r = std::min(vineColor.r * 1.5 + 0.3, 1.0);
    vineColor.g = std::min(vineColor.g * 0.8 + 0.2, 1.0);
    vineColor.b = std::min(vineColor.b * 0.3, 1.0);
  } else {
    // Green color during day
    vineColor.r = std::min(vineColor.r * 0.5 + 0.2, 1.0);
    vineColor.g = std::min(vineColor.g * 1.2, 1.0);
    vineColor.b = std::min(vineColor.b * 0.5, 1.0);
  }
  
  return vineColor;
}

// Generates a curved vine path between two points
// Creates natural-looking curves using sine waves and randomization
void CBordersPlusPlus::generateVinePath(std::vector<Vector2D>& points, Vector2D start, Vector2D end, int segments, float curviness) {
  points.clear();
  
  static std::mt19937 rng(std::random_device{}());
  std::uniform_real_distribution<float> dist(-curviness, curviness);
  
  Vector2D direction = end - start;
  float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
  Vector2D normalized = {direction.x / length, direction.y / length};
  Vector2D perpendicular = {-normalized.y, normalized.x};
  
  for (int i = 0; i <= segments; ++i) {
    float t = static_cast<float>(i) / segments;
    
    // Base position along the line
    Vector2D basePos = start + direction * t;
    
    // Add sinusoidal wave for organic look
    float wave = std::sin(t * M_PI * 3.0f + m_fVineAnimationTime * 0.5f) * curviness * 2.0f;
    
    // Add some randomness for natural variation
    float randomOffset = dist(rng) * 0.3f;
    
    // Apply perpendicular offset
    Vector2D offset = perpendicular * (wave + randomOffset);
    
    points.push_back(basePos + offset);
  }
}

// Draws decorative vines around the window
// Creates multiple vine strands with leaves and tendrils
// Vines grow from top-left based on time of day
void CBordersPlusPlus::drawVines(PHLMONITOR pMonitor, const CBox& box, const float& a, const CHyprColor& color, int thickness) {
  const int numVines = 3; // Number of vine strands per side
  const int segments = 40; // Smoothness of the vine curves (increased for smoother appearance)
  const float curviness = thickness * 0.5f;
  
  // Get current growth progress
  float growthProgress = getVineGrowthProgress();
  
  // Regenerate vines if growth progress changed significantly (every ~1% or 10 minutes)
  if (!m_bVinePathsGenerated || m_vVinePaths.empty() || 
      std::abs(growthProgress - m_fLastGrowthProgress) > 0.01f) {
    m_vVinePaths.clear();
    m_fLastGrowthProgress = growthProgress;
    
    // Calculate which edges to populate based on growth
    // Growth pattern: top-left corner expands clockwise
    // 0% = just top-left corner
    // 25% = top edge complete
    // 50% = top + right edge
    // 75% = top + right + bottom
    // 100% = all edges
    
    bool drawTop = growthProgress > 0.0f;
    bool drawRight = growthProgress > 0.25f;
    bool drawBottom = growthProgress > 0.5f;
    bool drawLeft = growthProgress > 0.75f;
    
    // Top vines - scale from left to right
    if (drawTop) {
      float topProgress = std::min((growthProgress * 4.0f), 1.0f);
      for (int i = 0; i < numVines; ++i) {
        std::vector<Vector2D> vinePath;
        float vineEnd = box.width * topProgress * (i + 1) / numVines;
        Vector2D start = {box.x + (box.width * i) / numVines, box.y};
        Vector2D end = {box.x + vineEnd, box.y};
        if (vineEnd > start.x) {
          generateVinePath(vinePath, start, end, segments, curviness);
          m_vVinePaths.push_back(vinePath);
        }
      }
    }
    
    // Right vines - scale from top to bottom
    if (drawRight) {
      float rightProgress = std::min((growthProgress - 0.25f) * 4.0f, 1.0f);
      for (int i = 0; i < numVines; ++i) {
        std::vector<Vector2D> vinePath;
        float vineEnd = box.height * rightProgress * (i + 1) / numVines;
        Vector2D start = {box.x + box.width, box.y + (box.height * i) / numVines};
        Vector2D end = {box.x + box.width, box.y + vineEnd};
        if (vineEnd > start.y - box.y) {
          generateVinePath(vinePath, start, end, segments, curviness);
          m_vVinePaths.push_back(vinePath);
        }
      }
    }
    
    // Bottom vines - scale from right to left
    if (drawBottom) {
      float bottomProgress = std::min((growthProgress - 0.5f) * 4.0f, 1.0f);
      for (int i = 0; i < numVines; ++i) {
        std::vector<Vector2D> vinePath;
        float vineStart = box.width * (1.0f - bottomProgress * (i + 1) / numVines);
        Vector2D start = {box.x + box.width - (box.width * i) / numVines, box.y + box.height};
        Vector2D end = {box.x + vineStart, box.y + box.height};
        if (start.x > end.x) {
          generateVinePath(vinePath, start, end, segments, curviness);
          m_vVinePaths.push_back(vinePath);
        }
      }
    }
    
    // Left vines - scale from bottom to top
    if (drawLeft) {
      float leftProgress = std::min((growthProgress - 0.75f) * 4.0f, 1.0f);
      for (int i = 0; i < numVines; ++i) {
        std::vector<Vector2D> vinePath;
        float vineStart = box.height * (1.0f - leftProgress * (i + 1) / numVines);
        Vector2D start = {box.x, box.y + box.height - (box.height * i) / numVines};
        Vector2D end = {box.x, box.y + vineStart};
        if (start.y > end.y) {
          generateVinePath(vinePath, start, end, segments, curviness);
          m_vVinePaths.push_back(vinePath);
        }
      }
    }
    
    m_bVinePathsGenerated = true;
  }
  
  // Update animation time
  m_fVineAnimationTime += 0.016f; // Assuming ~60fps
  
  // Draw the vines
  for (const auto& vinePath : m_vVinePaths) {
    if (vinePath.size() < 2) continue;
    
    // Draw the main vine stem with smooth connections
    for (size_t i = 0; i < vinePath.size() - 1; ++i) {
      Vector2D p1 = vinePath[i];
      Vector2D p2 = vinePath[i + 1];
      
      // Calculate center point and dimensions
      float centerX = (p1.x + p2.x) / 2.0f;
      float centerY = (p1.y + p2.y) / 2.0f;
      float dx = p2.x - p1.x;
      float dy = p2.y - p1.y;
      float segmentLength = std::sqrt(dx * dx + dy * dy);
      
      // Create overlapping rounded rectangles for smooth appearance
      CBox lineBox = {
        centerX - segmentLength / 2.0f - thickness / 2.0f,
        centerY - thickness / 2.0f,
        segmentLength + static_cast<double>(thickness),
        static_cast<double>(thickness)
      };
      
      if (segmentLength > 0.1f) {
        CHyprColor lineColor = color;
        lineColor.a = a;
        // Use rounded rectangles for smoother appearance
        g_pHyprOpenGL->renderRect(lineBox, lineColor, {.round = thickness / 2});
      }
      
      // Draw circular joints at each point for smooth connections
      CBox jointBox = {
        p1.x - thickness / 2.0f,
        p1.y - thickness / 2.0f,
        static_cast<double>(thickness),
        static_cast<double>(thickness)
      };
      CHyprColor jointColor = color;
      jointColor.a = a;
      g_pHyprOpenGL->renderRect(jointBox, jointColor, {.round = thickness / 2});
    }
    
    // Draw leaves at intervals
    for (size_t i = 0; i < vinePath.size(); i += 5) {  // Reduced frequency for less clutter
      float leafSize = thickness * 2.0f;
      CBox leafBox = {
        vinePath[i].x - leafSize / 2.0f,
        vinePath[i].y - leafSize / 2.0f,
        leafSize,
        leafSize
      };
      
      // Make leaves slightly transparent with rounded corners
      CHyprColor leafColor = color;
      leafColor.a = a * 0.6f;
      // Rounded leaves look more organic
      g_pHyprOpenGL->renderRect(leafBox, leafColor, {.round = sc<int>(leafSize / 3.0f)});
    }
  }
}

// Performs the actual rendering of the borders
// Draws multiple border layers based on configuration, handling colors, sizes, and rounding
void CBordersPlusPlus::drawPass(PHLMONITOR pMonitor, const float &a) {
  const auto PWINDOW = m_pWindow.lock();

  static std::vector<Hyprlang::INT *const *> PCOLORS;
  static std::vector<Hyprlang::INT *const *> PSIZES;
  for (size_t i = 0; i < 9; ++i) {
    PCOLORS.push_back((Hyprlang::INT *const *)HyprlandAPI::getConfigValue(
                          PHANDLE, "plugin:borders-plus-plus:col.border_" +
                                       std::to_string(i + 1))
                          ->getDataStaticPtr());
    PSIZES.push_back((Hyprlang::INT *const *)HyprlandAPI::getConfigValue(
                         PHANDLE, "plugin:borders-plus-plus:border_size_" +
                                      std::to_string(i + 1))
                         ->getDataStaticPtr());
  }
  static auto *const PBORDERS =
      (Hyprlang::INT *const *)HyprlandAPI::getConfigValue(
          PHANDLE, "plugin:borders-plus-plus:add_borders")
          ->getDataStaticPtr();
  static auto *const PNATURALROUND =
      (Hyprlang::INT *const *)HyprlandAPI::getConfigValue(
          PHANDLE, "plugin:borders-plus-plus:natural_rounding")
          ->getDataStaticPtr();
  static auto *const PROUNDING =
      (Hyprlang::INT *const *)HyprlandAPI::getConfigValue(PHANDLE,
                                                          "decoration:rounding")
          ->getDataStaticPtr();
  static auto *const PBORDERSIZE =
      (Hyprlang::INT *const *)HyprlandAPI::getConfigValue(PHANDLE,
                                                          "general:border_size")
          ->getDataStaticPtr();
  static auto *const PVINETOGGLE =
      (Hyprlang::INT *const *)HyprlandAPI::getConfigValue(
          PHANDLE, "plugin:borders-plus-plus:enable_vines")
          ->getDataStaticPtr();
  static auto *const PVINETHICKNESS =
      (Hyprlang::INT *const *)HyprlandAPI::getConfigValue(
          PHANDLE, "plugin:borders-plus-plus:vine_thickness")
          ->getDataStaticPtr();

  if (**PBORDERS < 1)
    return;

  if (m_bAssignedGeometry.width < m_seExtents.topLeft.x + 1 ||
      m_bAssignedGeometry.height < m_seExtents.topLeft.y + 1)
    return;

  const auto PWORKSPACE = PWINDOW->m_workspace;
  const auto WORKSPACEOFFSET = PWORKSPACE && !PWINDOW->m_pinned
                                   ? PWORKSPACE->m_renderOffset->value()
                                   : Vector2D();

  auto rounding =
      PWINDOW->rounding() == 0
          ? 0
          : (PWINDOW->rounding() + **PBORDERSIZE) * pMonitor->m_scale;
  const auto ROUNDINGPOWER = PWINDOW->roundingPower();
  const auto ORIGINALROUND =
      rounding == 0 ? 0
                    : (PWINDOW->rounding() + **PBORDERSIZE) * pMonitor->m_scale;

  CBox fullBox = m_bAssignedGeometry;
  fullBox.translate(g_pDecorationPositioner->getEdgeDefinedPoint(
      DECORATION_EDGE_BOTTOM | DECORATION_EDGE_LEFT | DECORATION_EDGE_RIGHT |
          DECORATION_EDGE_TOP,
      m_pWindow.lock()));

  fullBox.translate(PWINDOW->m_floatingOffset - pMonitor->m_position +
                    WORKSPACEOFFSET);

  if (fullBox.width < 1 || fullBox.height < 1)
    return;

  double fullThickness = 0;

  for (size_t i = 0; i < **PBORDERS; ++i) {
    const int THISBORDERSIZE =
        **(PSIZES[i]) == -1 ? **PBORDERSIZE : (**PSIZES[i]);
    fullThickness += THISBORDERSIZE;
  }

  fullBox.expand(-fullThickness).scale(pMonitor->m_scale).round();

  for (size_t i = 0; i < **PBORDERS; ++i) {
    const int PREVBORDERSIZESCALED =
        i == 0 ? 0
               : (**PSIZES[i - 1] == -1 ? **PBORDERSIZE : **(PSIZES[i - 1])) *
                     pMonitor->m_scale;
    const int THISBORDERSIZE =
        **(PSIZES[i]) == -1 ? **PBORDERSIZE : (**PSIZES[i]);

    if (i != 0) {
      rounding += rounding == 0 ? 0 : PREVBORDERSIZESCALED;
      fullBox.x -= PREVBORDERSIZESCALED;
      fullBox.y -= PREVBORDERSIZESCALED;
      fullBox.width += PREVBORDERSIZESCALED * 2;
      fullBox.height += PREVBORDERSIZESCALED * 2;
    }

    if (fullBox.width < 1 || fullBox.height < 1)
      break;

    g_pHyprOpenGL->scissor(nullptr);

    g_pHyprOpenGL->renderBorder(
        fullBox, CHyprColor{(uint64_t)**PCOLORS[i]},
        {.round = **PNATURALROUND ? sc<int>(ORIGINALROUND) : sc<int>(rounding),
         .roundingPower = ROUNDINGPOWER,
         .borderSize = THISBORDERSIZE,
         .a = a,
         .outerRound = **PNATURALROUND ? sc<int>(ORIGINALROUND) : -1});
  }

  // Draw vines on top of borders if enabled
  if (**PVINETOGGLE && **PBORDERS > 0) {
    int vineThickness = **PVINETHICKNESS > 0 ? **PVINETHICKNESS : 2;
    CHyprColor baseVineColor = CHyprColor{(uint64_t)**PCOLORS[0]}; // Use first border color
    
    // Get time-appropriate color (green during day, orange after 17:00)
    CHyprColor vineColor = getVineColorForTime(baseVineColor);
    
    drawVines(pMonitor, fullBox, a, vineColor, vineThickness);
  }

  m_seExtents = {{fullThickness, fullThickness},
                 {fullThickness, fullThickness}};

  m_bLastRelativeBox =
      CBox{0, 0, m_lastWindowSize.x, m_lastWindowSize.y}.addExtents(
          m_seExtents);

  if (fullThickness != m_fLastThickness) {
    m_fLastThickness = fullThickness;
    g_pDecorationPositioner->repositionDeco(this);
  }
}

// Returns the type of this decoration (custom type)
eDecorationType CBordersPlusPlus::getDecorationType() {
  return DECORATION_CUSTOM;
}

// Updates the decoration when the window changes
// Stores new window position and size, then damages the area for redraw
void CBordersPlusPlus::updateWindow(PHLWINDOW pWindow) {
  m_lastWindowPos = pWindow->m_realPosition->value();
  m_lastWindowSize = pWindow->m_realSize->value();

  // Reset vine paths when window size changes
  m_bVinePathsGenerated = false;

  damageEntire();
}

// Marks the entire decoration area as damaged for redraw
// Calculates the bounding box including borders and requests a redraw from the renderer
void CBordersPlusPlus::damageEntire() {
  CBox dm = m_bLastRelativeBox.copy().translate(m_lastWindowPos).expand(2);
  g_pHyprRenderer->damageBox(dm);
}
