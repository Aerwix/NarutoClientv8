---
name: otui-interface-design
description: Create premium, production-grade OTUI interfaces for Open Tibia clients. Use this skill when the user asks to build game UI components, windows, panels, or interfaces. Generates polished, professional OTUI/Lua code with attention to visual hierarchy, user experience, and game-specific patterns.
license: Complete terms in LICENSE.txt
---

This skill guides creation of distinctive, production-grade OTUI (Open Tibia User Interface) interfaces. Implement real working OTUI and Lua code with exceptional attention to visual design, usability, and game-specific patterns.

The user provides interface requirements: a window, panel, mini-window, dialog, or game UI component to build. They may include context about purpose, target users, or technical constraints.

## Design Thinking

Before coding, understand the context and commit to a clear design direction:
- **Purpose**: What game function does this interface serve? Who uses it and when?
- **Context**: Is this a quick-access panel, a configuration window, a data display, or an interactive tool?
- **Visual Hierarchy**: What information is most critical? What actions are primary vs secondary?
- **Integration**: How does this fit with existing game UI? What layout (retro/modern/mobile) should it support?
- **Differentiation**: What makes this interface efficient and pleasant to use?

**CRITICAL**: OTUI design requires balancing visual appeal with game usability. Interfaces must be functional during gameplay, readable at a glance, and consistent with the game's visual language.

Then implement working code (OTUI + Lua) that is:
- Production-grade and fully functional
- Visually refined with clear hierarchy
- Consistent with Open Tibia UI patterns
- Optimized for game context (performance, readability, accessibility)

---

## OTUI System Architecture

### File Organization

```
/data/styles/           # Core style definitions (numbered for load order)
  0-vars.otui          # Global variables, colors, fonts
  10-*.otui            # Basic components (buttons, labels, panels)
  20-*.otui            # Advanced components (tabs, tables, scrollbars)
  30-*.otui            # Complex widgets (windows, mini-windows, dialogs)
  40-*.otui            # Game-specific interfaces

/layouts/              # Alternative style sets
  /retro/styles/       # Classic client appearance
  /mobile/styles/      # Mobile-optimized layouts

/modules/              # Feature modules with UI
  /module_name/
    module_name.otmod  # Module metadata
    module_name.lua    # Main logic
    *.otui             # UI definitions
```

### OTUI Syntax Fundamentals

```otui
// Widget inheritance
WidgetName < ParentWidget
  property: value

  // Nested children
  ChildWidget < ChildType
    property: value

  // State selectors
  $hover:
    property: changed-value

  $disabled:
    property: disabled-value
```

### Core Widget Types

| Widget | Use Case | Key Properties |
|--------|----------|----------------|
| `UIWidget` | Base container | anchors, size, margin, padding |
| `UILabel` | Text display | text, font, color, text-align |
| `UIButton` | Clickable actions | image-source, image-clip, @onClick |
| `UICheckBox` | Toggle options | $checked, @onCheckChange |
| `UITextEdit` | Text input | text-hidden, max-length, editable |
| `UIComboBox` | Dropdown selection | menu-height, @onOptionChange |
| `UISpinBox` | Numeric input | minimum, maximum, step |
| `UIScrollArea` | Scrollable content | vertical-scrollbar, layout |
| `UIProgressBar` | Progress display | percent, background |
| `UIWindow` | Draggable dialog | title, closeable, @onClose |
| `UIMiniWindow` | Collapsible panel | minimizedHeight, @onMinimize |
| `UITabBar` | Tab navigation | tab-spacing, movable |
| `UITable` | Data grid | row-height, column-width |
| `Item` | Game item display | &position, virtual |
| `Creature` | Creature display | creature reference |

---

## OTUI Design Guidelines

### 1. Visual Hierarchy & Typography

**Font Selection**:
```otui
// Available fonts (use consistently)
font: verdana-11px-antialised    // Standard body text
font: verdana-11px-rounded       // Numeric values, stats
font: verdana-11px-monochrome    // High contrast needs
font: verdana-14px-bold          // Headers, titles (if available)
```

**Text Styling**:
```otui
Label
  font: verdana-11px-antialised
  color: #dfdfdf                   // Standard text
  text-auto-resize: true           // Fit content
  text-align: center               // Alignment options: left, center, right, top, bottom
  text-wrap: true                  // Multi-line text
  text-offset: 0 1                 // Fine-tune positioning
```

