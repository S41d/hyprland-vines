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
  
  // Helper lambda to draw a simple, stylized leaf shape
  auto drawLeaf = [&](Vector2D pos, float size, const CHyprColor& leafColor, float angleOffset) {
    // Simplified heart/teardrop leaf shape - clean and recognizable
    
    // Main leaf body - single rounded rectangle with high rounding for organic shape
    CBox leafBody = {
      pos.x - size * 0.45f,
      pos.y - size * 0.55f,
      static_cast<double>(size * 0.9f),
      static_cast<double>(size * 1.1f)
    };
    // Heavy rounding creates teardrop shape
    g_pHyprOpenGL->renderRect(leafBody, leafColor, {.round = sc<int>(size * 0.45f)});
    
    // Small stem at base for detail
    CBox stem = {
      pos.x - size * 0.1f,
      pos.y + size * 0.4f,
      static_cast<double>(size * 0.2f),
      static_cast<double>(size * 0.3f)
    };
    CHyprColor stemColor = leafColor;
    stemColor.r *= 0.75f;
    stemColor.g *= 0.85f;
    stemColor.b *= 0.75f;
    g_pHyprOpenGL->renderRect(stem, stemColor, {.round = sc<int>(size * 0.1f)});
  };
  
  // Draw the vines
  for (const auto& vinePath : m_vVinePaths) {
    if (vinePath.size() < 2) continue;
    
    // Draw the main vine stem using small overlapping circles for smooth lines
    for (size_t i = 0; i < vinePath.size(); ++i) {
      Vector2D p = vinePath[i];
      
      // Draw smooth stem using circles (not leaves for the main line)
      float stemSize = static_cast<float>(thickness) * 0.8f;
      CBox stemCircle = {
        p.x - stemSize / 2.0f,
        p.y - stemSize / 2.0f,
        static_cast<double>(stemSize),
        static_cast<double>(stemSize)
      };
      
      CHyprColor stemColor = color;
      stemColor.a = a;
      // Darken stem slightly
      stemColor.r *= 0.8f;
      stemColor.g *= 0.9f;
      stemColor.b *= 0.8f;
      
      g_pHyprOpenGL->renderRect(stemCircle, stemColor, {.round = sc<int>(stemSize / 2)});
      
      // For thicker vines, draw additional connecting segments between points
      if (i > 0) {
        Vector2D p1 = vinePath[i - 1];
        Vector2D p2 = vinePath[i];
        
        // Draw interpolated circles between points for smooth stems
        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        int steps = std::max(1, static_cast<int>(distance / (thickness * 0.4f)));
        
        for (int step = 1; step < steps; ++step) {
          float t = static_cast<float>(step) / steps;
          Vector2D interpPoint = {
            p1.x + dx * t,
            p1.y + dy * t
          };
          
          CBox interpCircle = {
            interpPoint.x - stemSize / 2.0f,
            interpPoint.y - stemSize / 2.0f,
            static_cast<double>(stemSize),
            static_cast<double>(stemSize)
          };
          
          g_pHyprOpenGL->renderRect(interpCircle, stemColor, {.round = sc<int>(stemSize / 2)});
        }
      }
    }
    
    // Draw larger decorative leaves at intervals
    for (size_t i = 0; i < vinePath.size(); i += 10) {  // Less frequent, more impactful
      float decorativeLeafSize = thickness * 4.0f;
      
      // Calculate direction for leaf orientation
      Vector2D direction = {1, 0};
      if (i > 0 && i < vinePath.size() - 1) {
        direction.x = vinePath[i + 1].x - vinePath[i - 1].x;
        direction.y = vinePath[i + 1].y - vinePath[i - 1].y;
        float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (len > 0.01f) {
          direction.x /= len;
          direction.y /= len;
        }
      }
      
      // Offset leaf perpendicular to vine direction (alternating sides)
      float side = (i / 10) % 2 == 0 ? 1.0f : -1.0f;
      float perpX = -direction.y * side * decorativeLeafSize * 0.5f;
      float perpY = direction.x * side * decorativeLeafSize * 0.5f;
      
      Vector2D leafPos = {
        vinePath[i].x + perpX,
        vinePath[i].y + perpY
      };
      
      // Make decorative leaves slightly transparent and vibrant
      CHyprColor decorativeColor = color;
      decorativeColor.a = a * 0.8f;
      
      // Add angle variation for natural look
      float angleVariation = std::sin(i * 0.5f + m_fVineAnimationTime * 0.2f) * 0.3f;
      
      drawLeaf(leafPos, decorativeLeafSize, decorativeColor, angleVariation);
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