**Hierarchy Example**:
```otui
// Title - prominent, high contrast
TitleLabel < Label
  font: verdana-11px-antialised
  color: #ffffff
  text-auto-resize: true

// Subtitle - secondary emphasis
SubtitleLabel < Label
  font: verdana-11px-antialised
  color: #aaaaaa
  margin-top: 2

// Body - standard readability
BodyLabel < Label
  font: verdana-11px-antialised
  color: #dfdfdf

// Muted - tertiary information
MutedLabel < Label
  font: verdana-11px-rounded
  color: #888888
```

### 2. Color System

**Define variables for consistency**:
```otui
// In module or at top of file
$var-text-primary: #ffffff
$var-text-secondary: #dfdfdf
$var-text-muted: #888888
$var-text-disabled: #dfdfdf88
$var-accent: #80c7f8            // Blue accent
$var-success: #44ff44           // Green
$var-warning: #ffff00           // Yellow
$var-danger: #ff4444            // Red
$var-bg-dark: #1a1a1a
$var-bg-medium: #2a2a2a
$var-border: #404040
```

**Color Application**:
```otui
// Primary actions - full opacity, accent color
PrimaryButton < Button
  color: #ffffff

  $hover:
    color: #80c7f8

// Secondary actions - muted
SecondaryButton < Button
  color: #aaaaaa

  $hover:
    color: #dfdfdf

// Disabled state - reduced opacity
$disabled:
  color: #dfdfdf88
  opacity: 0.8
  change-cursor-image: false
```

### 3. Layout & Positioning

**Anchor System (Primary Method)**:
```otui
// Fill parent
anchors.fill: parent

// Center in parent
anchors.centerIn: parent

// Dock to edges
anchors.top: parent.top
anchors.bottom: parent.bottom
anchors.left: parent.left
anchors.right: parent.right

// Relative to siblings
anchors.top: prev.bottom
anchors.left: prev.right

// Reference by ID
anchors.top: headerPanel.bottom
anchors.right: scrollBar.left
```

**Layout Engines**:
```otui
// Vertical stack
Panel
  layout:
    type: verticalBox
    spacing: 4
    fit-children: true
    align-bottom: true         // Anchor to bottom

// Horizontal row
Panel
  layout:
    type: horizontalBox
    spacing: 8
    fit-children: true
    align-right: true          // Anchor to right

// Grid layout
Panel
  layout:
    type: grid
    cell-size: 34 34
    flow: true
```

**Spacing Scale** (Use consistently):
```
4px  - Tight spacing (within components)
8px  - Standard spacing (between related items)
12px - Medium spacing (between groups)
16px - Large spacing (between sections)
```

### 4. State Management

**Common States**:
```otui
Button
  image-clip: 0 0 22 23           // Normal state
  color: #dfdfdf

  $hover !disabled:
    image-clip: 0 23 22 23        // Hover state
    color: #ffffff

  $pressed:
    image-clip: 0 46 22 23        // Pressed state
    text-offset: 1 1              // Visual feedback

  $disabled:
    color: #dfdfdf88
    opacity: 0.8
    change-cursor-image: false

CheckBox
  $!checked:
    image-clip: 0 0 15 15

  $checked:
    image-clip: 0 30 15 15

  $hover !checked:
    image-clip: 0 15 15 15

  $hover checked:
    image-clip: 0 45 15 15
```

**Toggle States**:
```otui
Panel
  $on:
    visible: true
    height: 100

  $!on:
    visible: false
    height: 0
    margin-top: -100           // Collapse completely
```

### 5. Image & Icon Integration

**Image Properties**:
```otui
Widget
  image-source: /images/ui/panel      // Path from root
  image-border: 4                      // 9-slice scaling
  image-border-top: 23                 // Per-side borders
  image-border-bottom: 4
  image-clip: 0 0 22 23               // Sprite sheet clipping (x y w h)
  image-color: #ffffff                 // Tint color
  image-repeated: true                 // Tile mode
```

**Icon Buttons**:
```otui
IconButton < UIButton
  size: 20 20
  image-source: /images/ui/icon_add
  image-color: #ffffff

  $hover:
    image-color: #ffffff99

  $pressed:
    image-color: #ffffff44

  $disabled:
    image-color: #ffffff55
```

**Tab Icons**:
```otui
TabButton
  icon: /images/topbuttons/battle
  icon-color: #dfdfdf
  icon-offset-y: 2
```

### 6. Windows & Containers

**Standard Window**:
```otui
CustomWindow < MainWindow
  id: customWindow
  !text: tr('Window Title')
  size: 300 400
  @onEnter: Module.onConfirm()
  @onEscape: Module.onCancel()

  // Content area
  Panel
    id: content
    anchors.fill: parent
    anchors.bottom: footer.top
    margin: 16
    layout:
      type: verticalBox
      spacing: 8

  // Footer with actions
  Panel
    id: footer
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    height: 50
    layout:
      type: horizontalBox
      align-right: true
      spacing: 8
    padding: 8

    Button
      id: cancelButton
      !text: tr('Cancel')
      width: 80
      @onClick: Module.onCancel()

    Button
      id: confirmButton
      !text: tr('Confirm')
      width: 80
      @onClick: Module.onConfirm()
```

**Mini Window (Collapsible Panel)**:
```otui
CustomMiniWindow < MiniWindow
  id: customMiniWindow
  !text: tr('Panel Title')
  icon: /images/topbuttons/icon
  height: 200
  &save: true                          // Persist state
  &autoOpen: 1                         // Auto-open on load

  MiniWindowContents
    padding: 8
    layout:
      type: verticalBox
      spacing: 4

    // Panel content here
```

### 7. Data Display Patterns

**Scrollable List**:
```otui
ScrollablePanel
  id: listContainer
  anchors.fill: parent
  vertical-scrollbar: listScrollBar
  layout:
    type: verticalBox
    spacing: 2
  padding: 2
  background: #00000066
  border-width: 1
  border-color: #272727

VerticalScrollBar
  id: listScrollBar
  anchors.top: parent.top
  anchors.bottom: parent.bottom
  anchors.right: parent.right
  step: 14
  pixels-scroll: true
```

**Key-Value Display**:
```otui
StatRow < Panel
  height: 18
  layout:
    type: horizontalBox

  Label
    id: label
    anchors.left: parent.left
    anchors.verticalCenter: parent.verticalCenter
    text-auto-resize: true
    color: #aaaaaa

  Label
    id: value
    anchors.right: parent.right
    anchors.verticalCenter: parent.verticalCenter
    text-auto-resize: true
    text-align: right
    color: #ffffff
```

**Progress/Health Bars**:
```otui
HealthBar < ProgressBar
  id: healthBar
  height: 12
  anchors.left: parent.left
  anchors.right: parent.right
  background-color: #00731e          // Green background
  border-width: 1
  border-color: #1a1a1a

ManaBar < ProgressBar
  id: manaBar
  height: 12
  background-color: #0044aa          // Blue background
```

### 8. Form Elements

**Input Field with Label**:
```otui
InputGroup < Panel
  height: 44
  layout:
    type: verticalBox
    spacing: 2

  Label
    id: label
    text-auto-resize: true
    color: #aaaaaa

  TextEdit
    id: input
    height: 22
    anchors.left: parent.left
    anchors.right: parent.right
```

**Checkbox Row**:
```otui
CheckboxRow < Panel
  height: 18

  CheckBox
    id: checkbox
    anchors.left: parent.left
    anchors.verticalCenter: parent.verticalCenter
    @onCheckChange: self:getParent():onCheckChange()

  Label
    id: label
    anchors.left: prev.right
    anchors.verticalCenter: parent.verticalCenter
    margin-left: 6
    text-auto-resize: true
    color: #dfdfdf
```

**Dropdown with Label**:
```otui
SelectGroup < Panel
  height: 44

  Label
    id: label
    anchors.top: parent.top
    anchors.left: parent.left
    text-auto-resize: true
    color: #aaaaaa

  ComboBox
    id: select
    anchors.top: prev.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    margin-top: 2
    menu-scroll: true
    menu-height: 125
    menu-scroll-step: 25
```

### 9. Event Binding & Lua Integration

**Event Handlers in OTUI**:
```otui
Widget
  @onClick: Module.handleClick()
  @onCheckChange: Module.handleToggle(self)
  @onOptionChange: Module.handleSelect()
  @onEnter: Module.handleSubmit()
  @onEscape: Module.handleCancel()
  @onSetup: |
    -- Inline Lua for initialization
    self:setText('Initialized')
```

**Lua Module Pattern**:
```lua
-- module_name.lua
ModuleName = {}

local window = nil

function ModuleName.init()
  -- Load UI
  window = g_ui.displayUI('module_name')

  -- Setup event handlers
  local button = window:getChildById('actionButton')
  button.onClick = ModuleName.handleAction
end

function ModuleName.terminate()
  if window then
    window:destroy()
    window = nil
  end
end

function ModuleName.handleAction()
  -- Handle button click
  local input = window:getChildById('inputField')
  local value = input:getText()

  -- Process value
  ModuleName.processData(value)
end

function ModuleName.updateUI(data)
  local label = window:getChildById('statusLabel')
  label:setText(data.status)
  label:setColor(data.success and '#44ff44' or '#ff4444')
end
```

**Dynamic List Population**:
```lua
function ModuleName.populateList(items)
  local container = window:getChildById('listContainer')
  container:destroyChildren()

  for _, item in ipairs(items) do
    local widget = g_ui.createWidget('ListItem', container)
    widget:getChildById('name'):setText(item.name)
    widget:getChildById('value'):setText(item.value)
    widget.itemData = item
    widget.onClick = function()
      ModuleName.selectItem(item)
    end
  end
end
```

### 10. Animation & Effects

**Available Effects (in Lua)**:
```lua
-- Fade in over 300ms
g_effects.fadeIn(widget, 300)

-- Fade out over 300ms
g_effects.fadeOut(widget, 300)

-- Cancel ongoing fade
g_effects.cancelFade(widget)

-- Blinking effect
g_effects.startBlink(widget, duration, interval, clickCancel)
-- duration: 0 = infinite, otherwise milliseconds
-- interval: blink speed in ms (default 500)
-- clickCancel: stop on click (default true)

g_effects.stopBlink(widget)
```

**Progress Animation**:
```lua
function updateProgress(widget, targetPercent, duration)
  local startPercent = widget:getPercent()
  local startTime = g_clock.millis()

  local function animate()
    local elapsed = g_clock.millis() - startTime
    if elapsed >= duration then
      widget:setPercent(targetPercent)
      return
    end

    local progress = elapsed / duration
    local current = startPercent + (targetPercent - startPercent) * progress
    widget:setPercent(current)

    scheduleEvent(animate, 30)
  end

  animate()
end
```

---

## Premium Design Patterns

### 1. Visual Feedback
Always provide clear feedback for user actions:
```otui
Button
  // Distinct hover state
  $hover !disabled:
    image-clip: 0 23 22 23
    color: #ffffff

  // Pressed feedback
  $pressed:
    image-clip: 0 46 22 23
    text-offset: 1 1            // Subtle press effect

  // Clear disabled state
  $disabled:
    color: #dfdfdf88
    opacity: 0.8
```

### 2. Information Density
Balance information display with readability:
```otui
// Compact mode for game panels
CompactRow < Panel
  height: 16
  padding: 2 4

// Comfortable mode for settings
ComfortableRow < Panel
  height: 24
  padding: 4 8

// Spacious mode for important dialogs
SpaciousRow < Panel
  height: 32
  padding: 8 12
```

### 3. Responsive Layouts
Support different contexts:
```otui
Panel
  $mobile:
    height: 44                   // Larger touch targets
    padding: 12 16

  Button
    $mobile:
      height: 40                 // Minimum 44px touch target
```

### 4. Consistent Spacing
Use the spacing scale systematically:
```otui
Window
  padding: 16 16 16 16           // Outer padding

  Section
    margin-bottom: 16            // Between sections

    Row
      margin-bottom: 8           // Between rows

      Element
        margin-right: 4          // Within rows
```

### 5. Clear Visual Hierarchy
Group related elements visually:
```otui
SectionPanel < Panel
  image-source: /images/ui/panel_flat
  image-border: 2
  padding: 12
  margin-bottom: 12

  SectionTitle < Label
    font: verdana-11px-antialised
    color: #ffffff
    margin-bottom: 8

  HorizontalSeparator
    margin-top: 8
    margin-bottom: 8
```

---

## Common Component Templates

### Modal Dialog
```otui
ConfirmDialog < MainWindow
  id: confirmDialog
  !text: tr('Confirm Action')
  size: 280 160
  @onEscape: self:destroy()

  Label
    id: message
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    margin: 16
    text-align: center
    text-wrap: true
    color: #dfdfdf

  HorizontalSeparator
    anchors.bottom: buttonPanel.top
    anchors.left: parent.left
    anchors.right: parent.right
    margin-bottom: 8

  Panel
    id: buttonPanel
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    height: 50
    layout:
      type: horizontalBox
      align-right: true
      spacing: 8
    padding: 8 16

    Button
      !text: tr('Cancel')
      width: 70
      @onClick: self:getParent():getParent():destroy()

    Button
      !text: tr('Confirm')
      width: 70
      @onClick: |
        local dialog = self:getParent():getParent()
        if dialog.onConfirm then dialog.onConfirm() end
        dialog:destroy()
```

### Settings Panel
```otui
SettingsPanel < Panel
  layout:
    type: verticalBox
    spacing: 8
  padding: 12

  // Section header
  Label
    !text: tr('Display Settings')
    font: verdana-11px-antialised
    color: #ffffff
    margin-bottom: 4

  // Checkbox options
  CheckBox
    id: option1
    !text: tr('Enable feature')
    !tooltip: tr('Description of what this does')

  CheckBox
    id: option2
    !text: tr('Show indicators')

  HorizontalSeparator
    margin-top: 8
    margin-bottom: 8

  // Slider option
  Panel
    height: 40

    Label
      anchors.top: parent.top
      anchors.left: parent.left
      !text: tr('Opacity')
      color: #aaaaaa

    HorizontalScrollBar
      id: opacitySlider
      anchors.top: prev.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      margin-top: 4
      minimum: 0
      maximum: 100
      step: 5
```

### Data Table
```otui
DataTable < Panel
  layout:
    type: verticalBox

  // Header row
  Panel
    id: header
    height: 24
    layout:
      type: horizontalBox
    background: #2a2a2a
    padding: 4 8

    Label
      width: 120
      !text: tr('Name')
      color: #aaaaaa

    Label
      width: 80
      !text: tr('Level')
      color: #aaaaaa
      text-align: center

    Label
      !text: tr('Status')
      color: #aaaaaa
      text-align: right

  // Scrollable body
  ScrollablePanel
    id: body
    anchors.fill: parent
    anchors.top: header.bottom
    vertical-scrollbar: tableScrollBar
    layout:
      type: verticalBox
      spacing: 1
    background: #1a1a1a
    padding: 2

  VerticalScrollBar
    id: tableScrollBar
    anchors.top: header.bottom
    anchors.bottom: parent.bottom
    anchors.right: parent.right
    step: 24
    pixels-scroll: true

// Row template
DataTableRow < Panel
  height: 22
  layout:
    type: horizontalBox
  padding: 2 8
  background: #252525

  $hover:
    background: #353535

  Label
    id: name
    width: 120
    color: #dfdfdf

  Label
    id: level
    width: 80
    text-align: center
    color: #dfdfdf

  Label
    id: status
    text-align: right
    color: #44ff44
```

---

## Quality Checklist

Before finalizing any OTUI interface:

- [ ] **Functionality**: All interactive elements work correctly
- [ ] **Visual Hierarchy**: Important elements stand out, secondary elements recede
- [ ] **Consistency**: Colors, spacing, and fonts follow established patterns
- [ ] **State Feedback**: Hover, pressed, disabled states are clearly distinct
- [ ] **Accessibility**: Text is readable, touch targets are adequate
- [ ] **Integration**: Fits with existing game UI style
- [ ] **Performance**: No unnecessary widgets or complex layouts
- [ ] **Localization**: All text uses `tr()` for translation support
- [ ] **Persistence**: User preferences save and restore correctly
- [ ] **Error States**: Handles edge cases gracefully

Remember: OTUI interfaces must be functional during active gameplay. Prioritize clarity, efficiency, and reliability over decorative complexity. The best game UI is one that players barely notice because it works so intuitively.
